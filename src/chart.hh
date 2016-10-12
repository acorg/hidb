#pragma once

#include <string>
#include <vector>

// ----------------------------------------------------------------------

class Chart;
class ChartReaderEventHandler;

class AntigenSerum
{
 public:
    std::string name() const;
    inline std::string passage() const { return mPassage; }
    inline std::string reassortant() const { return mReassortant; }
    bool is_egg() const;
    inline bool is_reassortant() const { return !mReassortant.empty(); }

 protected:
    inline AntigenSerum(Chart& aChart) : mChart(aChart) {}

 private:
    friend class ChartReaderEventHandler;
    Chart& mChart;
    std::string mName; // "N" "[VIRUS_TYPE/][HOST/]LOCATION/ISOLATION/YEAR" or "CDC_ABBR NAME" or "NAME"
    std::string mLineage; // "L"
    std::string mPassage; // "P"
    std::string mReassortant; // "R"
    std::vector<std::string> mAnnotations; // "a"

};

class Antigen : public AntigenSerum
{
 public:
    inline Antigen(Chart& aChart) : AntigenSerum(aChart), mReference(false) {}
    inline std::string date() const { return mDate; }

 private:
    friend class ChartReaderEventHandler;
    std::string mDate; // "D"
    std::string mLabId; // "l"
    bool mReference; // "r"
    std::vector<std::string> mClades; // "c"
};

class Serum : public AntigenSerum
{
 public:
    inline Serum(Chart& aChart) : AntigenSerum(aChart), mHomologous(-1) {}

 private:
    friend class ChartReaderEventHandler;
    std::string mSerumId; // "I"
    int mHomologous; // "h"
    std::string mSerumSpecies; // "s"
};

// ----------------------------------------------------------------------

class ChartInfo
{
 public:
    enum TableType {Antigenic, Genetic};
    inline ChartInfo() : mType(Antigenic) {}
    inline std::string virus_type() const { return mVirusType; }

 private:
    friend class ChartReaderEventHandler;
    std::string mVirus;     // "v"
    std::string mVirusType;     // "V"
    std::string mAssay;         // "A"
    std::string mDate;          // "D"
    std::string mLab;           // "l"
    std::string mRbc;           // "r"
    std::string mName;           // "N"
    std::string mSubset;           // "s"
    TableType mType;             // "T"
    std::vector<ChartInfo> mSources; // "S"
};

// ----------------------------------------------------------------------

class Chart
{
 public:
    inline Chart() {}

    inline std::string virus_type() const { return mInfo.virus_type(); }

    inline size_t number_of_antigens() const { return mAntigens.size(); }
    inline size_t number_of_sera() const { return mSera.size(); }

    inline Antigen& antigen(size_t ag_no) { return mAntigens[ag_no]; }
    inline Serum& serum(size_t sr_no) { return mSera[sr_no]; }

 private:
    friend class ChartReaderEventHandler;
    ChartInfo mInfo;
    std::vector<Antigen> mAntigens;
    std::vector<Serum> mSera;

    inline Chart(const Chart&) = default;
};

// ----------------------------------------------------------------------

Chart* import_chart(std::string data);

// ----------------------------------------------------------------------

inline std::string AntigenSerum::name() const
{
    if (std::count(mName.begin(), mName.end(), '/') >= 2 && mName.compare(0, 2, "A("))
        return mChart.virus_type() + "/" + mName;
    else
        return mName;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
