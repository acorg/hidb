#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <optional>

#include "acmacs-base/timeit.hh"
#include "acmacs-chart/chart.hh"

// ----------------------------------------------------------------------

namespace hidb
{

    class HiDb;

// ----------------------------------------------------------------------

    class PerTable
    {
     public:
        inline PerTable() = default;
        inline PerTable(std::string aTableId, const Antigen& aAntigen) : mTableId(aTableId), mDate(aAntigen.date()), mLabId(aAntigen.lab_id()) {}
        inline PerTable(std::string aTableId, const Serum& /*aSerum*/) : mTableId(aTableId) {}

        inline const std::string table_id() const { return mTableId; }
        inline std::string& table_id() { return mTableId; }
        inline const std::string date() const { return mDate; } // date of an antigen in that table! (NOT date of a table!)
        inline std::string& date() { return mDate; }
        inline const std::vector<std::string>& lab_id() const { return mLabId; }
        inline std::vector<std::string>& lab_id() { return mLabId; }
        inline bool has_lab_id(std::string aLabId) const { return std::find(mLabId.begin(), mLabId.end(), aLabId) != mLabId.end(); }
        inline const std::string homologous() const { return mHomologous; }
        inline std::string& homologous() { return mHomologous; }

        inline void set_homologous(std::string aHomologous) { mHomologous = aHomologous; }

        inline bool operator < (const PerTable& aNother) const { return mTableId < aNother.mTableId; }
        inline bool operator < (const std::string& aTableId) const { return mTableId < aTableId; }

     private:
        std::string mTableId;
        std::string mDate;
        std::vector<std::string> mLabId;
        std::string mHomologous;    // variant_id of the homologous antigen
    };

// ----------------------------------------------------------------------

    template <typename AS> class AntigenSerumData
    {
     public:
        inline AntigenSerumData() = default;
        inline AntigenSerumData(const AS& aData) : mData(aData) {}

        inline void update(std::string aTableId, const AS& aData)
            {
                // std::cerr << "add " << aTableId << " " << aData.full_name() << std::endl;
                if (lineage() != aData.lineage())
                    std::cerr << "WARNING: conflicting lineage for " << full_name() << ": db:" << lineage() << " new:" << aData.lineage() << std::endl;
                PerTable pt(aTableId, aData);
                auto insert_at = std::lower_bound(mTables.begin(), mTables.end(), pt);
                if (insert_at == mTables.end() || insert_at->table_id() != aTableId) {
                    mTables.insert(insert_at, std::move(pt));
                }
                else {
                      // std::cerr << "mTableIds " << mTableIds.size() << std::endl;
                    throw std::runtime_error("AntigenSerumData::update: table_id " + aTableId + " already present for antigen/serum: " + aData.full_name() + " (duplicates in the table?)");
                }
            }

        inline void set_homologous(std::string aTableId, std::string aHomologous)
            {
                auto existing = std::lower_bound(mTables.begin(), mTables.end(), aTableId); //, [](const PerTable& a, const);
                if (existing->table_id() == aTableId) {
                    existing->set_homologous(aHomologous);
                }
                else {
                    throw std::runtime_error("AntigenSerumData::set_homologous");
                }
            }

        // inline bool operator < (const AntigenSerumData& aNother) const { return mData < aNother.mData; }
        // inline bool operator == (const AntigenSerumData& aNother) const { return mData == aNother.mData; }

        inline const AS& data() const { return mData; }
        inline AS& data() { return mData; }
        inline std::string name() const { return mData.name(); }
        inline std::string full_name() const { return mData.full_name(); }
        inline const std::vector<PerTable>& per_table() const { return mTables; }
        inline std::vector<PerTable>& per_table() { return mTables; }
        inline size_t number_of_tables() const { return mTables.size(); }
        inline const PerTable& most_recent_table() const { return *std::max_element(mTables.begin(), mTables.end()); }
        inline const PerTable& oldest_table() const { return *std::min_element(mTables.begin(), mTables.end()); }
        inline bool has_lab_id(std::string aLabId) const { return std::any_of(mTables.begin(), mTables.end(), [&](const auto& e) -> bool { return e.has_lab_id(aLabId); }); }
        inline std::string lineage() const { return mData.lineage(); }
        void labs(const HiDb& aHiDb, std::vector<std::string>& aLabs) const;
        bool has_lab(const HiDb& aHiDb, std::string aLab) const;
        bool in_hi_assay(const HiDb& aHiDb) const;
        bool in_neut_assay(const HiDb& aHiDb) const;

        inline std::vector<std::pair<std::string, std::string>> homologous() const
            {
                std::vector<std::pair<std::string, std::string>> result;
                for (const auto& t: mTables) {
                    if (!t.homologous().empty())
                        result.emplace_back(t.table_id(), t.homologous());
                }
                return result;
            }

        inline std::vector<std::string> homologous_variant_ids() const
            {
                std::vector<std::string> result;
                for (const auto& t: mTables) {
                    if (!t.homologous().empty())
                        result.push_back(t.homologous());
                }
                std::sort(result.begin(), result.end());
                result.erase(std::unique(result.begin(), result.end()), result.end());
                return result;
            }

          // returns if serum has passed variant_id among variant ids of its homologous antigens
        inline bool has_homologous_variant_id(std::string variant_id) const
            {
                return std::find_if(mTables.begin(), mTables.end(), [&variant_id](const auto& t) -> bool { return t.homologous() == variant_id; }) != mTables.end();
            }

          // returns isolation date (or empty string, if not available), if multiple dates are found in different tables, returns the most recent date
        inline std::string date() const
            {
                return mTables.empty() ? std::string() : std::max_element(mTables.begin(), mTables.end(), [](const auto& a, const auto& b) { return a.date() < b.date(); })->date();
            }

     private:
        AS mData;
        std::vector<PerTable> mTables;

    }; // class AntigenSerumData<>

    using AntigenData = AntigenSerumData<Antigen>;
    using SerumData = AntigenSerumData<Serum>;

// ----------------------------------------------------------------------

    class ChartData
    {
     public:
        using AgSrRef = std::pair<std::string, std::string>;
        using Titers = std::vector<std::vector<std::string>>;

        inline ChartData() = default;
        ChartData(const Chart& aChart);

        inline const std::string table_id() const { return mTableId; }
        inline const ChartInfo& chart_info() const { return mChartInfo; }
        inline size_t number_of_antigens() const { return mAntigens.size(); }
        inline size_t number_of_sera() const { return mSera.size(); }
        inline const std::vector<AgSrRef>& antigens() const { return mAntigens; }
        inline const std::vector<AgSrRef>& sera() const { return mSera; }
        inline const Titers& titers() const { return mTiters; }
        inline std::string titer(size_t antigen_no, size_t serum_no) const { return titers()[antigen_no][serum_no]; }

        inline std::string& table_id() { return mTableId; }
        inline ChartInfo& chart_info() { return mChartInfo; }
        inline std::vector<AgSrRef>& antigens() { return mAntigens; }
        inline std::vector<AgSrRef>& sera() { return mSera; }
        inline Titers& titers() { return mTiters; }

        inline bool operator <(const ChartData& aNother) const { return table_id() < aNother.table_id(); }

        size_t antigen_index_by_full_name(std::string full_name) const; // returns -1 if not found
        inline std::string antigen_full_name(size_t index) const { const auto& ag = mAntigens[index]; return ag.first + " " + ag.second; }
        inline std::string serum_full_name(size_t index) const { const auto& sr = mSera[index]; return sr.first + " " + sr.second; }

     private:
        std::string mTableId;
        ChartInfo mChartInfo;
        std::vector<AgSrRef> mAntigens;
        std::vector<AgSrRef> mSera;
        Titers mTiters;

    }; // class ChartData

// ----------------------------------------------------------------------

    class Tables : public std::vector<ChartData>
    {
     public:
        inline auto insert_pos(const ChartData& aChart)
            {
                const auto chart_insert_at = std::lower_bound(begin(), end(), aChart);
                if (chart_insert_at != end() && chart_insert_at->table_id() == aChart.table_id())
                    throw std::runtime_error("Chart " + aChart.table_id() + " already in hidb");
                return chart_insert_at;
            }

        inline const ChartData& operator[](std::string aTableId) const
            {
                auto c = std::lower_bound(begin(), end(), aTableId, [](const auto& a, const auto& b) { return a.table_id() < b; });
                if (c == end() || c->table_id() != aTableId)
                    throw std::runtime_error("Tables::[]: table_id not found");
                return *c;
            }

    }; // class Tables

// ----------------------------------------------------------------------

    class AntigenRefs : public std::vector<const AntigenData*>
    {
     public:
        inline AntigenRefs() : mHiDb(nullptr) {}
        inline AntigenRefs(const HiDb& aHiDb) : mHiDb(&aHiDb) {}
        template <typename Iterator> inline AntigenRefs(const HiDb& aHiDb, Iterator first, Iterator last) : std::vector<const AntigenData*>(first, last), mHiDb(&aHiDb) {}
        AntigenRefs& country(std::string aCountry);
        AntigenRefs& date_range(std::string aBegin, std::string aEnd);

     private:
        const HiDb* mHiDb;

    }; // class AntigenRefs

// ----------------------------------------------------------------------

    class Antigens : public std::vector<AntigenData>
    {
     public:
        AntigenRefs all(const HiDb& aHiDb) const;

        void make_index(const HiDb& aHiDb);
          // if location is not found and aNotFoundLocation is not nullptr, location name is copied there and not reported to std::cerr
        AntigenRefs find_by_index(std::string name, std::string* aNotFoundLocation = nullptr) const;
        AntigenRefs find_by_cdcid(std::string cdcid) const;

        inline const AntigenRefs* all_by_index(std::string name) const
            {
                try {
                    return for_key(index_key(name));
                }
                catch (NotFound&) {
                    return nullptr;
                }
            }

     private:
        static constexpr const size_t IndexKeySize = 2;
        std::map<std::string, AntigenRefs> mIndex;

        class NotFound : public std::runtime_error { public: using std::runtime_error::runtime_error; };

        inline void split(std::string name, std::string& virus_type, std::string& host, std::string& location, std::string& isolation, std::string& year, std::string& passage, std::string& index_key) const
            {
                try {
                    virus_name::split(name, virus_type, host, location, isolation, year, passage);
                    index_key = location.substr(0, IndexKeySize);
                }
                catch (virus_name::Unrecognized&) {
                    throw NotFound{"cannot split " + name};
                }
            }

        inline std::string index_key(std::string name) const
            {
                try {
                    return virus_name::location(name).substr(0, IndexKeySize);
                }
                catch (virus_name::Unrecognized&) {
                    throw NotFound{"cannot find location in " + name};
                }
            }

        inline const AntigenRefs* for_key(std::string key) const
            {
                auto p = mIndex.find(key);
                return p != mIndex.end() ? &p->second : nullptr;
            }

        void find_by_index_cdc_name(std::string name, AntigenRefs& aResult) const;

    }; // class Antigens

// ----------------------------------------------------------------------

    class Sera : public std::vector<SerumData>
    {
    }; // class Sera

// ----------------------------------------------------------------------

    using VirusType = std::string;
    using Lab = std::string;
    using YearMonth = std::string;
    using Continent = std::string;
    using HiDbStatContainer = std::map<VirusType, std::map<Lab, std::map<YearMonth, std::map<Continent, size_t>>>>;

    class HiDbStat : public HiDbStatContainer
    {
     public:
        void compute_totals();
    };

// ----------------------------------------------------------------------

    class HiDb
    {
     public:
        class NotFound : public std::runtime_error
        {
         public:
            using std::runtime_error::runtime_error;
            inline NotFound(std::string aMessage, const AntigenRefs& aSuggestions) : std::runtime_error::runtime_error(aMessage), mSuggestions(aSuggestions) {}
            const AntigenRefs& suggestions() const { return mSuggestions; }

         private:
            AntigenRefs mSuggestions;
        };

        inline HiDb() {}

        void add(const Chart& aChart);
        void importFrom(std::string aFilename, report_time timer = report_time::No);
        void exportTo(std::string aFilename, bool aPretty, report_time timer = report_time::No) const;

        inline const Antigens& antigens() const { return mAntigens; }
        inline Antigens& antigens() { return mAntigens; }
        inline const Sera& sera() const { return mSera; }
        inline Sera& sera() { return mSera; }
        inline const Tables& charts() const { return mCharts; }
        inline Tables& charts() { return mCharts; }
        inline const ChartData& table(std::string table_id) const { return charts()[table_id]; }

        std::vector<const AntigenData*> find_antigens(std::string name_reassortant_annotations_passage) const;
        const AntigenData& find_antigen_exactly(std::string name_reassortant_annotations_passage) const; // throws NotFound if antigen with this very set of data not found
        std::vector<const AntigenData*> find_antigens_fuzzy(std::string name_reassortant_annotations_passage) const;
        std::vector<const AntigenData*> find_antigens_extra_fuzzy(std::string name_reassortant_annotations_passage) const;
        inline std::vector<const AntigenData*> find_antigens_by_name(std::string name, std::string* aNotFoundLocation = nullptr) const { return mAntigens.find_by_index(name, aNotFoundLocation); }
        inline std::vector<const AntigenData*> find_antigens_by_cdcid(std::string cdcid) const  { return mAntigens.find_by_cdcid(cdcid); }
        const AntigenData& find_antigen_of_chart(const Antigen& aAntigen) const; // throws if not found

        std::vector<std::pair<const AntigenData*, size_t>> find_antigens_with_score(std::string name) const;
        std::vector<std::string> list_antigen_names(std::string aLab, std::string aLineage, bool aFullName) const;
        std::vector<const AntigenData*> list_antigens(std::string aLab, std::string aLineage, std::string aAssay) const;
        std::vector<const SerumData*> find_sera(std::string name) const;
        const SerumData& find_serum_exactly(std::string name_reassortant_annotations_serum_id) const; // throws NotFound if serum with this very set of data not found
        std::vector<std::pair<const SerumData*, size_t>> find_sera_with_score(std::string name) const;
        std::vector<std::string> list_serum_names(std::string aLab, std::string aLineage, bool aFullName) const;
        std::vector<const SerumData*> list_sera(std::string aLab, std::string aLineage) const;
        std::vector<const SerumData*> find_homologous_sera(const AntigenData& aAntigen) const;
        const SerumData& find_serum_of_chart(const Serum& aSerum, bool report_if_not_found = false) const; // throws if not found
        void find_homologous_antigens_for_sera_of_chart(Chart& aChart);
        std::string serum_date(const SerumData& aSerum) const;

          // name is just (international) name without reassortant/passage

        inline AntigenRefs all_antigens() const { return mAntigens.all(*this); }

        std::vector<std::string> all_countries() const;
        std::vector<std::string> unrecognized_locations() const;
        void stat_antigens(HiDbStat& aStat, std::string aStart, std::string aEnd) const;
        void stat_sera(HiDbStat& aStat, HiDbStat* aStatUnique, std::string aStart, std::string aEnd) const;

     private:
        Antigens mAntigens;
        Sera mSera;
        Tables mCharts;

        void add_antigen(const Antigen& aAntigen, std::string aTableId);
        void add_serum(const Serum& aSerum, std::string aTableId, const std::vector<Antigen>& aAntigens);
        const AntigenData& find_antigen_in_suggestions(std::string aName, const AntigenRefs& aSuggestions) const;

    }; // class HiDb

    template <typename AS> inline std::string report(const std::vector<const AS*>& aAntigens, std::string aPrefix = "")
    {
        std::ostringstream out;
        for (const auto* ag: aAntigens) {
            out << aPrefix << ag->data().full_name() << std::endl;
        }
        return out.str();
    }

// ----------------------------------------------------------------------

    template <typename AS> void AntigenSerumData<AS>::labs(const HiDb& aHiDb, std::vector<std::string>& aLabs) const
    {
        std::transform(per_table().begin(), per_table().end(), std::back_inserter(aLabs), [&aHiDb](const auto& pt) -> std::string { return aHiDb.charts()[pt.table_id()].chart_info().lab(); });
        std::sort(aLabs.begin(), aLabs.end());
        aLabs.erase(std::unique(aLabs.begin(), aLabs.end()), aLabs.end());
    }

    template <typename AS> bool AntigenSerumData<AS>::has_lab(const HiDb& aHiDb, std::string aLab) const
    {
        return std::find_if(per_table().begin(), per_table().end(), [&aHiDb,&aLab](const auto& pt) -> bool { return aHiDb.charts()[pt.table_id()].chart_info().lab() == aLab; }) != per_table().end();
    }

    template <typename AS> bool AntigenSerumData<AS>::in_hi_assay(const HiDb& aHiDb) const
    {
        return std::find_if(per_table().begin(), per_table().end(), [&aHiDb](const auto& pt) -> bool { return aHiDb.charts()[pt.table_id()].chart_info().assay() == "HI"; }) != per_table().end();
    }

    template <typename AS> bool AntigenSerumData<AS>::in_neut_assay(const HiDb& aHiDb) const
    {
        return std::find_if(per_table().begin(), per_table().end(), [&aHiDb](const auto& pt) -> bool { return aHiDb.charts()[pt.table_id()].chart_info().assay() != "HI"; }) != per_table().end();
    }

// ----------------------------------------------------------------------

    class NoHiDb : public std::exception {};

    void setup(std::string aHiDbDir, std::optional<std::string> aLocDbFilename = {});
    const HiDb& get(std::string aVirusType, report_time timer = report_time::No);

// ----------------------------------------------------------------------

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
