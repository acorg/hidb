#include <algorithm>
#include <functional>
#include <iomanip>

#include "hidb.hh"
#include "variant-id.hh"
#include "vaccines.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static std::map<std::string, std::vector<hidb::Vaccine>> sVaccines = {
    {"A(H1N1)", {
            {"CALIFORNIA/7/2009",        hidb::Vaccine::Previous},
            {"MICHIGAN/45/2015",         hidb::Vaccine::Current},
        }},
    {"A(H3N2)", {
            {"BRISBANE/10/2007",         hidb::Vaccine::Previous},
            {"PERTH/16/2009",            hidb::Vaccine::Previous},
            {"VICTORIA/361/2011",        hidb::Vaccine::Previous},
            {"TEXAS/50/2012",            hidb::Vaccine::Previous},
            {"SWITZERLAND/9715293/2013", hidb::Vaccine::Previous},
            {"HONG KONG/4801/2014",      hidb::Vaccine::Current},
            {"SAITAMA/103/2014",         hidb::Vaccine::Surrogate},
            {"HONG KONG/7295/2014",      hidb::Vaccine::Surrogate},
        }},
    {"BVICTORIA", {
            {"MALAYSIA/2506/2004",       hidb::Vaccine::Previous},
            {"BRISBANE/60/2008",         hidb::Vaccine::Current},
              // {"PARIS/1762/2009",          hidb::Vaccine::Current}, // not used by Crick anymore, B/Ireland/3154/2016 is used instead (2017-08-21)
            {"SOUTH AUSTRALIA/81/2012",  hidb::Vaccine::Surrogate},
            {"IRELAND/3154/2016",  hidb::Vaccine::Surrogate},
        }},
    {"BYAMAGATA", {
            {"FLORIDA/4/2006",           hidb::Vaccine::Previous},
            {"WISCONSIN/1/2010",         hidb::Vaccine::Previous},
            {"MASSACHUSETTS/2/2012",     hidb::Vaccine::Previous},
            {"PHUKET/3073/2013",         hidb::Vaccine::Current},
        }},
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

std::string hidb::Vaccine::type_as_string(hidb::Vaccine::Type aType)
{
    switch (aType) {
      case Previous:
          return "previous";
      case Current:
          return "current";
      case Surrogate:
          return "surrogate";
    }
    return {};

} // hidb::Vaccine::type_as_string

// ----------------------------------------------------------------------

hidb::Vaccine::Type hidb::Vaccine::type_from_string(std::string aType)
{
    if (aType == "previous")
        return Previous;
    else if (aType == "current")
        return Current;
    else if (aType == "surrogate")
        return Surrogate;
    return Previous;

} // hidb::Vaccine::type_from_string

// ----------------------------------------------------------------------

const std::vector<hidb::Vaccine>& hidb::vaccines(std::string aSubtype, std::string aLineage)
{
    return sVaccines.at(aSubtype + aLineage);

} // hidb::vaccines

// ----------------------------------------------------------------------

const std::vector<hidb::Vaccine>& hidb::vaccines(const Chart& aChart)
{
    return vaccines(aChart.chart_info().virus_type(), aChart.lineage());

} // hidb::vaccines

// ----------------------------------------------------------------------

std::string hidb::Vaccine::type_as_string() const
{
    return type_as_string(type);

} // hidb::Vaccine::type_as_string

// ----------------------------------------------------------------------

inline bool hidb::Vaccines::Entry::operator < (const hidb::Vaccines::Entry& a) const
{
    const auto a_nt = a.antigen_data->number_of_tables(), t_nt = antigen_data->number_of_tables();
    return t_nt == a_nt ? most_recent_table_date > a.most_recent_table_date : t_nt > a_nt;
}

// ----------------------------------------------------------------------

bool hidb::Vaccines::HomologousSerum::operator < (const hidb::Vaccines::HomologousSerum& a) const
{
    bool result = true;
    if (serum->serum_species() == "SHEEP") { // avoid using sheep serum as homologous (NIMR)
        result = false;
    }
    else {
        const auto s_nt = a.serum_data->number_of_tables(), t_nt = serum_data->number_of_tables();
        result = t_nt == s_nt ? most_recent_table_date > a.most_recent_table_date : t_nt > s_nt;
    }
    return result;

} // hidb::Vaccines::HomologousSerum::operator <

// ----------------------------------------------------------------------

size_t hidb::Vaccines::HomologousSerum::number_of_tables() const
{
    return serum_data->number_of_tables();

} // hidb::Vaccines::HomologousSerum::number_of_tables

// ----------------------------------------------------------------------

std::string hidb::Vaccines::report(size_t aIndent) const
{
    std::ostringstream out;
    if (!empty()) {
        out << std::string(aIndent, ' ') << "Vaccine " << type_as_string() << ' ' << mNameType.name << std::endl;
        for_each_passage_type([&](PassageType pt) { out << this->report(pt, aIndent + 2); });
    }
    return out.str();

} // hidb::Vaccines::report

// ----------------------------------------------------------------------

std::string hidb::Vaccines::report(PassageType aPassageType, size_t aIndent, size_t aMark) const
{
    std::ostringstream out;
    const std::string indent(aIndent, ' ');
    auto entry_report = [&](size_t aNo, const auto& entry, bool aMarkIt) {
        out << indent << (aMarkIt ? ">>" : "  ") << std::setw(2) << aNo << ' ' << entry.antigen_index << ' ' << entry.antigen->full_name() << " tables:" << entry.antigen_data->number_of_tables() << " recent:" << entry.antigen_data->most_recent_table().table_id() << std::endl;
        for (const auto& hs: entry.homologous_sera)
            out << indent << "      " << hs.serum->serum_id() << ' ' << hs.serum->annotations().join() << " tables:" << hs.serum_data->number_of_tables() << " recent:" << hs.serum_data->most_recent_table().table_id() << std::endl;
    };

    const auto& entry = mEntries[aPassageType];
    if (!entry.empty()) {
        out << indent << passage_type_name(aPassageType) << " (" << entry.size() << ')' << std::endl;
        for (size_t no = 0; no < entry.size(); ++no)
            entry_report(no, entry[no], aMark == no);
    }
    return out.str();

} // hidb::Vaccines::report

// ----------------------------------------------------------------------

void hidb::vaccines_for_name(Vaccines& aVaccines, std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb)
{
    std::vector<size_t> by_name;
    aChart.antigens().find_by_name(aName, by_name);
    for (size_t ag_no: by_name) {
        try {
            const auto& ag = static_cast<const Antigen&>(aChart.antigen(ag_no));
              // std::cerr << ag.full_name() << std::endl;
            const auto& data = aHiDb.find_antigen_of_chart(ag);
            std::vector<hidb::Vaccines::HomologousSerum> homologous_sera;
            for (const auto* sd: aHiDb.find_homologous_sera(data)) {
                if (const auto sr_no = aChart.sera().find_by_full_name(hidb::name_for_exact_matching(sd->data())))
                    homologous_sera.emplace_back(*sr_no, static_cast<const Serum*>(&aChart.serum(sr_no)), sd, aHiDb.charts()[sd->most_recent_table().table_id()].chart_info().date());
            }
            aVaccines.add(ag_no, ag, &data, std::move(homologous_sera), aHiDb.charts()[data.most_recent_table().table_id()].chart_info().date());
        }
        catch (hidb::HiDb::NotFound&) {
        }
    }
    aVaccines.sort();

} // hidb::vaccines_for_name

// ----------------------------------------------------------------------

hidb::Vaccines* hidb::find_vaccines_in_chart(std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb)
{
    Vaccines* result = new Vaccines(Vaccine(aName, hidb::Vaccine::Previous));
    vaccines_for_name(*result, aName, aChart, aHiDb);
    return result;

} // find_vaccines_in_chart

// ----------------------------------------------------------------------

void hidb::vaccines(VaccinesOfChart& aVaccinesOfChart, const Chart& aChart, const hidb::HiDb& aHiDb)
{
    for (const auto& name_type: vaccines(aChart)) {
        aVaccinesOfChart.emplace_back(name_type);
        vaccines_for_name(aVaccinesOfChart.back(), name_type.name, aChart, aHiDb);
    }

} // hidb::vaccines

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

std::string hidb::VaccinesOfChart::report(size_t aIndent) const
{
    std::string result;
    for (const auto& v: *this)
        result += v.report(aIndent);
    return result;

} // hidb::VaccinesOfChart::report

// ----------------------------------------------------------------------

// void hidb::VaccinesOfChart::remove(std::string aName, std::string aType, std::string aPassageType)
// {
//     for (auto& v: *this) {
//         if (v.match(aName, aType))
//             v.remove(aPassageType);
//     }
//     erase(std::remove_if(begin(), end(), std::mem_fn<bool() const>(&hidb::Vaccines::empty)), end());

// } // hidb::VaccinesOfChart::remove

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
