#pragma once

#include <string>

#include "acmacs-chart/ace.hh"
#include "hidb/hidb.hh"
#include "hidb/json-keys.hh"

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const hidb::HiDb& aHiDb, size_t aIndent);

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

template <typename RW, typename AS> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::AntigenSerumData<AS>& ag_sr)
{
    return writer << jsw::start_object << ag_sr.data() << JsonKey::PerTable << ag_sr.per_table() << jsw::end_object;
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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
