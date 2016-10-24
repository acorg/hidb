#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "chart.hh"

// ----------------------------------------------------------------------

class PerTable
{
 public:
    inline PerTable(const Antigen& aAntigen) : mDate(aAntigen.date()), mLabId(aAntigen.lab_id()) {}
    inline PerTable(const Serum& /*aSerum*/) {}

 private:
    std::string mDate;
    std::vector<std::string> mLabId;
};

// ----------------------------------------------------------------------

template <typename AS> class AntigenSerumData
{
 public:
    inline AntigenSerumData(const AS& aAS) : mAS(aAS) {}

    inline void update(std::string aTableId, const AS& aAS)
        {
            auto exisiting = mTableIds.find(aTableId);
            if (exisiting == mTableIds.end()) {
                mTableIds.insert(std::make_pair(aTableId, PerTable(aAS)));
            }
            else {
                throw std::runtime_error("AntigenSerumData::update");
            }
        }

    inline bool operator < (const AntigenSerumData& aNother) const { return mAS < aNother.mAS; }
    inline bool operator == (const AntigenSerumData& aNother) const { return mAS == aNother.mAS; }

 private:
    AS mAS;
    std::map<std::string, PerTable> mTableIds;
};

typedef AntigenSerumData<Antigen> AntigenData;
typedef AntigenSerumData<Serum> SerumData;

// ----------------------------------------------------------------------

class HiDb
{
 public:
    inline HiDb() {}

    void add(const Chart& aChart);

 private:
    std::vector<AntigenData> mAntigens;
    std::vector<SerumData> mSera;
    std::vector<Chart> mTables;

    void add_antigen(const Antigen& aAntigen, std::string aTableId);
    void add_serum(const Serum& aSerum, std::string aTableId);
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
