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

    inline std::string date() const { return mDate; }
    inline const std::vector<std::string>& lab_id() const { return mLabId; }
    inline std::string homologous() const { return mHomologous; }

    inline void set_homologous(std::string aHomologous) { mHomologous = aHomologous; }

 private:
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
            auto exisiting = mTableIds.find(aTableId);
            if (exisiting == mTableIds.end()) {
                mTableIds.insert(std::make_pair(aTableId, PerTable(aAS)));
            }
            else {
                  // std::cerr << "mTableIds " << mTableIds.size() << std::endl;
                throw std::runtime_error("AntigenSerumData::update: table_id already present for this antigen/serum: " + aTableId);
            }
        }

    inline void set_homologous(std::string aTableId, std::string aHomologous)
        {
            auto exisiting = mTableIds.find(aTableId);
            if (exisiting != mTableIds.end()) {
                exisiting->second.set_homologous(aHomologous);
            }
            else {
                throw std::runtime_error("AntigenSerumData::set_homologous");
            }
        }

    inline bool operator < (const AntigenSerumData& aNother) const { return mAS < aNother.mAS; }
    inline bool operator == (const AntigenSerumData& aNother) const { return mAS == aNother.mAS; }

    inline const AS& as() const { return mAS; }
    inline const std::map<std::string, PerTable>& per_table() const { return mTableIds; }

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
    void exportTo(std::string aFilename) const;

 private:
    std::vector<AntigenData> mAntigens;
    std::vector<SerumData> mSera;
    std::vector<Chart> mTables;

    void add_antigen(const Antigen& aAntigen, std::string aTableId);
    void add_serum(const Serum& aSerum, std::string aTableId, const std::vector<Antigen>& aAntigens);
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
