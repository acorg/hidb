#include <stack>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include "ace.hh"
#include "json-keys.hh"
#include "chart.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/xz.hh"

// ----------------------------------------------------------------------

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif

class ChartReaderEventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, ChartReaderEventHandler>
{
 private:
    enum class State { Ignore, Init, Root, Version, Chart, Info, TableType, Antigen, Serum, Titers, Projections, PlotSpec,
                       TitersList, TitersList2, TitersDict, TitersLayers,
                       StringField, BoolField, IntField, DoubleField, VectorStringField,
                       Sources, ColumnBases };

    typedef JsonKey AceKey;

 public:
    inline ChartReaderEventHandler(hidb::Chart* aChart)
        : chart(aChart), ignore_compound(0),
          string_to_fill(nullptr), bool_to_fill(nullptr), int_to_fill(nullptr), double_to_fill(nullptr), vector_string_to_fill(nullptr), layer_to_fill(nullptr)
        { state.push(State::Init); }

    inline bool StartObject()
        {
            bool r = true;
            switch (state.top()) {
              case State::Init:
                  state.push(State::Root);
                  break;
              case State::Chart:
              case State::Info:
              case State::Titers:
              case State::PlotSpec:
                  break;
              case State::Ignore:
                  ++ignore_compound;
                  break;
              case State::Antigen:
                  state.push(State::Antigen);
                  chart->antigens().emplace_back();
                  break;
              case State::Serum:
                  state.push(State::Serum);
                  chart->sera().emplace_back();
                  break;
              case State::Sources:
                  chart->chart_info().sources().emplace_back();
                  break;
              case State::Projections:
                  state.push(State::Ignore);
                  ++ignore_compound;
                  break;
              case State::TitersDict:
                  layer_to_fill->emplace_back();
                  break;
              default:
                  r = false;
                  break;
            }
            return r;
        }

    inline bool EndObject(rapidjson::SizeType /*memberCount*/)
        {
            bool r = true;
            switch (state.top()) {
              case State::Root:
              case State::Antigen:
              case State::Serum:
              case State::Info:
              case State::Chart:
              case State::Titers:
              case State::Projections:
              case State::PlotSpec:
                  state.pop();
                  break;
              case State::Sources:
              case State::TitersDict:
                  break;
              case State::Ignore:
                  if (ignore_compound == 0)
                      r = false;
                  else {
                      --ignore_compound;
                      if (ignore_compound == 0)
                          state.pop();
                  }
                  break;
              default:
                    // std::cerr << "EndObject " << static_cast<int>(state.top()) << std::endl;
                  std::cerr << "Error: EndObject " << static_cast<int>(state.top()) << std::endl;
                  r = false;
                  break;
            }
            return r;
        }

    inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/)
        {
            bool r = true;
            State push_state;
            switch (length) {
              case 0:
                  r = false;
                  break;
              case 1:           // "_", "?"
                  switch (state.top()) {
                    case State::Init:
                    case State::Version:
                        r = false;
                        break;
                    case State::Root:
                        switch (static_cast<AceKey>(str[0])) {
                          case AceKey::Comment:
                          case AceKey::Comment_:
                              state.push(State::Ignore);
                              break;
                          case AceKey::Chart:
                              state.push(State::Chart);
                              break;
                          default:
                              r = false;
                        }
                        break;
                    case State::Chart:
                          // std::cerr << "State::Chart " << str[0] << std::endl;
                        switch (static_cast<AceKey>(str[0])) {
                          case AceKey::Comment:
                          case AceKey::Comment_:
                              state.push(State::Ignore);
                              break;
                          case AceKey::Antigens:
                              state.push(State::Antigen);
                              break;
                          case AceKey::Sera:
                              state.push(State::Serum);
                              break;
                          case AceKey::Info:
                              state.push(State::Info);
                              break;
                          case AceKey::Titers:
                              state.push(State::Titers);
                              break;
                          case AceKey::Projections:
                              state.push(State::Projections);
                              break;
                          case AceKey::PlotSpec:
                              state.push(State::PlotSpec);
                              break;
                          case AceKey::ColumnBases:
                              state.push(State::ColumnBases);
                              break;
                          default:
                              r = false;
                              break;
                        }
                        break;
                    case State::Info:
                        r = infoField(static_cast<AceKey>(str[0]), chart->chart_info(), false);
                        break;
                    case State::Sources:
                        r = infoField(static_cast<AceKey>(str[0]), chart->chart_info().sources().back(), true);
                        break;
                    case State::Antigen:
                        push_state = State::StringField;
                        switch (static_cast<AceKey>(str[0])) {
                          case AceKey::Comment:
                          case AceKey::Comment_:
                              push_state = State::Ignore;
                              break;
                          case AceKey::Name:
                              string_to_fill = &chart->antigens().back().name();
                              break;
                          case AceKey::Date:
                              string_to_fill = &chart->antigens().back().date();
                              break;
                          case AceKey::Lineage:
                              string_to_fill = &chart->antigens().back().lineage();
                              break;
                          case AceKey::Passage:
                              string_to_fill = &chart->antigens().back().passage();
                              break;
                          case AceKey::Reassortant:
                              string_to_fill = &chart->antigens().back().reassortant();
                              break;
                          case AceKey::SemanticAttributes:
                              string_to_fill = &chart->antigens().back().semantic();
                              break;
                          case AceKey::LabId:
                              vector_string_to_fill = &chart->antigens().back().lab_id();
                              push_state = State::VectorStringField;
                              break;
                          case AceKey::Annotations:
                              vector_string_to_fill = &chart->antigens().back().annotations();
                              push_state = State::VectorStringField;
                              break;
                          case AceKey::Clades:
                              vector_string_to_fill = &chart->antigens().back().clades();
                              push_state = State::VectorStringField;
                              break;
                          default:
                              r = false;
                              break;
                        }
                        state.push(push_state);
                        break;
                    case State::Serum:
                        push_state = State::StringField;
                        switch (static_cast<AceKey>(str[0])) {
                          case AceKey::Comment:
                          case AceKey::Comment_:
                              push_state = State::Ignore;
                              break;
                          case AceKey::Name:
                              string_to_fill = &chart->sera().back().name();
                              break;
                          case AceKey::Lineage:
                              string_to_fill = &chart->sera().back().lineage();
                              break;
                          case AceKey::Passage:
                              string_to_fill = &chart->sera().back().passage();
                              break;
                          case AceKey::Reassortant:
                              string_to_fill = &chart->sera().back().reassortant();
                              break;
                          case AceKey::Annotations:
                              vector_string_to_fill = &chart->sera().back().annotations();
                              push_state = State::VectorStringField;
                              break;
                          case AceKey::SemanticAttributes:
                              string_to_fill = &chart->sera().back().semantic();
                              break;
                          case AceKey::SerumId:
                              string_to_fill = &chart->sera().back().serum_id();
                              break;
                          case AceKey::HomologousAntigen:
                              push_state = State::IntField;
                              int_to_fill = &chart->sera().back().homologous();
                              break;
                          case AceKey::SerumSpecies:
                              string_to_fill = &chart->sera().back().serum_species();
                              break;
                          default:
                              r = false;
                              break;
                        }
                        state.push(push_state);
                        break;
                    case State::Titers:
                          // std::cerr << "State::Titers " << str[0] << std::endl;
                        switch (static_cast<AceKey>(str[0])) {
                          case AceKey::Comment:
                          case AceKey::Comment_:
                              push_state = State::Ignore;
                              break;
                          case AceKey::TitersList:
                              push_state = State::TitersList;
                              break;
                          case AceKey::TitersDict:
                              push_state = State::TitersDict;
                              break;
                          case AceKey::TitersLayers:
                              push_state = State::TitersLayers;
                              break;
                          default:
                              r = false;
                              break;
                        }
                        state.push(push_state);
                        break;
                    case State::Ignore:
                        break;
                    case State::PlotSpec:
                        r = plotSpecField(static_cast<AceKey>(str[0]));
                        break;
                    case State::TitersDict:
                        layer_to_fill->back().emplace_back(std::string(str, length), std::string());
                        break;
                    case State::StringField:
                    case State::VectorStringField:
                    case State::BoolField:
                    case State::IntField:
                    case State::DoubleField:
                    case State::Projections:
                    case State::TableType:
                    case State::TitersList:
                    case State::TitersList2:
                    case State::TitersLayers:
                    case State::ColumnBases:
                        r = false;
                        break;
                  }
                  break;
              default:
                  if (static_cast<AceKey>(str[0]) == AceKey::Comment) {
                      state.push(State::Ignore);
                  }
                  else {
                      switch (state.top()) {
                        case State::Root:
                            if (std::string(str, length) == "  version")
                                state.push(State::Version);
                            else
                                r = false;
                            break;
                        case State::TitersDict:
                            layer_to_fill->back().emplace_back(std::string(str, length), std::string());
                            break;
                        case State::Ignore:
                            break;
                        default:
                            r = false;
                            break;
                      }
                  }
                  break;
            }
            return r;
        }

    inline bool infoField(AceKey key, hidb::ChartInfo& info, bool aSources)
        {
            bool r = true;
            State push_state = State::StringField;
            switch (key) {
              case AceKey::Comment:
              case AceKey::Comment_:
                  push_state = State::Ignore;
                  break;
              case AceKey::Assay:
                  string_to_fill = &info.assay();
                  break;
              case AceKey::Virus:
                  string_to_fill = &info.virus();
                  break;
              case AceKey::VirusType:
                  string_to_fill = &info.virus_type();
                  break;
              case AceKey::Date:
                  string_to_fill = &info.date();
                  break;
              case AceKey::Name:
                  string_to_fill = &info.name();
                  break;
              case AceKey::Lab:
                  string_to_fill = &info.lab();
                  break;
              case AceKey::Rbc:
                  string_to_fill = &info.rbc();
                  break;
              case AceKey::VirusSubset:
                  string_to_fill = &info.subset();
                  break;
              case AceKey::TableType:
                  r = !aSources;
                  push_state = State::TableType;
                  break;
              case AceKey::Sources:
                  r = !aSources;
                  push_state = State::Sources;
                  break;
              default:
                  r = false;
                  break;
            }
            state.push(push_state);
            return r;
        }

    inline bool plotSpecField(AceKey key)
        {
            bool r = true;
            State push_state = State::StringField;
            switch (key) {
              case AceKey::Comment:
              case AceKey::Comment_:
                  push_state = State::Ignore;
                  break;
              case AceKey::DrawingOrder:
              case AceKey::ErrorLinePositive:
              case AceKey::ErrorLineNegative:
              case AceKey::Grid:
              case AceKey::PointIndex:
              case AceKey::PointStyles:
              case AceKey::ProcrustesIndex:
              case AceKey::ProcrustesStyle:
              case AceKey::ShownOnAll:
              case AceKey::Title:
                  std::cerr << "Warning: plot spec field " << static_cast<char>(key) << " ignored" << std::endl;
                  push_state = State::Ignore;
                  break;
              default:
                  r = false;
                  break;
            }
            state.push(push_state);
            return r;
        }

    inline bool StartArray()
        {
            bool r = true;
            switch (state.top()) {
              case State::Antigen:
              case State::Serum:
              case State::Projections:
              case State::Sources:
              case State::ColumnBases:
              case State::VectorStringField:
                  break;
              case State::TitersList:
                  state.push(State::TitersList2);
                  break;
              case State::TitersList2:
                  state.push(State::TitersList2);
                  chart->titers().list().emplace_back();
                  break;
              case State::TitersLayers:
                  chart->titers().layers().emplace_back();
                  layer_to_fill = &chart->titers().layers().back();
                  state.push(State::TitersDict);
                  break;
              case State::TitersDict:
                  if (layer_to_fill == nullptr) {
                      layer_to_fill = &chart->titers().dict();
                  }
                  break;
              case State::Ignore:
                  ++ignore_compound;
                  break;
              default:
                  r = false;
                  break;
            }
            return r;
        }

    inline bool EndArray(rapidjson::SizeType /*elementCount*/)
        {
            bool r = true;
            switch (state.top()) {
              case State::Antigen:
              case State::Serum:
              case State::TitersList:
              case State::TitersLayers:
              case State::Projections:
              case State::Sources:
              case State::ColumnBases:
                  state.pop();
                  break;
              case State::TitersList2:
                  state.pop();
                  if (state.top() == State::TitersList)
                      state.pop();
                  break;
              case State::VectorStringField:
                  state.pop();
                  vector_string_to_fill = nullptr;
                  break;
              case State::TitersDict:
                  layer_to_fill = nullptr;
                  state.pop();
                  break;
              case State::Ignore:
                  if (ignore_compound == 0)
                      r = false;
                  else {
                      --ignore_compound;
                      if (ignore_compound == 0)
                          state.pop();
                  }
                  break;
              default:
                  std::cerr << "Error: EndArray " << static_cast<int>(state.top()) << std::endl;
                  r = false;
                  break;
            }
            return r;
        }

    inline bool String(const char* str, rapidjson::SizeType length, bool /*copy*/)
        {
              // std::cout << "String " << std::string(str, length) << std::endl;
            bool r = true;
            switch (state.top()) {
              case State::Ignore:
                  if (ignore_compound == 0)
                      state.pop();
                  break;
              case State::Version:
                  if (std::memcmp(str, "acmacs-ace-v1", length))
                      r = false;
                  state.pop();
                  break;
              case State::StringField:
                  string_to_fill->assign(str, length);
                  string_to_fill = nullptr;
                  state.pop();
                  break;
              case State::VectorStringField:
                  vector_string_to_fill->emplace_back(str, length);
                  break;
              case State::TableType:
                  switch (str[0]) {
                    case 'A':
                    case 'a':
                        chart->chart_info().type() = hidb::ChartInfo::Antigenic;
                        break;
                    case 'G':
                    case 'g':
                        chart->chart_info().type() = hidb::ChartInfo::Genetic;
                        break;
                    default:
                        r = false;
                        break;
                  }
                  break;
              case State::TitersList2:
                  chart->titers().list().back().emplace_back(str, length);
                  break;
              case State::TitersDict:
                  layer_to_fill->back().back().second.assign(str, length);
                  break;
              default:
                  r = false;
                  break;
            }
            return r;
    }

    inline bool Bool(bool b)
        {
            bool r = true;
            switch (state.top()) {
              case State::Ignore:
                  if (ignore_compound == 0)
                      state.pop();
                  break;
              case State::BoolField:
                  *bool_to_fill = b;
                  bool_to_fill = nullptr;
                  state.pop();
                  break;
              default:
                  r = false;
                  break;
            }
            return r;
        }

    inline bool Int(int i)
        {
            bool r = true;
            switch (state.top()) {
              case State::Ignore:
                    // std::cerr << "Int ignore " << i << " compound " << ignore_compound << std::endl;
                  if (ignore_compound == 0)
                      state.pop();
                  break;
              case State::IntField:
                  *int_to_fill = i;
                  int_to_fill = nullptr;
                  state.pop();
                  break;
              default:
                  r = false;
                  break;
            }
            return r;
        }

    bool Double(double d)
        {
            bool r = true;
            switch (state.top()) {
              case State::Ignore:
                  if (ignore_compound == 0)
                      state.pop();
                  break;
              case State::DoubleField:
                  *double_to_fill = d;
                  double_to_fill = nullptr;
                  state.pop();
                  break;
              case State::ColumnBases:
                  chart->column_bases().push_back(d);
                  break;
              default:
                  r = false;
                  break;
            }
            return r;
        }

    bool Uint(unsigned u)
        {
            return Int(static_cast<int>(u));
        }

    inline bool Null() { std::cout << "Null()" << std::endl; return false; }
    bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return false; }
    bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return false; }

 private:
    hidb::Chart* chart;
    std::stack<State> state;
    size_t ignore_compound;
    std::string* string_to_fill;
    bool* bool_to_fill;
    int* int_to_fill;
    double* double_to_fill;
    std::vector<std::string>* vector_string_to_fill;
    hidb::ChartTiters::Dict* layer_to_fill;
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

hidb::Chart* import_chart(std::string buffer)
{
    if (buffer == "-")
        buffer = acmacs_base::read_stdin();
    else
        buffer = acmacs_base::read_file(buffer);
    hidb::Chart* chart = nullptr;
    if (buffer[0] == '{') { // && buffer.find("\"  version\": \"acmacs-ace-v1\"") != std::string::npos) {
        chart = new hidb::Chart{};
        ChartReaderEventHandler handler{chart};
        rapidjson::Reader reader;
        rapidjson::StringStream ss(buffer.c_str());
        reader.Parse(ss, handler);
        if (reader.HasParseError())
            throw std::runtime_error("cannot import chart: data parsing failed at " + std::to_string(reader.GetErrorOffset()) + ": " +  GetParseError_En(reader.GetParseErrorCode()) + "\n" + buffer.substr(reader.GetErrorOffset(), 50));
    }
    else
        throw std::runtime_error("cannot import chart: unrecognized source format");
    return chart;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
