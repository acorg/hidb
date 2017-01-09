#pragma once

#include <string>

#include "hidb.hh"
#include "chart-export.hh"

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const hidb::HiDb& aHiDb, size_t aIndent);

// ----------------------------------------------------------------------

template <typename RW, typename AS> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const hidb::AntigenSerumData<AS>& ag_sr)
{
    return writer << StartObject << ag_sr.data() << JsonKey::PerTable << ag_sr.per_table() << EndObject;
}

// ----------------------------------------------------------------------

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const hidb::ChartData::AgSrRef& ref)
{
    return writer << StartArray << ref.first << ref.second << EndArray;
}

// ----------------------------------------------------------------------

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const hidb::ChartData& chart)
{
    return writer << StartObject
                  << JsonKey::TableId << chart.table_id()
                  << if_not_empty(JsonKey::Virus, chart.chart_info().virus())
                  << if_not_empty(JsonKey::VirusType, chart.chart_info().virus_type())
                  << if_not_empty(JsonKey::Assay, chart.chart_info().assay())
                  << if_not_empty(JsonKey::Date, chart.chart_info().date())
                  << if_not_empty(JsonKey::Lab, chart.chart_info().lab())
                  << if_not_empty(JsonKey::Rbc, chart.chart_info().rbc())
                  << if_not_empty(JsonKey::Name, chart.chart_info().name())
              // conflicts with JsonKey::Sera << if_not_empty(JsonKey::VirusSubset, chart.chart_info().subset())
                  << JsonKey::Antigens << chart.antigens()
                  << JsonKey::Sera << chart.sera()
                  << JsonKey::Titers << chart.titers()
                  << EndObject;
}

// ----------------------------------------------------------------------

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const hidb::PerTable& per_table)
{
    return writer << StartObject << JsonKey::TableId << per_table.table_id() << if_not_empty(JsonKey::Date, per_table.date())
                  << if_not_empty(JsonKey::LabId, per_table.lab_id()) << if_not_empty(JsonKey::HomologousAntigen, per_table.homologous()) << EndObject;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
