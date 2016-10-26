#include "rapidjson/reader.h"

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
// ----------------------------------------------------------------------

class HiDbReaderEventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, HiDbReaderEventHandler>
{
 private:
    enum class State : unsigned
    {
        Ignore, Init, Root, Version,
        Antigens, Sera, Tables,
    };

 public:
    inline HiDbReaderEventHandler(HiDb& aHiDb) : mHiDb(aHiDb) {}

    inline bool StartObject()
        {
            return false;
        }

    inline bool EndObject(rapidjson::SizeType /*memberCount*/)
        {
            return false;
        }

    inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/)
        {
            return false;
        }

    inline bool StartArray()
        {
            return false;
        }

    inline bool EndArray(rapidjson::SizeType /*elementCount*/)
        {
            return false;
        }

    inline bool String(const char* str, rapidjson::SizeType length, bool /*copy*/)
        {
            return false;
        }

    inline bool Bool(bool b)
        {
            return false;
        }

    inline bool Int(int i)
        {
            return false;
        }

    bool Double(double d)
        {
            return false;
        }

    bool Uint(unsigned u)
        {
            return Int(static_cast<int>(u));
        }

    inline bool Null() { std::cout << "Null()" << std::endl; return false; }
    bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return false; }
    bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return false; }

 private:
    HiDb& mHiDb;

    typedef State (HiDbReaderEventHandler::*Ptr)();

    static constexpr const Ptr N = nullptr;
    static constexpr Ptr transition[][58] = { // A - }: StartArray: [, EndArray: ], StartObject: {, EndObject: }, String: _, Bool: ^, Int: \\, Double: |
        {},                                  // Ignore
        {},                     // Init
        {},                     // Root
        {},                     // Version
        {},                     // Antigens
        {},                     // Sera
        {},                     // Tables
    };
};

// ----------------------------------------------------------------------

void hidb_import(std::string buffer, HiDb& aHiDb)
{
    if (buffer == "-")
        buffer = read_stdin();
    else
        buffer = read_file(buffer);
    if (buffer[0] == '{') { // && buffer.find("\"  version\": \"hidb-v4\"") != std::string::npos) {
        HiDbReaderEventHandler handler{aHiDb};
        rapidjson::Reader reader;
        rapidjson::StringStream ss(buffer.c_str());
        reader.Parse(ss, handler);
        if (reader.HasParseError())
            throw std::runtime_error("cannot import hidb: data parsing failed at " + std::to_string(reader.GetErrorOffset()) + ": " +  GetParseError_En(reader.GetParseErrorCode()) + "\n" + buffer.substr(reader.GetErrorOffset(), 50));
    }
    else
        throw std::runtime_error("cannot import hidb: unrecognized source format");

} // hidb_import

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
