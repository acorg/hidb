#include <stack>

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

class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class HiDbReaderEventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, HiDbReaderEventHandler>
{
 private:
    enum class State : unsigned
    {
        Ignore, Init, Root, Version,
        Antigens, Sera, Tables,
    };

    union Arg
    {
        inline Arg() {}
        inline Arg(const char* str, rapidjson::SizeType length) : mStr({str, length}) {}
        inline Arg(int aInt) : mInt(aInt) {}
        inline Arg(bool aBool) : mBool(aBool) {}
        inline Arg(double aDouble) : mDouble(aDouble) {}

        struct {
            const char* str;
            rapidjson::SizeType length;
        } mStr;
        int mInt;
        bool mBool;
        double mDouble;
    };

    static constexpr char input_base = 'A';

    enum class Tr : char { StartArray='['-input_base, EndArray=']'-input_base, StartObject='{'-input_base, EndObject='}'-input_base, String='_'-input_base, Bool='^'-input_base, Int='\\'-input_base, Double='|'-input_base, Version='~'-input_base };

 public:
    inline HiDbReaderEventHandler(HiDb& aHiDb) : mHiDb(aHiDb), ignore_compound(0) { state.push(State::Init); }

    inline bool transit(char input, Arg arg = Arg())
        {
              // std::cerr << "transit.ch " << static_cast<unsigned>(input) << std::endl;
            if (input < input_base)
                return false;
            return (this->*transition[static_cast<unsigned>(state.top())][input - input_base])(arg);
        }

    inline bool transit(Tr input, Arg arg = Arg())
        {
              // std::cerr << "transit.Tr " << static_cast<unsigned>(input) << ' ' << static_cast<unsigned>(state.top()) << std::endl;
            return (this->*transition[static_cast<unsigned>(state.top())][static_cast<char>(input)])(arg);
        }

    inline bool StartObject() { return transit(Tr::StartObject); }
    inline bool EndObject(rapidjson::SizeType /*memberCount*/) { return transit(Tr::EndObject); }
    inline bool StartArray() { return transit(Tr::StartArray); }
    inline bool EndArray(rapidjson::SizeType /*elementCount*/) { return transit(Tr::EndArray); }

    inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/)
        {
            switch (length) {
              case 0:
                  return false;
              case 1:           // "_", "?"
                  return transit(*str);
              default:
                  if (static_cast<JsonKey>(str[0]) == JsonKey::Comment)
                      return start_ignore();
                  else if (std::string(str, length) == "  version")
                      return transit(Tr::Version);
                  else
                      return fail();
            }
              // return false;
        }

    inline bool String(const char* str, rapidjson::SizeType length, bool /*copy*/) { return transit(Tr::String, {str, length}); }

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

    typedef bool (HiDbReaderEventHandler::*Ptr)(Arg arg);
    bool nope(Arg=Arg()) { return true; }
    bool fail(Arg=Arg()) { return false; }
    bool in_init_state() const { return state.top() == State::Init; }

 private:
    HiDb& mHiDb;
    std::stack<State> state;
    size_t ignore_compound;

      // ----------------------------------------------------------------------

    bool start_hidb(Arg=Arg()) { state.push(State::Root); return true; }
    bool start_ignore(Arg=Arg()) { state.push(State::Ignore); return true; }
    bool start_version(Arg=Arg()) { state.push(State::Version); return true; }
    bool pop(Arg=Arg()) { state.pop(); return true; }

    bool push_ignore(Arg=Arg())
        {
            ++ignore_compound;
            return true;
        }

    bool pop_ignore(Arg=Arg())
        {
            if (ignore_compound == 0)
                return false;
            --ignore_compound;
            if (ignore_compound == 0)
                state.pop();
            return true;
        }

    bool ignore(Arg=Arg())
        {
            if (ignore_compound == 0)
                state.pop();
            return true;
        }

      // ----------------------------------------------------------------------

    bool version(Arg arg)
        {
            if (std::memcmp(arg.mStr.str, "hidb-v4", arg.mStr.length))
                throw Error("Unsupported version: " + std::string(arg.mStr.str, arg.mStr.length));
            state.pop();
            return true;
        }

      // ----------------------------------------------------------------------

    static const Ptr transition[][62];
};

// ----------------------------------------------------------------------

typedef HiDbReaderEventHandler H;
constexpr static H::Ptr N = &H::nope;
constexpr static H::Ptr F = &H::fail;

const HiDbReaderEventHandler::Ptr HiDbReaderEventHandler::transition[][62] = {
  // A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, [,               \ (int),    ],              ^ (bool),   _ (str),     `, a,                b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s,                t,                u, v, w, x, y, z, {,               | (double), },              ~ (version)
    {N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, &H::push_ignore, &H::ignore, &H::pop_ignore, &H::ignore, &H::ignore,  F, N,                N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N, N,                N,                N, N, N, N, N, N, &H::push_ignore, &H::ignore, &H::pop_ignore, N}, // Ignore
    {F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,               F,          F,              F,          F,           F, F,                F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,                F,                F, F, F, F, F, F, &H::start_hidb,  F,          F,              F}, // Init
    {F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,               F,          F,              F,          F,           F, &H::start_ignore, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, &H::start_ignore, &H::start_ignore, F, F, F, F, F, F, F,               F,          &H::pop,        &H::start_version}, // Root
    {F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,               F,          F,              F,          &H::version, F, F,                F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,                F,                F, F, F, F, F, F, F,               F,          F,              F}, // Version
    {F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,               F,          F,              F,          F,           F, F,                F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,                F,                F, F, F, F, F, F, F,               F,          F,              F}, // Antigens
    {F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,               F,          F,              F,          F,           F, F,                F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,                F,                F, F, F, F, F, F, F,               F,          F,              F}, // Sera
    {F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,               F,          F,              F,          F,           F, F,                F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F, F,                F,                F, F, F, F, F, F, F,               F,          F,              F}, // Tables
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
            throw Error("cannot import hidb: data parsing failed at " + std::to_string(reader.GetErrorOffset()) + ": " +  GetParseError_En(reader.GetParseErrorCode()) + "\n" + buffer.substr(reader.GetErrorOffset(), 50));
        if (!handler.in_init_state())
            throw Error("internal: not in init state on parsing completion");
    }
    else
        throw std::runtime_error("cannot import hidb: unrecognized source format");

} // hidb_import

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
