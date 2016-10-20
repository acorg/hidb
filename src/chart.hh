#pragma once

#include <string>
#include <vector>

// ----------------------------------------------------------------------

// class Chart;
class ChartReaderEventHandler;

// ----------------------------------------------------------------------

class AntigenSerumMatch
{
 public:
    enum Level : size_t { Perfect=0, SerumSpeciesMismatch, PassageMismatch=10, PassageWithoutDateMismatch, EggCellMismatch=100,
                          Mismatch=1000, ReassortantMismatch, NameMismatch, SerumIdMismatch, AnnotationMismatch };

    inline bool operator < (AntigenSerumMatch m) const { return mLevel < m.mLevel; }
    inline bool operator < (Level l) const { return mLevel < l; }
    inline bool operator == (AntigenSerumMatch m) const { return mLevel == m.mLevel; }
    inline bool operator != (AntigenSerumMatch m) const { return ! operator==(m); }

    inline void add(Level toAdd) { mLevel += toAdd; }
    inline bool mismatch() const { return mLevel > Mismatch; }
    inline bool homologous_match() const { return mLevel < EggCellMismatch; }
    inline bool perfect() const { return mLevel == Perfect; }

 private:
    size_t mLevel;
    inline AntigenSerumMatch() : mLevel(Perfect) {}

    friend class AntigenSerum;

    friend inline std::ostream& operator << (std::ostream& out, const AntigenSerumMatch& m) { return out << m.mLevel; }
};

// ----------------------------------------------------------------------

class Annotations : public std::vector<std::string>
{
 public:
    inline Annotations() = default;

    inline bool has(std::string anno) const { return std::find(begin(), end(), anno) != end(); }
    inline bool distinct() const { return has("DISTINCT"); }

      // note annotations has to be sorted (regardless of const) to compare
    inline bool operator == (const Annotations& aNother) const
        {
            bool equal = size() == aNother.size();
            if (equal) {
                std::sort(const_cast<Annotations*>(this)->begin(), const_cast<Annotations*>(this)->end());
                std::sort(const_cast<Annotations&>(aNother).begin(), const_cast<Annotations&>(aNother).end());
                equal = std::mismatch(begin(), end(), aNother.begin()).first == end();
            }
            return equal;
        }
};

// ----------------------------------------------------------------------

class AntigenSerum
{
 public:
    virtual ~AntigenSerum();
    virtual std::string full_name() const;

    inline std::string name() const { return mName; }
    inline std::string passage() const { return mPassage; }
    std::string passage_without_date() const;
    inline std::string reassortant() const { return mReassortant; }
    bool is_egg() const;
    inline bool is_reassortant() const { return !mReassortant.empty(); }
    inline bool distinct() const { return mAnnotations.distinct(); }
    inline const Annotations& annotations() const { return mAnnotations; }

    virtual AntigenSerumMatch match(const AntigenSerum& aNother) const;

 protected:
    inline AntigenSerum() = default;
    inline AntigenSerum(const AntigenSerum&) = default;
      //inline AntigenSerum(Chart& aChart) : mChart(aChart) {}

    inline bool has_semantic(char c) const { return mSemanticAttributes.find(c) != std::string::npos; }

 private:
    friend class ChartReaderEventHandler;
      // Chart& mChart;
    std::string mName; // "N" "[VIRUS_TYPE/][HOST/]LOCATION/ISOLATION/YEAR" or "CDC_ABBR NAME" or "NAME"
    std::string mLineage; // "L"
    std::string mPassage; // "P"
    std::string mReassortant; // "R"
    Annotations mAnnotations; // "a"
    std::string mSemanticAttributes;       // string of single letter semantic boolean attributes: R - reference, V - current vaccine, v - previous vaccine, S - vaccine surrogate

};

// ----------------------------------------------------------------------

class Antigen : public AntigenSerum
{
 public:
      // inline Antigen(Chart& aChart) : AntigenSerum(aChart) {}
    inline Antigen() = default;
    virtual std::string full_name() const;

    inline std::string date() const { return mDate; }
    inline bool reference() const { return has_semantic('R'); }

    using AntigenSerum::match;
    virtual AntigenSerumMatch match(const Antigen& aNother) const;

 private:
    friend class ChartReaderEventHandler;
    std::string mDate; // "D"
    std::vector<std::string> mLabId; // "l"
    std::vector<std::string> mClades; // "c"
};

// ----------------------------------------------------------------------

class Serum : public AntigenSerum
{
 public:
      // inline Serum(Chart& aChart) : AntigenSerum(aChart), mHomologous(-1) {}
    inline Serum() : mHomologous(-1) {}
    virtual std::string full_name() const;

    inline std::string serum_id() const { return mSerumId; }
    inline std::string serum_species() const { return mSerumSpecies; }

    template <typename No> inline void set_homologous(No ag_no) { mHomologous = static_cast<decltype(mHomologous)>(ag_no); }
    inline bool has_homologous() const { return mHomologous >= 0; }

    using AntigenSerum::match;
    virtual AntigenSerumMatch match(const Serum& aNother) const;

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

class ChartTiters
{
 public:
    typedef std::vector<std::vector<std::string>> List;
    typedef std::vector<std::vector<std::pair<std::string, std::string>>> Dict;

    inline ChartTiters() {}

 private:
    friend class ChartReaderEventHandler;
    List mList;
    Dict mDict;
    std::vector<Dict> mLayers;
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

    void find_homologous_antigen_for_sera();

 private:
    friend class ChartReaderEventHandler;
    ChartInfo mInfo;
    std::vector<Antigen> mAntigens;
    std::vector<Serum> mSera;
    ChartTiters mTiters;
    std::vector <double> mColumnBases;

    inline Chart(const Chart&) = default;
};

// ----------------------------------------------------------------------

Chart* import_chart(std::string data);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
