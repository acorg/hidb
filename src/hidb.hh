#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "chart.hh"

// ----------------------------------------------------------------------

class PerTable
{
 public:
    inline PerTable() = default;
    inline PerTable(std::string aTableId, const Antigen& aAntigen) : mTableId(aTableId), mDate(aAntigen.date()), mLabId(aAntigen.lab_id()) {}
    inline PerTable(std::string aTableId, const Serum& /*aSerum*/) : mTableId(aTableId) {}

    inline std::string table_id() const { return mTableId; }
    inline std::string& table_id() { return mTableId; }
    inline std::string date() const { return mDate; }
    inline std::string& date() { return mDate; }
    inline const std::vector<std::string>& lab_id() const { return mLabId; }
    inline std::vector<std::string>& lab_id() { return mLabId; }
    inline std::string homologous() const { return mHomologous; }
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
              // std::cerr << "add " << aTableId << " " << aAS.full_name() << std::endl;
            PerTable pt(aTableId, aData);
            auto insert_at = std::lower_bound(mTables.begin(), mTables.end(), pt);
            if (insert_at == mTables.end() || insert_at->table_id() != aTableId) {
                mTables.insert(insert_at, std::move(pt));
            }
            else {
                  // std::cerr << "mTableIds " << mTableIds.size() << std::endl;
                throw std::runtime_error("AntigenSerumData::update: table_id already present for this antigen/serum: " + aTableId);
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

    inline bool operator < (const AntigenSerumData& aNother) const { return mData < aNother.mData; }
    inline bool operator == (const AntigenSerumData& aNother) const { return mData == aNother.mData; }

    inline const AS& data() const { return mData; }
    inline AS& data() { return mData; }
    inline const std::vector<PerTable>& per_table() const { return mTables; }
    inline std::vector<PerTable>& per_table() { return mTables; }
    inline size_t number_of_tables() const { return mTables.size(); }
    inline const PerTable& most_recent_table() const { return *std::max_element(mTables.begin(), mTables.end()); }

 private:
    AS mData;
    std::vector<PerTable> mTables;
};

typedef AntigenSerumData<Antigen> AntigenData;
typedef AntigenSerumData<Serum> SerumData;

// ----------------------------------------------------------------------

class ChartData
{
 public:
    typedef std::pair<std::string, std::string> AgSrRef;
    typedef std::vector<std::vector<std::string>> Titers;

    inline ChartData() = default;
    ChartData(const Chart& aChart);

    inline std::string table_id() const { return mTableId; }
    inline const std::vector<AgSrRef>& antigens() const { return mAntigens; }
    inline const std::vector<AgSrRef>& sera() const { return mSera; }
    inline const Titers& titers() const { return mTiters; }

    inline std::string& table_id() { return mTableId; }
    inline std::vector<AgSrRef>& antigens() { return mAntigens; }
    inline std::vector<AgSrRef>& sera() { return mSera; }
    inline Titers& titers() { return mTiters; }

    inline bool operator <(const ChartData& aNother) const { return table_id() < aNother.table_id(); }

 private:
    std::string mTableId;
    std::vector<AgSrRef> mAntigens;
    std::vector<AgSrRef> mSera;
    Titers mTiters;
};

// ----------------------------------------------------------------------

class HiDb
{
 public:
    inline HiDb() {}

    void add(const Chart& aChart);
    void importFrom(std::string aFilename);
    void exportTo(std::string aFilename, bool aPretty) const;

    const std::vector<AntigenData>& antigens() const { return mAntigens; }
    std::vector<AntigenData>& antigens() { return mAntigens; }
    const std::vector<SerumData>& sera() const { return mSera; }
    std::vector<SerumData>& sera() { return mSera; }
    const std::vector<ChartData>& charts() const { return mCharts; }
    std::vector<ChartData>& charts() { return mCharts; }

    std::vector<const AntigenData*> find_antigens(std::string name) const;
    std::vector<std::pair<const AntigenData*, size_t>> find_antigens_with_score(std::string name) const;
    std::vector<std::string> list_antigens() const;
    std::vector<const SerumData*> find_sera(std::string name) const;
    std::vector<std::pair<const SerumData*, size_t>> find_sera_with_score(std::string name) const;
    std::vector<std::string> list_sera() const;

 private:
    std::vector<AntigenData> mAntigens;
    std::vector<SerumData> mSera;
    std::vector<ChartData> mCharts;

    void add_antigen(const Antigen& aAntigen, std::string aTableId);
    void add_serum(const Serum& aSerum, std::string aTableId, const std::vector<Antigen>& aAntigens);
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
