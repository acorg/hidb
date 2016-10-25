#include "hidb-export.hh"
#include "hidb.hh"
#include "json-writer.hh"
#include "read-file.hh"

// ----------------------------------------------------------------------

template <typename AS> inline JsonWriter& operator <<(JsonWriter& writer, const AntigenSerumData<AS>& ag_sr)
{
    return writer << StartObject << ag_sr.as() << 'T' << ag_sr.per_table() << EndObject;
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const ChartData::AgSrRef& ref)
{
    return writer << StartArray << ref.first << ref.second << EndArray;
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const ChartData& chart)
{
    return writer << StartObject << 'T' << chart.table_id() << 'a' << chart.antigens() << 's' << chart.sera() << 't' << chart.titers() << EndObject;
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const Antigen& antigen)
{
    return writer << 'N' << antigen.name() << if_not_empty('L', antigen.lineage())
                  << if_not_empty('P', antigen.passage()) << if_not_empty('R', antigen.reassortant())
                  << if_not_empty('a', antigen.annotations());
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const Serum& serum)
{
    return writer << 'N' << serum.name() << if_not_empty('L', serum.lineage())
                  << if_not_empty('P', serum.passage()) << if_not_empty('R', serum.reassortant())
                  << if_not_empty('I', serum.serum_id()) << if_not_empty('a', serum.annotations()) << if_not_empty('s', serum.serum_species());
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const PerTable& per_table)
{
    return writer << StartObject << 'T' << per_table.table_id()
                  << if_not_empty('D', per_table.date()) << if_not_empty('l', per_table.lab_id()) << if_not_empty('h', per_table.homologous()) << EndObject;
}

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const HiDb& aHiDb)
{
    JsonWriter writer;

    writer.StartObject();
    writer.Key("  version");
    writer.String("hidb-v4");
    // writer.Key("?created");
    // writer.String("hidb on");
    writer << 'a' << aHiDb.antigens() << 's' << aHiDb.sera() << 't' << aHiDb.charts() << EndObject;

    write_file(aFilename, writer);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
