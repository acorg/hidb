#include <stack>

#include "acmacs-base/read-file.hh"
#include "acmacs-chart-1/chart.hh"
#include "hidb-export.hh"
#include "hidb-4/hidb.hh"
#include "hidb-4/json-keys.hh"

// ----------------------------------------------------------------------

namespace json_writer
{
    template <typename RW> class writer;
}
namespace jsw = json_writer;

template <typename RW> jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::HiDb& aHiDb);
template <typename RW> jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::ChartData& chart);
template <typename RW> jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::ChartData::AgSrRef& ref);

#include "acmacs-base/json-writer.hh"

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::PerTable& per_table)
{
    return writer << jsw::start_object << JsonKey::TableId << per_table.table_id() << jsw::if_not_empty(JsonKey::Date, per_table.date())
                  << jsw::if_not_empty(JsonKey::LabId, per_table.lab_id()) << jsw::if_not_empty(JsonKey::HomologousAntigen, per_table.homologous()) << jsw::end_object;
}

// ----------------------------------------------------------------------

template <typename RW> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>& aWriter, const std::vector<hidb::PerTable>& aList)
{
    aWriter << json_writer::start_array;
    for (const auto& e: aList)
        aWriter << e;
    return aWriter << json_writer::end_array;
}

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::AntigenSerumData<Antigen>& aAntigenData)
{
    const Antigen& antigen = aAntigenData.data();
    return writer << jsw::start_object
                  << jsw::key("N") << antigen.name()
                  << jsw::if_not_empty("L", antigen.lineage())
                  << jsw::if_not_empty("P", antigen.passage())
                  << jsw::if_not_empty("R", antigen.reassortant())
                  << jsw::if_not_empty("a", antigen.annotations())
                  << JsonKey::PerTable << aAntigenData.per_table()
                  << jsw::end_object;
}

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::AntigenSerumData<Serum>& aSerumData)
{
    const Serum& serum = aSerumData.data();
    return writer << jsw::start_object
                  << jsw::key("N") << serum.name()
                  << jsw::if_not_empty("L", serum.lineage())
                  << jsw::if_not_empty("P", serum.passage())
                  << jsw::if_not_empty("R", serum.reassortant())
                  << jsw::if_not_empty("a", serum.annotations())
                  << jsw::if_not_empty("I", serum.serum_id())
                  << jsw::if_not_empty("s", serum.serum_species())
                  << JsonKey::PerTable << aSerumData.per_table()
                  << jsw::end_object;
}

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::ChartData::AgSrRef& ref)
{
    return writer << jsw::start_array << ref.first << ref.second << jsw::end_array;
}

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::ChartData& chart)
{
    return writer << jsw::start_object
                  << JsonKey::TableId << chart.table_id()
                  << jsw::if_not_empty(JsonKey::Virus, chart.chart_info().virus())
                  << jsw::if_not_empty(JsonKey::VirusType, chart.chart_info().virus_type())
                  << jsw::if_not_empty(JsonKey::Assay, chart.chart_info().assay())
                  << jsw::if_not_empty(JsonKey::Date, chart.chart_info().date())
                  << jsw::if_not_empty(JsonKey::Lab, chart.chart_info().lab())
                  << jsw::if_not_empty(JsonKey::Rbc, chart.chart_info().rbc())
                  << jsw::if_not_empty(JsonKey::Name, chart.chart_info().name())
              // conflicts with JsonKey::Sera << jsw::if_not_empty(JsonKey::VirusSubset, chart.chart_info().subset())
                  << JsonKey::Antigens << chart.antigens()
                  << JsonKey::Sera << chart.sera()
                  << JsonKey::Titers << chart.titers()
                  << jsw::end_object;
}

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::HiDb& aHiDb)
{
    return writer << jsw::start_object
                  << jsw::key("  version") << "hidb-v4"
                  << JsonKey::Antigens << aHiDb.antigens()
                  << JsonKey::Sera << aHiDb.sera()
                  << JsonKey::Tables << aHiDb.charts()
                  << jsw::end_object;
}

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const hidb::HiDb& aHiDb, size_t aIndent)
{
    jsw::export_to_json(aHiDb, aFilename, aIndent);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
