#include <stack>

#include "rapidjson/reader.h"

#include "hidb-export.hh"
#include "read-file.hh"

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const HiDb& aHiDb)
{
    JsonWriter writer("hidb");
    writer.StartObject();
    writer.Key("  version");
    writer.String("hidb-v4");
    writer << JsonKey::Antigens << aHiDb.antigens() << JsonKey::Sera << aHiDb.sera() << JsonKey::Tables << aHiDb.charts() << EndObject;
    write_file(aFilename, writer);
}

// ----------------------------------------------------------------------

void hidb_export_pretty(std::string aFilename, const HiDb& aHiDb)
{
    JsonPrettyWriter writer("hidb");

    writer.StartObject();
    writer.Key("  version");
    writer.String("hidb-v4");
    // writer.Key("?created");
    // writer.String("hidb on");
    writer << JsonKey::Antigens << aHiDb.antigens() << JsonKey::Sera << aHiDb.sera() << JsonKey::Tables << aHiDb.charts() << EndObject;

    write_file(aFilename, writer);

} // hidb_export_pretty

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

class Error : public std::runtime_error { public: using std::runtime_error::runtime_error; };

class HiDbReaderEventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, HiDbReaderEventHandler>
{
 private:
    enum class State : unsigned
    {
        Ignore, Init, Root, Version, // 0-3
        Antigens, Sera, Antigen, Serum, PerTableList, PerTable, // 4-9
        Tables, Table, TableAntigens, TableAntigenList, TableSera, TableSerumList, TableAntigenSerumRef, TableTiters, TableTiterRows, // TableTiterRow, // 10-
        StringField, StringListField,
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
    inline HiDbReaderEventHandler(HiDb& aHiDb)
        : mHiDb(aHiDb), ignore_compound(0),
          string_to_fill(nullptr),
            // bool_to_fill(nullptr), int_to_fill(nullptr), double_to_fill(nullptr),
          ag_sr_ref_to_fill(nullptr),
          vector_string_to_fill(nullptr), antigen_to_fill(nullptr), serum_to_fill(nullptr), per_table_list(nullptr)
        { state.push(State::Init); }

    inline bool transit(char input, Arg arg = Arg())
        {
              // std::cerr << "transit.ch " << static_cast<unsigned>(input) << std::endl;
            if (input < input_base)
                return false;
            return (this->*transition[static_cast<unsigned>(state.top())][input - input_base])(arg);
        }

    inline bool transit(Tr input, Arg arg = Arg())
        {
              // std::cerr << "transit.Tr " << static_cast<char>(static_cast<char>(input) + input_base) << ' ' << static_cast<unsigned>(state.top()) << std::endl;
            return (this->*transition[static_cast<unsigned>(state.top())][static_cast<unsigned>(input)])(arg);
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
    inline bool Bool(bool b) { return transit(Tr::Bool, b); }

    inline bool Int(int /*i*/) { return false; }
    bool Double(double /*d*/) { return false; }
    // bool Uint(unsigned u) { return Int(static_cast<int>(u)); }

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
    std::string* string_to_fill;
    // bool* bool_to_fill;
    // int* int_to_fill;
    // double* double_to_fill;
    ChartData::AgSrRef* ag_sr_ref_to_fill;
    std::vector<std::string>* vector_string_to_fill;
    AntigenData* antigen_to_fill;
    SerumData* serum_to_fill;
    std::vector<PerTable>* per_table_list;

      // ----------------------------------------------------------------------

    bool start_hidb(Arg=Arg()) { state.push(State::Root); return true; }
    bool start_ignore(Arg=Arg()) { state.push(State::Ignore); return true; }
    bool start_version(Arg=Arg()) { state.push(State::Version); return true; }
    bool pop(Arg=Arg()) { state.pop(); return true; }

      // ----------------------------------------------------------------------

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

    bool start_antigens(Arg=Arg()) { state.push(State::Antigens); return true; }
    bool start_sera(Arg=Arg()) { state.push(State::Sera); return true; }
    bool start_antigen(Arg) { state.push(State::Antigen); auto& antigens = mHiDb.antigens(); antigens.emplace_back(); antigen_to_fill = &antigens.back(); return true; }
    bool start_serum(Arg) { state.push(State::Serum); auto& sera = mHiDb.sera(); sera.emplace_back(); serum_to_fill = &sera.back(); return true; }

    bool antigen_name(Arg) { state.push(State::StringField); string_to_fill = &antigen_to_fill->data().name(); return true; }
    bool antigen_lineage(Arg) { state.push(State::StringField); string_to_fill = &antigen_to_fill->data().lineage(); return true; }
    bool antigen_passage(Arg) { state.push(State::StringField); string_to_fill = &antigen_to_fill->data().passage(); return true; }
    bool antigen_reassortant(Arg) { state.push(State::StringField); string_to_fill = &antigen_to_fill->data().reassortant(); return true; }
    bool antigen_annotations(Arg) { state.push(State::StringListField); vector_string_to_fill = &antigen_to_fill->data().annotations(); return true; }
    bool antigen_per_table(Arg) { state.push(State::PerTableList); per_table_list = &antigen_to_fill->per_table(); return true; }

    bool serum_name(Arg) { state.push(State::StringField); string_to_fill = &serum_to_fill->data().name(); return true; }
    bool serum_lineage(Arg) { state.push(State::StringField); string_to_fill = &serum_to_fill->data().lineage(); return true; }
    bool serum_passage(Arg) { state.push(State::StringField); string_to_fill = &serum_to_fill->data().passage(); return true; }
    bool serum_reassortant(Arg) { state.push(State::StringField); string_to_fill = &serum_to_fill->data().reassortant(); return true; }
    bool serum_annotations(Arg) { state.push(State::StringListField); vector_string_to_fill = &serum_to_fill->data().annotations(); return true; }
    bool serum_id(Arg) { state.push(State::StringField); string_to_fill = &serum_to_fill->data().serum_id(); return true; }
    bool serum_species(Arg) { state.push(State::StringField); string_to_fill = &serum_to_fill->data().serum_species(); return true; }
    bool serum_per_table(Arg) { state.push(State::PerTableList); per_table_list = &serum_to_fill->per_table(); return true; }

    bool per_table(Arg) { state.push(State::PerTable); per_table_list->emplace_back(); return true; }
    bool per_table_id(Arg) { state.push(State::StringField); string_to_fill = &per_table_list->back().table_id(); return true; } // T
    bool per_table_date(Arg) { state.push(State::StringField); string_to_fill = &per_table_list->back().date(); return true; } // D
    bool per_table_lab(Arg) { state.push(State::StringListField); vector_string_to_fill = &per_table_list->back().lab_id(); return true; } // l
    bool per_table_homologous(Arg) { state.push(State::StringField); string_to_fill = &per_table_list->back().homologous(); return true; } // h

      // ----------------------------------------------------------------------

    bool start_tables(Arg) { state.push(State::Tables); return true; }
    bool start_table(Arg) { mHiDb.charts().emplace_back(); state.push(State::Table); return true; }
    bool table_table_id(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().table_id(); return true; }
    bool table_virus(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().virus(); return true; }
    bool table_virus_type(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().virus_type(); return true; }

    bool table_assay(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().assay(); return true; }
    bool table_date(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().date(); return true; }
    bool table_lab(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().lab(); return true; }
    bool table_rbc(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().rbc(); return true; }
    bool table_name(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().name(); return true; }
    bool table_subset(Arg) { state.push(State::StringField); string_to_fill = &mHiDb.charts().back().chart_info().subset(); return true; }

    bool start_table_antigens(Arg) { state.push(State::TableAntigens); return true; }
    bool start_table_sera(Arg) { state.push(State::TableSera); return true; }
    bool start_table_antigen_list(Arg) { state.pop(); state.push(State::TableAntigenList); return true; }
    bool start_table_serum_list(Arg) { state.pop(); state.push(State::TableSerumList); return true; }
    bool start_table_antigen(Arg) { state.push(State::TableAntigenSerumRef); auto& ags = mHiDb.charts().back().antigens(); ags.emplace_back(); ag_sr_ref_to_fill = &ags.back(); return true; }
    bool start_table_serum(Arg) { state.push(State::TableAntigenSerumRef); auto& srs = mHiDb.charts().back().sera(); srs.emplace_back(); ag_sr_ref_to_fill = &srs.back(); return true; }
    bool start_table_titers(Arg) { state.push(State::TableTiters); return true; }
    bool start_table_titer_rows(Arg) { state.pop(); state.push(State::TableTiterRows); return true; }
    bool start_table_titer_row(Arg) { state.push(State::StringListField); auto& titers = mHiDb.charts().back().titers(); titers.emplace_back(); vector_string_to_fill = &titers.back(); return true; }

    bool table_ag_sr(Arg arg)
        {
            bool r = true;
            if (ag_sr_ref_to_fill->first.empty()) {
                ag_sr_ref_to_fill->first.assign(arg.mStr.str, arg.mStr.length);
            }
            else if (ag_sr_ref_to_fill->second.empty()) {
                ag_sr_ref_to_fill->second.assign(arg.mStr.str, arg.mStr.length);
            }
            else {
                r = false;
            }
            return r;
        }

      // ----------------------------------------------------------------------

    bool version(Arg arg)
        {
            if (std::memcmp(arg.mStr.str, "hidb-v4", arg.mStr.length))
                throw Error("Unsupported version: " + std::string(arg.mStr.str, arg.mStr.length));
            state.pop();
            return true;
        }

    bool str_assign(Arg arg)
        {
            string_to_fill->assign(arg.mStr.str, arg.mStr.length);
              // string_to_fill = nullptr;
            state.pop();
            return true;
        }

    bool str_append(Arg arg)
        {
            vector_string_to_fill->emplace_back(arg.mStr.str, arg.mStr.length);
            return true;
        }

      // ----------------------------------------------------------------------

    static const Ptr transition[][62];

    friend void hidb_import(std::string, HiDb&);

};

// ----------------------------------------------------------------------

typedef HiDbReaderEventHandler H;
constexpr static H::Ptr N = &H::nope;
constexpr static H::Ptr F = &H::fail;

const HiDbReaderEventHandler::Ptr HiDbReaderEventHandler::transition[][62] = {
  // A,               B, C, D,                  E, F, G, H, I,            J, K, L,                   M, N,                O, P,                   Q, R,                       S, T,                     U, V,                    W, X, Y, Z, [,                            \ (int),    ],              ^ (bool),   _ (str),         `, a,                        b, c, d, e, f, g, h,                        i, j, k, l,                 m, n, o, p, q, r,             s,                    t,                      u, v,               w, x, y, z, {,                 | (double), },              ~ (version)
    {N,               N, N, N,                  N, N, N, N, N,            N, N, N,                   N, N,                N, N,                   N, N,                       N, N,                     N, N,                    N, N, N, N, &H::push_ignore,              &H::ignore, &H::pop_ignore, &H::ignore, &H::ignore,      F, N,                        N, N, N, N, N, N, N,                        N, N, N, N,                 N, N, N, N, N, N,             N,                    N,                      N, N,               N, N, N, N, &H::push_ignore,   &H::ignore, &H::pop_ignore, N}, // Ignore
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, F,                            F,          F,              F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, &H::start_hidb,    F,          F,              F}, // Init
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, F,                            F,          F,              F,          F,               F, &H::start_antigens,       F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             &H::start_sera,       &H::start_tables,       F, F,               F, F, F, F, F,                 F,          &H::pop,        &H::start_version}, // Root
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, F,                            F,          F,              F,          &H::version,     F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // Version

    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, N,                            F,          &H::pop,        F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, &H::start_antigen, F,          F,              F}, // Antigens
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, N,                            F,          &H::pop,        F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, &H::start_serum,   F,          F,              F}, // Sera
    {F,               F, F, F,                  F, F, F, F, F,            F, F, &H::antigen_lineage, F, &H::antigen_name, F, &H::antigen_passage, F, &H::antigen_reassortant, F, &H::antigen_per_table, F, F,                    F, F, F, F, F,                            F,          F,              F,          F,               F, &H::antigen_annotations,  F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          &H::pop,        F}, // Antigen
    {F,               F, F, F,                  F, F, F, F, &H::serum_id, F, F, &H::serum_lineage,   F, &H::serum_name,   F, &H::serum_passage,   F, &H::serum_reassortant,   F, &H::serum_per_table,   F, F,                    F, F, F, F, F,                            F,          F,              F,          F,               F, &H::serum_annotations,    F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             &H::serum_species,    F,                      F, F,               F, F, F, F, F,                 F,          &H::pop,        F}, // Serum
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, N,                            F,          &H::pop,        F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, &H::per_table,     F,          F,              F}, // PerTableList
    {F,               F, F, &H::per_table_date, F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, &H::per_table_id,      F, F,                    F, F, F, F, F,                            F,          F,              F,          F,               F, F,                        F, F, F, F, F, F, &H::per_table_homologous, F, F, F, &H::per_table_lab, F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          &H::pop,        F}, // PerTable

    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, N,                            F,          &H::pop,        F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, &H::start_table,   F,          F,              F}, // Tables
    {&H::table_assay, F, F, &H::table_date,     F, F, F, F, F,            F, F, F,                   F, &H::table_name,   F, F,                   F, F,                       F, &H::table_table_id,    F, &H::table_virus_type, F, F, F, F, F,                            F,          F,              F,          F,               F, &H::start_table_antigens, F, F, F, F, F, F, F,                        F, F, F, &H::table_lab,     F, F, F, F, F, &H::table_rbc, &H::start_table_sera, &H::start_table_titers, F, &H::table_virus, F, F, F, F, F,                 F,          &H::pop,        F}, // Table
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, &H::start_table_antigen_list, F,          F,              F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // TableAntigens
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, &H::start_table_antigen,      F,          &H::pop,        F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // TableAntigenList
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, &H::start_table_serum_list,   F,          F,              F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // TableSera
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, &H::start_table_serum,        F,          &H::pop,        F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // TableSerumList
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, F,                            F,          &H::pop,        F,          &H::table_ag_sr, F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // TableAntigenSerumRef
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, &H::start_table_titer_rows,   F,          F,              F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // TableTiters
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, &H::start_table_titer_row,    F,          &H::pop,        F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // TableTiterRows
      //               {F, F, F                 , F, F, F, F, F           , F, F, F                  , F, F, F,                F                  , F, F                      , F, F, F,                     F                   , F, F, F, F, F, F,                            F,          &H::pop,        F,          &H::str_append,  F, F,                        F, F, F, F, F, F                       , F, F, F, F                , F, F, F, F, F, F            , F, F,                    F,                      F              , F, F, F, F, F, F,                 F,          F,              F}, // TableTiterRow

    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, F,                            F,          F,              F,          &H::str_assign,  F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // StringField
    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, N,                            F,          &H::pop,        F,          &H::str_append,  F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // StringListField

    {F,               F, F, F,                  F, F, F, F, F,            F, F, F,                   F, F,                F, F,                   F, F,                       F, F,                     F, F,                    F, F, F, F, F,                            F,          F,              F,          F,               F, F,                        F, F, F, F, F, F, F,                        F, F, F, F,                 F, F, F, F, F, F,             F,                    F,                      F, F,               F, F, F, F, F,                 F,          F,              F}, // Last
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
            throw Error("cannot import hidb: data parsing failed at state " + std::to_string(static_cast<unsigned>(handler.state.top())) + " at pos " + std::to_string(reader.GetErrorOffset()) + ": " +  GetParseError_En(reader.GetParseErrorCode()) + "\n" + buffer.substr(reader.GetErrorOffset(), 50));
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
