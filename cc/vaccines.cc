#include "acmacs-chart/chart.hh"
#include "hidb.hh"
#include "variant-id.hh"
#include "vaccines.hh"

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
        out << indent << "CELL" << std::endl;
        for (size_t no = 0; no < mCell.size(); ++no)
            entry_report(no, mCell[no]);
    }

    if (!mEgg.empty()) {
        out << indent << "EGG" << std::endl;
        for (size_t no = 0; no < mEgg.size(); ++no)
            entry_report(no, mEgg[no]);
    }

    if (!mReassortant.empty()) {
        out << indent << "REASSORTANT" << std::endl;
        for (size_t no = 0; no < mReassortant.size(); ++no)
            entry_report(no, mReassortant[no]);
    }
    return out.str();

} // Vaccines::report

// ----------------------------------------------------------------------

Vaccines* find_vaccines_in_chart(std::string aName, const Chart& aChart, const hidb::HiDb& aHiDb)
{
    Vaccines* result = new Vaccines();
    std::vector<size_t> by_name;
    aChart.antigens().find_by_name(aName, by_name);
    for (size_t ag_no: by_name) {
          // std::cerr << ag->full_name() << std::endl;
        try {
            const Antigen& ag = aChart.antigens()[ag_no];
            const auto& data = aHiDb.find_antigen_of_chart(ag);
            std::vector<Vaccines::HomologousSerum> homologous_sera;
            for (const auto* sd: aHiDb.find_homologous_sera(data)) {
                const size_t sr_no = aChart.sera().find_by_name_for_exact_matching(hidb::name_for_exact_matching(sd->data()));
                  // std::cerr << "   " << sd->data().name_for_exact_matching() << " " << (serum ? "Y" : "N") << std::endl;
                if (sr_no != static_cast<size_t>(-1))
                    homologous_sera.emplace_back(sr_no, &aChart.sera()[sr_no], sd, aHiDb.charts()[sd->most_recent_table().table_id()].chart_info().date());
            }
            result->add(ag_no, ag, &data, std::move(homologous_sera), aHiDb.charts()[data.most_recent_table().table_id()].chart_info().date());
        }
        catch (hidb::HiDb::NotFound&) {
        }
    }
    result->sort();
    result->report();
    return result;

} // find_vaccines_in_chart

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
