#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "acmacs-chart/chart.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    class HiDb;
    template <typename AS> class AntigenSerumData;
}

// ----------------------------------------------------------------------

namespace hidb
{
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

        enum PassageType : int { Cell, Egg, Reassortant, PassageTypeSize };
        template <typename UnaryFunction> static void for_each_passage_type(UnaryFunction f) { f(Cell); f(Egg); f(Reassortant); }

        inline Vaccines(const Vaccine& aNameType) : mNameType(aNameType) {}
          // inline Vaccines(const Vaccines&) = default;
          // inline Vaccines& operator=(const Vaccines&) = default;

        inline size_t number_of(PassageType pt) const { return mEntries[pt].size(); }
        inline size_t number_of_eggs() const { return number_of(Egg); }
        inline size_t number_of_cells() const { return number_of(Cell); }
        inline size_t number_of_reassortants() const { return number_of(Reassortant); }
        inline bool empty(PassageType pt) const { return mEntries[pt].empty(); }
        inline bool empty() const { return std::all_of(std::begin(mEntries), std::end(mEntries), [](const auto& e) { return e.empty(); }); }

        inline const Entry* for_passage_type(PassageType pt, size_t aNo = 0) const { return mEntries[pt].size() > aNo ? &mEntries[pt][aNo] : nullptr; }
        inline const Entry* egg(size_t aNo = 0) const { return for_passage_type(Egg, aNo); }
        inline const Entry* cell(size_t aNo = 0) const { return for_passage_type(Cell, aNo); }
        inline const Entry* reassortant(size_t aNo = 0) const { return for_passage_type(Reassortant, aNo); }

        inline std::string type_as_string() const { return mNameType.type_as_string(); }
        std::string report(size_t aIndent = 0) const;
        std::string report(PassageType aPassageType, size_t aIndent = 0, size_t aMark = static_cast<size_t>(-1)) const;

        inline bool match(std::string aName, std::string aType) const
            {
                return (aName.empty() || mNameType.name.find(aName) != std::string::npos) && (aType.empty() || mNameType.type == Vaccine::type_from_string(aType));
            }

        inline void remove(std::string aPassageType)
            {
                const PassageType pt = passage_type(aPassageType);
                if (pt != PassageTypeSize)
                    mEntries[pt].clear();
                else
                    std::for_each(std::begin(mEntries), std::end(mEntries), [](auto& e) { e.clear(); });
            }

        inline std::string type() const { return mNameType.type_as_string(); }
        inline std::string name() const { return mNameType.name; }

     private:
        Vaccine mNameType;
        std::vector<Entry> mEntries[PassageTypeSize];

        friend void vaccines_for_name(Vaccines& aVaccines, std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb);

        static inline PassageType passage_type(const Antigen& aAntigen)
            {
                if (aAntigen.is_reassortant())
                    return Reassortant;
                else if (aAntigen.is_egg())
                    return Egg;
                return Cell;
            }

        static inline PassageType passage_type(std::string pt)
            {
                if (pt == "egg")
                    return Egg;
                else if (pt == "cell")
                    return Cell;
                else if (pt == "reassortant")
                    return Reassortant;
                else if (pt.empty())
                    return PassageTypeSize;
                else
                    throw std::runtime_error("Unrecognized passage type: " + pt);
            }

        static inline const char* passage_type_name(PassageType pt)
            {
                switch (pt) {
                  case Egg: return "Egg";
                  case Cell: return "Cell";
                  case Reassortant: return "Reassortant";
                  case PassageTypeSize: return "?";
                }
                return "?";
            }

        inline void add(size_t aAntigenIndex, const Antigen& aAntigen, const hidb::AntigenSerumData<Antigen>* aAntigenData, std::vector<HomologousSerum>&& aSera, std::string aMostRecentTableDate)
            {
                mEntries[passage_type(aAntigen)].emplace_back(aAntigenIndex, &aAntigen, aAntigenData, std::move(aSera), aMostRecentTableDate);
            }

        inline void sort()
            {
                for (auto& entry: mEntries)
                    std::sort(entry.begin(), entry.end());
            }

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
    void vaccines(VaccinesOfChart& aVaccinesOfChart, const Chart& aChart, const hidb::HiDb& aHiDb);

    inline VaccinesOfChart* vaccines(const Chart& aChart, const hidb::HiDb& aHiDb)
    {
        auto* result = new VaccinesOfChart{};
        vaccines(*result, aChart, aHiDb);
        return result;
    }

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
