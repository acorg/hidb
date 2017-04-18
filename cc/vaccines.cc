#include "acmacs-chart/chart.hh"
#include "hidb.hh"
#include "variant-id.hh"
#include "vaccines.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

static std::map<std::string, std::vector<Vaccine>> sVaccines = {
    {"A(H1N1)", {
            {"CALIFORNIA/7/2009",        Vaccine::Previous},
            {"MICHIGAN/45/2015",         Vaccine::Current},
        }},
    {"A(H3N2)", {
            {"BRISBANE/10/2007",         Vaccine::Previous},
            {"PERTH/16/2009",            Vaccine::Previous},
            {"VICTORIA/361/2011",        Vaccine::Previous},
            {"TEXAS/50/2012",            Vaccine::Previous},
            {"SWITZERLAND/9715293/2013", Vaccine::Previous},
            {"HONG KONG/4801/2014",      Vaccine::Current},
            {"SAITAMA/103/2014",         Vaccine::Surrogate},
            {"HONG KONG/7295/2014",      Vaccine::Surrogate},
        }},
    {"BVICTORIA", {
            {"MALAYSIA/2506/2004",       Vaccine::Previous},
            {"BRISBANE/60/2008",         Vaccine::Current},
            {"PARIS/1762/2009",          Vaccine::Current},
            {"SOUTH AUSTRALIA/81/2012",  Vaccine::Surrogate},
        }},
    {"BYAMAGATA", {
            {"FLORIDA/4/2006",           Vaccine::Previous},
            {"WISCONSIN/1/2010",         Vaccine::Previous},
            {"MASSACHUSETTS/2/2012",     Vaccine::Previous},
            {"PHUKET/3073/2013",         Vaccine::Current},
        }},
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

static inline std::string vaccine_type_as_string(Vaccine::Type aType)
{
    switch (aType) {
      case Vaccine::Previous:
          return "previous";
      case Vaccine::Current:
          return "current";
      case Vaccine::Surrogate:
          return "surrogate";
    }
    return {};
}

// ----------------------------------------------------------------------

const std::vector<Vaccine>& vaccines(std::string aSubtype, std::string aLineage)
{
    return sVaccines.at(aSubtype + aLineage);

} // vaccines

// ----------------------------------------------------------------------

const std::vector<Vaccine>& vaccines(const Chart& aChart)
{
    return vaccines(aChart.chart_info().virus_type(), aChart.lineage());

} // vaccines

// ----------------------------------------------------------------------

std::string Vaccine::type_as_string() const
{
    return vaccine_type_as_string(type);

} // Vaccine::type_as_string

// ----------------------------------------------------------------------

inline bool Vaccines::Entry::operator < (const Vaccines::Entry& a) const
{
    const auto a_nt = a.antigen_data->number_of_tables(), t_nt = antigen_data->number_of_tables();
    return t_nt == a_nt ? most_recent_table_date > a.most_recent_table_date : t_nt > a_nt;
}

// ----------------------------------------------------------------------

bool Vaccines::HomologousSerum::operator < (const Vaccines::HomologousSerum& a) const
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

} // Vaccines::HomologousSerum::operator <

// ----------------------------------------------------------------------

size_t Vaccines::HomologousSerum::number_of_tables() const
{
    return serum_data->number_of_tables();

} // Vaccines::HomologousSerum::number_of_tables

// ----------------------------------------------------------------------

void Vaccines::add(size_t aAntigenIndex, const Antigen& aAntigen, const hidb::AntigenSerumData<Antigen>* aAntigenData, std::vector<HomologousSerum>&& aSera, std::string aMostRecentTableDate)
{
    if (aAntigen.is_reassortant())
        mReassortant.emplace_back(aAntigenIndex, &aAntigen, aAntigenData, std::move(aSera), aMostRecentTableDate);
    else if (aAntigen.is_egg())
        mEgg.emplace_back(aAntigenIndex, &aAntigen, aAntigenData, std::move(aSera), aMostRecentTableDate);
    else
        mCell.emplace_back(aAntigenIndex, &aAntigen, aAntigenData, std::move(aSera), aMostRecentTableDate);

} // Vaccines::add

// ----------------------------------------------------------------------

void Vaccines::sort()
{
    std::sort(mCell.begin(), mCell.end());
    std::sort(mEgg.begin(), mEgg.end());
    std::sort(mReassortant.begin(), mReassortant.end());

} // Vaccines::sort

// ----------------------------------------------------------------------

std::string Vaccines::report(size_t aIndent) const
{
    const std::string indent(aIndent, ' ');
    std::ostringstream out;
    auto entry_report = [&out,&indent](size_t aNo, const auto& entry) {
        out << indent << "  " << aNo << ' ' << entry.antigen->full_name() << " tables:" << entry.antigen_data->number_of_tables() << " recent:" << entry.antigen_data->most_recent_table().table_id() << std::endl;
        for (const auto& hs: entry.homologous_sera)
            out << indent << "    " << hs.serum->full_name() << " tables:" << hs.serum_data->number_of_tables() << " recent:" << hs.serum_data->most_recent_table().table_id() << std::endl;
    };

    if (!mCell.empty()) {
        out << indent << "Vaccine " << type_as_string() << " Cell (" << mCell.size() << ')' << std::endl;
        for (size_t no = 0; no < mCell.size(); ++no)
            entry_report(no, mCell[no]);
    }

    if (!mEgg.empty()) {
        out << indent << "Vaccine " << type_as_string() << " Egg (" << mEgg.size() << ')' << std::endl;
        for (size_t no = 0; no < mEgg.size(); ++no)
            entry_report(no, mEgg[no]);
    }

    if (!mReassortant.empty()) {
        out << indent << "Vaccine " << type_as_string() << " Reassortant (" << mReassortant.size() << ')' << std::endl;
        for (size_t no = 0; no < mReassortant.size(); ++no)
            entry_report(no, mReassortant[no]);
    }
    return out.str();

} // Vaccines::report

// ----------------------------------------------------------------------

std::string Vaccines::type_as_string() const
{
    return vaccine_type_as_string(mType);

} // Vaccines::type_as_string

// ----------------------------------------------------------------------

void vaccines_for_name(Vaccines& aVaccines, std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb)
{
    std::vector<size_t> by_name;
    aChart.antigens().find_by_name(aName, by_name);
    for (size_t ag_no: by_name) {
        try {
            const Antigen& ag = aChart.antigens()[ag_no];
              // std::cerr << ag.full_name() << std::endl;
            const auto& data = aHiDb.find_antigen_of_chart(ag);
            std::vector<Vaccines::HomologousSerum> homologous_sera;
            for (const auto* sd: aHiDb.find_homologous_sera(data)) {
                const size_t sr_no = aChart.sera().find_by_name_for_exact_matching(hidb::name_for_exact_matching(sd->data()));
                  // std::cerr << "   " << sd->data().name_for_exact_matching() << " " << (serum ? "Y" : "N") << std::endl;
                if (sr_no != static_cast<size_t>(-1))
                    homologous_sera.emplace_back(sr_no, &aChart.sera()[sr_no], sd, aHiDb.charts()[sd->most_recent_table().table_id()].chart_info().date());
            }
            aVaccines.add(ag_no, ag, &data, std::move(homologous_sera), aHiDb.charts()[data.most_recent_table().table_id()].chart_info().date());
        }
        catch (hidb::HiDb::NotFound&) {
        }
    }
    aVaccines.sort();

} // vaccines_for_name

// ----------------------------------------------------------------------

Vaccines* find_vaccines_in_chart(std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb)
{
    Vaccines* result = new Vaccines(Vaccine::Previous);
    vaccines_for_name(*result, aName, aChart, aHiDb);
    return result;

} // find_vaccines_in_chart

// ----------------------------------------------------------------------

VaccinesOfChart* vaccines(const Chart& aChart, const hidb::HiDb& aHiDb)
{
    auto* result = new VaccinesOfChart{};
    for (const auto& name_type: vaccines(aChart)) {
        result->emplace_back(name_type.type);
        vaccines_for_name(result->back(), name_type.name, aChart, aHiDb);
    }
    return result;

} // vaccines

// ----------------------------------------------------------------------

std::string VaccinesOfChart::report(size_t aIndent) const
{
    std::string result;
    for (const auto& v: *this)
        result += v.report(aIndent);
    return result;

} // VaccinesOfChart::report

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
