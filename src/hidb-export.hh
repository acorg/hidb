#pragma once

#include "hidb.hh"

// ----------------------------------------------------------------------

enum _StartArray { StartArray };
enum _EndArray { EndArray };
enum _StartObject { StartObject };
enum _EndObject { EndObject };

template <typename Writer> inline Writer& operator <<(Writer& writer, _StartArray) { writer.StartArray(); return writer; }
template <typename Writer> inline Writer& operator <<(Writer& writer, _EndArray) { writer.EndArray(); return writer; }
template <typename Writer> inline Writer& operator <<(Writer& writer, _StartObject) { writer.StartObject(); return writer; }
template <typename Writer> inline Writer& operator <<(Writer& writer, _EndObject) { writer.EndObject(); return writer; }

template <typename Writer> inline Writer& operator <<(Writer& writer, char key) { writer.Key(&key, 1, false); return writer; }
template <typename Writer> inline Writer& operator <<(Writer& writer, std::string s) { writer.String(s.c_str(), static_cast<unsigned>(s.size())); return writer; }

template <typename Writer> inline Writer& operator <<(Writer& writer, const std::vector<std::string>& list_strings)
{
    writer << StartArray;
    for (const auto& e: list_strings)
        writer << e;
    return writer << EndArray;
}

template <typename Writer> inline Writer& operator <<(Writer& writer, const std::vector<std::vector<std::string>>& list_list_strings)
{
    writer << StartArray;
    for (const auto& e: list_list_strings)
        writer << e;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename Value> class _if_not_empty
{
 public:
    inline _if_not_empty(char key, Value value) : mKey(key), mValue(value) {}

    template <typename Writer> friend inline Writer& operator <<(Writer& writer, const _if_not_empty<Value>& data)
        {
            if (!data.mValue.empty())
                writer << data.mKey << data.mValue;
            return writer;
        }

 private:
    char mKey;
    Value mValue;
};

template <typename Value> _if_not_empty<Value> if_not_empty(char key, Value value) { return _if_not_empty<Value>(key, value); }

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const std::vector<AntigenData>& antigens)
{
    writer << 'a' << StartArray;
    for (const auto& data: antigens)
        writer << data;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const std::vector<SerumData>& sera)
{
    writer << 's' << StartArray;
    for (const auto& data: sera)
        writer << data;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const std::vector<ChartData>& charts)
{
    writer << 't' << StartArray;
    for (const auto& data: charts)
        writer << data;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename Writer, typename AS> inline Writer& operator <<(Writer& writer, const AntigenSerumData<AS>& ag_sr)
{
    return writer << StartObject << ag_sr.as() << ag_sr.per_table() << EndObject;
}

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const Antigen& antigen)
{
    return writer << 'N' << antigen.name() << if_not_empty('L', antigen.lineage())
                  << if_not_empty('P', antigen.passage()) << if_not_empty('R', antigen.reassortant())
                  << if_not_empty('a', antigen.annotations());
}

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const Serum& serum)
{
    return writer << 'N' << serum.name() << if_not_empty('L', serum.lineage())
                  << if_not_empty('P', serum.passage()) << if_not_empty('R', serum.reassortant())
                  << if_not_empty('I', serum.serum_id()) << if_not_empty('a', serum.annotations()) << if_not_empty('s', serum.serum_species());
}

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const std::map<std::string, PerTable>& per_table)
{
    writer << 'T' << StartObject;
    for (const auto& table_data: per_table) {
        writer.Key(table_data.first.c_str(), static_cast<unsigned>(table_data.first.size()));
        writer << StartObject << if_not_empty('D', table_data.second.date()) << if_not_empty('l', table_data.second.lab_id()) << if_not_empty('h', table_data.second.homologous()) << EndObject;
    }
    return writer << EndObject;
}

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const std::vector<ChartData::AgSrRef>& refs)
{
    writer << StartArray;
    for (const auto& ref: refs)
        writer << StartArray << ref.first << ref.second << EndArray;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename Writer> inline Writer& operator <<(Writer& writer, const ChartData& chart)
{
    return writer << StartObject << 'T' << chart.table_id() << 'a' << chart.antigens() << 's' << chart.sera() << 't' << chart.titers() << EndObject;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
