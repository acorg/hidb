#pragma once

#include <string>
#include <vector>
#include <algorithm>

// ----------------------------------------------------------------------

namespace hidb
{
    class HiDb;
    template <typename AS> class AntigenSerumData;
}

class Antigen;
class Serum;
class Chart;

// ----------------------------------------------------------------------

class Vaccine
{
 public:
    enum Type { Previous, Current, Surrogate };

    inline Vaccine(std::string aName, Type aType) : name(aName), type(aType) {}

    std::string name;
    Type type;

    std::string type_as_string() const;

    static std::string type_as_string(Type aType);
    static Type type_from_string(std::string aType);

}; // class Vaccine

// ----------------------------------------------------------------------

class Vaccines
{
 public:
    class HomologousSerum
    {
     public:
        inline HomologousSerum(size_t aSerumIndex, const Serum* aSerum, const hidb::AntigenSerumData<Serum>* aSerumData, std::string aMostRecentTableDate)
            : serum(aSerum), serum_index(aSerumIndex), serum_data(aSerumData), most_recent_table_date(aMostRecentTableDate) {}
        bool operator < (const HomologousSerum& a) const;
        size_t number_of_tables() const;

        const Serum* serum;
        size_t serum_index;
        const hidb::AntigenSerumData<Serum>* serum_data;
        std::string most_recent_table_date;
    };

    class Entry
    {
     public:
        inline Entry(size_t aAntigenIndex, const Antigen* aAntigen, const hidb::AntigenSerumData<Antigen>* aAntigenData, std::vector<HomologousSerum>&& aSera, std::string aMostRecentTableDate)
            : antigen(aAntigen), antigen_index(aAntigenIndex), antigen_data(aAntigenData), homologous_sera(aSera), most_recent_table_date(aMostRecentTableDate)
            { std::sort(homologous_sera.begin(), homologous_sera.end()); }
        bool operator < (const Entry& a) const;

        const Antigen* antigen;
        size_t antigen_index;
        const hidb::AntigenSerumData<Antigen>* antigen_data;
        std::vector<HomologousSerum> homologous_sera; // sorted by number of tables and the most recent table
        std::string most_recent_table_date;
    };

    inline Vaccines(const Vaccine& aNameType) : mNameType(aNameType) {}
    // inline Vaccines(const Vaccines&) = default;
    // inline Vaccines& operator=(const Vaccines&) = default;

    inline size_t number_of_eggs() const { return mEgg.size(); }
    inline size_t number_of_cells() const { return mCell.size(); }
    inline size_t number_of_reassortants() const { return mReassortant.size(); }
    inline bool empty() const { return mEgg.empty() && mCell.empty() && mReassortant.empty(); }

    inline const Entry* egg(size_t aNo = 0) const { return mEgg.size() > aNo ? &mEgg[aNo] : nullptr; }
    inline const Entry* cell(size_t aNo = 0) const { return mCell.size() > aNo ? &mCell[aNo] : nullptr; }
    inline const Entry* reassortant(size_t aNo = 0) const { return mReassortant.size() > aNo ? &mReassortant[aNo] : nullptr; }

    inline std::string type_as_string() const { return mNameType.type_as_string(); }
    std::string report(size_t aIndent = 0) const;

    inline bool match(std::string aName, std::string aType) const
        {
            return (aName.empty() || mNameType.name.find(aName) != std::string::npos) && (aType.empty() || mNameType.type == Vaccine::type_from_string(aType));
        }

    inline void remove(std::string aPassageType)
        {
            if (aPassageType == "egg")
                mEgg.clear();
            else if (aPassageType == "cell")
                mCell.clear();
            else if (aPassageType == "reassortant")
                mReassortant.clear();
            else if (aPassageType.empty()) {
                mEgg.clear();
                mCell.clear();
                mReassortant.clear();
            }
            else
                throw std::runtime_error("Vaccines::remove: Unrecognized passage type: " + aPassageType);
        }

    inline std::string type() const { return mNameType.type_as_string(); }
    inline std::string name() const { return mNameType.name; }

 private:
    Vaccine mNameType;
    std::vector<Entry> mEgg;
    std::vector<Entry> mCell;
    std::vector<Entry> mReassortant;

    friend void vaccines_for_name(Vaccines& aVaccines, std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb);

    void add(size_t aAntigenIndex, const Antigen& aAntigen, const hidb::AntigenSerumData<Antigen>* aAntigenData, std::vector<HomologousSerum>&& aSera, std::string aMostRecentTableDate);
    void sort();

}; // class Vaccines

// ----------------------------------------------------------------------

class VaccinesOfChart : public std::vector<Vaccines>
{
 public:
    using std::vector<Vaccines>::vector;

    void remove(std::string aName, std::string aType, std::string aPassageType);
    std::string report(size_t aIndent = 0) const;

};

// ----------------------------------------------------------------------

const std::vector<Vaccine>& vaccines(std::string aSubtype, std::string aLineage);
const std::vector<Vaccine>& vaccines(const Chart& aChart);
Vaccines* find_vaccines_in_chart(std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb);
void vaccines_for_name(Vaccines& aVaccines, std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb);
VaccinesOfChart* vaccines(const Chart& aChart, const hidb::HiDb& aHiDb);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
