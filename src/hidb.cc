#include "hidb.hh"
#include "hidb-export.hh"
#include "read-file.hh"

#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"

// ----------------------------------------------------------------------

void HiDb::add(const Chart& aChart)
{
    ChartData chart(aChart);
    auto chart_insert_at = std::lower_bound(mCharts.begin(), mCharts.end(), chart);
    if (chart_insert_at != mCharts.end() && chart_insert_at->table_id() == chart.table_id())
        throw std::runtime_error("Chart " + chart.table_id() + " already in hidb");
    mCharts.insert(chart_insert_at, chart);

    aChart.find_homologous_antigen_for_sera_const();

    const auto table_id = aChart.table_id();
    for (const auto& antigen: aChart.antigens()) {
        add_antigen(antigen, table_id);
    }
    for (const auto& serum: aChart.sera()) {
        add_serum(serum, table_id, aChart.antigens());
    }

    // std::cout << "Chart: antigens:" << aChart.number_of_antigens() << " sera:" << aChart.number_of_sera() << std::endl;
    // std::cout << "HDb: antigens:" << mAntigens.size() << " sera:" << mSera.size() << std::endl;

} // HiDb::add

// ----------------------------------------------------------------------

void HiDb::add_antigen(const Antigen& aAntigen, std::string aTableId)
{
    if (!aAntigen.distinct()) {
        AntigenData antigen_data(aAntigen);
        auto insert_at = std::lower_bound(mAntigens.begin(), mAntigens.end(), aAntigen);
        if (insert_at != mAntigens.end() && *insert_at == antigen_data) {
              // update
              // std::cout << "Common antigen " << aAntigen.full_name() << std::endl;
        }
        else {
            insert_at = mAntigens.insert(insert_at, std::move(antigen_data));
        }
        insert_at->update(aTableId, aAntigen);
    }

} // HiDb::add_antigen

// ----------------------------------------------------------------------

void HiDb::add_serum(const Serum& aSerum, std::string aTableId, const std::vector<Antigen>& aAntigens)
{
    if (!aSerum.distinct()) {
        SerumData serum_data(aSerum);
        auto insert_at = std::lower_bound(mSera.begin(), mSera.end(), aSerum);
        if (insert_at != mSera.end() && *insert_at == serum_data) {
              // update
        }
        else {
            insert_at = mSera.insert(insert_at, std::move(serum_data));
        }
        insert_at->update(aTableId, aSerum);
        if (aSerum.has_homologous())
            insert_at->set_homologous(aTableId, aAntigens[static_cast<size_t>(aSerum.homologous())].variant_id());
    }

} // HiDb::add_serum

// ----------------------------------------------------------------------

void HiDb::exportTo(std::string aFilename) const
{
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
    writer.SetIndent(' ', 1);

    writer.StartObject();
    writer.Key("  version");
    writer.String("hidb-v4");
    // writer.Key("?created");
    // writer.String("hidb on");
    writer << mAntigens << mSera << mCharts;
    writer.EndObject();

    write_file(aFilename, s.GetString());

} // HiDb::exportTo

// ----------------------------------------------------------------------

ChartData::ChartData(const Chart& aChart)
    : mTableId(aChart.table_id()), mTiters(aChart.titers().as_list())
{
    for (const auto& antigen: aChart.antigens()) {
        mAntigens.emplace_back(antigen.name(), antigen.variant_id());
    }
    for (const auto& serum: aChart.sera()) {
        mSera.emplace_back(serum.name(), serum.variant_id());
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
