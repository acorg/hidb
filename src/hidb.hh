#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "chart.hh"

// ----------------------------------------------------------------------

class PerTable
{
 public:
    inline PerTable(std::string aTableId, const Antigen& aAntigen) : mTableId(aTableId), mDate(aAntigen.date()), mLabId(aAntigen.lab_id()) {}
    inline PerTable(std::string aTableId, const Serum& /*aSerum*/) : mTableId(aTableId) {}

    inline std::string table_id() const { return mTableId; }
    inline std::string date() const { return mDate; }
    inline const std::vector<std::string>& lab_id() const { return mLabId; }
    inline std::string homologous() const { return mHomologous; }

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
    inline AntigenSerumData(const AS& aAS) : mAS(aAS) {}

    inline void update(std::string aTableId, const AS& aAS)
        {
              // std::cerr << "add " << aTableId << " " << aAS.full_name() << std::endl;
            PerTable pt(aTableId, aAS);
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

    inline bool operator < (const AntigenSerumData& aNother) const { return mAS < aNother.mAS; }
    inline bool operator == (const AntigenSerumData& aNother) const { return mAS == aNother.mAS; }

    inline const AS& as() const { return mAS; }
    inline const std::vector<PerTable>& per_table() const { return mTables; }

 private:
    AS mAS;
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

    ChartData(const Chart& aChart);

    inline std::string table_id() const { return mTableId; }
    inline const std::vector<AgSrRef>& antigens() const { return mAntigens; }
    inline const std::vector<AgSrRef>& sera() const { return mSera; }
    inline const Titers& titers() const { return mTiters; }

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
    void exportTo(std::string aFilename) const;

    const std::vector<AntigenData>& antigens() const { return mAntigens; }
    const std::vector<SerumData>& sera() const { return mSera; }
    const std::vector<ChartData>& charts() const { return mCharts; }

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
