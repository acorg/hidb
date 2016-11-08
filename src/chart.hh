#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// ----------------------------------------------------------------------

class AntigenSerumMatch
{
 public:
    enum Level : size_t { Perfect=0, SerumSpeciesMismatch=0x1, PassageMismatch=0x2, PassageWithoutDateMismatch=0x4, EggCellUnknown=0x8, EggCellMismatch=0x10,
                          Mismatch=0x20, AnnotationMismatch=0x40, ReassortantMismatch=0x80, SerumIdMismatch=0x100, NameMismatch=0x200 };

    inline bool operator < (AntigenSerumMatch m) const { return mLevel < m.mLevel; }
    inline bool operator < (Level l) const { return mLevel < l; }
    inline bool operator == (AntigenSerumMatch m) const { return mLevel == m.mLevel; }
    inline bool operator != (AntigenSerumMatch m) const { return ! operator==(m); }

    inline void add(Level toAdd) { mLevel += toAdd; }
    inline void add(AntigenSerumMatch toAdd) { mLevel += toAdd.mLevel; }
    inline bool mismatch() const { return mLevel > Mismatch; }
    inline bool name_match() const { return mLevel < NameMismatch; }
    inline bool reassortant_match() const { return mLevel < ReassortantMismatch; }
    inline bool homologous_match() const { return mLevel < EggCellMismatch; }
    inline bool perfect() const { return mLevel == Perfect; }

 private:
    size_t mLevel;
    inline AntigenSerumMatch() : mLevel(Perfect) {}

    friend class AntigenSerum;
    friend class Serum;

    friend inline std::ostream& operator << (std::ostream& out, const AntigenSerumMatch& m) { return out << "0x" << std::hex << m.mLevel << std::dec; }
};

// ----------------------------------------------------------------------

class Annotations : public std::vector<std::string>
{
 public:
    inline Annotations() = default;

    inline bool has(std::string anno) const { return std::find(begin(), end(), anno) != end(); }
    inline bool distinct() const { return has("DISTINCT"); }

    inline void sort() { std::sort(begin(), end()); }
    inline void sort() const { const_cast<Annotations*>(this)->sort(); }

      // note annotations has to be sorted (regardless of const) to compare
    inline bool operator == (const Annotations& aNother) const
        {
            bool equal = size() == aNother.size();
            if (equal) {
                sort();
                aNother.sort();
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
    inline AntigenSerum& operator=(const AntigenSerum&) = default;

    virtual std::string full_name() const = 0;

    inline std::string name() const { return mName; }
    inline std::string& name() { return mName; }
    inline std::string lineage() const { return mLineage; }
    inline std::string& lineage() { return mLineage; }
    inline std::string passage() const { return mPassage; }
    inline std::string& passage() { return mPassage; }
    inline bool has_passage() const { return !mPassage.empty(); }
    std::string passage_without_date() const;
    inline std::string reassortant() const { return mReassortant; }
    inline std::string& reassortant() { return mReassortant; }
    virtual bool is_egg() const;
    inline bool is_reassortant() const { return !mReassortant.empty(); }
    inline bool distinct() const { return mAnnotations.distinct(); }
    inline const Annotations& annotations() const { return mAnnotations; }
    inline Annotations& annotations() { return mAnnotations; }
    inline bool has_semantic(char c) const { return mSemanticAttributes.find(c) != std::string::npos; }
    inline std::string semantic() const { return mSemanticAttributes; }
    inline std::string& semantic() { return mSemanticAttributes; }

    virtual std::string variant_id() const = 0;
    virtual AntigenSerumMatch match(const AntigenSerum& aNother) const;

    inline bool operator == (const AntigenSerum& aNother) const { return name() == aNother.name() && variant_id() == aNother.variant_id(); }
    inline bool operator < (const AntigenSerum& aNother) const { return name() == aNother.name() ? variant_id() < aNother.variant_id() : name() < aNother.name(); }

 protected:
    inline AntigenSerum() = default;
    inline AntigenSerum(const AntigenSerum&) = default;
      //inline AntigenSerum(Chart& aChart) : mChart(aChart) {}

    virtual AntigenSerumMatch match_passage(const AntigenSerum& aNother) const;

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

class Serum;

class Antigen : public AntigenSerum
{
 public:
      // inline Antigen(Chart& aChart) : AntigenSerum(aChart) {}
    inline Antigen() = default;
    virtual inline std::string full_name() const { const auto vi = variant_id(); std::string n = name(); if (!vi.empty()) { n.append(1, ' '); n.append(vi); } return n; }

    inline std::string date() const { return mDate; }
    inline bool reference() const { return has_semantic('R'); }
    inline const std::vector<std::string>& lab_id() const { return mLabId; }
    virtual std::string variant_id() const;
    inline const std::vector<std::string>& clades() const { return mClades; }

    using AntigenSerum::match;
    virtual AntigenSerumMatch match(const Serum& aNother) const;
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
    inline std::string& serum_id() { return mSerumId; }
    inline std::string serum_species() const { return mSerumSpecies; }
    inline std::string& serum_species() { return mSerumSpecies; }
    virtual std::string variant_id() const;

    template <typename No> inline void set_homologous(No ag_no) { mHomologous = static_cast<decltype(mHomologous)>(ag_no); }
    inline bool has_homologous() const { return mHomologous >= 0; }
    inline int homologous() const { return mHomologous; }
    virtual bool is_egg() const;

    using AntigenSerum::match;
    virtual AntigenSerumMatch match(const Serum& aNother) const;
    virtual AntigenSerumMatch match(const Antigen& aNother) const;

 protected:
    virtual AntigenSerumMatch match_passage(const AntigenSerum& aNother) const;

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
    std::string table_id(std::string lineage) const;

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

    inline const List& as_list() const
        {
            if (mList.empty()) {
                std::cerr << "Warning: sparse matrix of titers ignored" << std::endl;
                  //throw std::runtime_error("ChartTiters::as_list");
            }
            return mList;
        }

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
    inline Chart(const Chart&) = default;
    inline Chart& operator=(const Chart&) = default;

    inline std::string virus_type() const { return mInfo.virus_type(); }

    inline size_t number_of_antigens() const { return mAntigens.size(); }
    inline size_t number_of_sera() const { return mSera.size(); }
    inline std::string table_id() const { return mInfo.table_id(lineage()); }
    std::string lineage() const;
    
    inline const std::vector<Antigen>& antigens() const { return mAntigens; }
    inline Antigen& antigen(size_t ag_no) { return mAntigens[ag_no]; }
    inline const std::vector<Serum>& sera() const { return mSera; }
    inline Serum& serum(size_t sr_no) { return mSera[sr_no]; }
    inline const ChartTiters& titers() const { return mTiters; }

    void find_homologous_antigen_for_sera();
    inline void find_homologous_antigen_for_sera_const() const { const_cast<Chart*>(this)->find_homologous_antigen_for_sera(); }

    inline bool operator < (const Chart& aNother) const { return table_id() < aNother.table_id(); }

 private:
    friend class ChartReaderEventHandler;
    ChartInfo mInfo;
    std::vector<Antigen> mAntigens;
    std::vector<Serum> mSera;
    ChartTiters mTiters;
    std::vector <double> mColumnBases;

};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
