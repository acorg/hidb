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
    inline PerTable() {}

 private:
    std::string mDate;
    std::string mLabId;
};

// ----------------------------------------------------------------------

template <typename AS> class AntigenSerumData
{
 public:
    inline AntigenSerumData() {}

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
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
