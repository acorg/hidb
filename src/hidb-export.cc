#include "hidb-export.hh"
#include "chart-export.hh"
#include "hidb.hh"
#include "json-writer.hh"
#include "read-file.hh"

// ----------------------------------------------------------------------

template <typename AS> inline JsonWriter& operator <<(JsonWriter& writer, const AntigenSerumData<AS>& ag_sr)
{
    return writer << StartObject << ag_sr.as() << JsonKey::PerTable << ag_sr.per_table() << EndObject;
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const ChartData::AgSrRef& ref)
{
    return writer << StartArray << ref.first << ref.second << EndArray;
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const ChartData& chart)
{
    return writer << StartObject << JsonKey::TableId << chart.table_id() << JsonKey::Antigens << chart.antigens()
                  << JsonKey::Sera << chart.sera() << JsonKey::Titers << chart.titers() << EndObject;
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const PerTable& per_table)
{
    return writer << StartObject << JsonKey::TableId << per_table.table_id() << if_not_empty(JsonKey::Date, per_table.date())
                  << if_not_empty(JsonKey::LabId, per_table.lab_id()) << if_not_empty(JsonKey::HomologousAntigen, per_table.homologous()) << EndObject;
}

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const HiDb& aHiDb)
{
    JsonWriter writer("hidb");

    writer.StartObject();
    writer.Key("  version");
    writer.String("hidb-v4");
    // writer.Key("?created");
    // writer.String("hidb on");
    writer << JsonKey::Antigens << aHiDb.antigens() << JsonKey::Sera << aHiDb.sera() << JsonKey::Tables << aHiDb.charts() << EndObject;

    write_file(aFilename, writer);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
