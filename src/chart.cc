#include <iostream>
#include <cctype>
#include <stack>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include "chart.hh"
#include "read-file.hh"
#include "xz.hh"

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
                       Clades, StringField, BoolField, IntField, AntigenAnnotations, SerumAnnotations };

                    // case State::Ignore:
                    // case State::Init:
                    // case State::Root:
                    // case State::Version:
                    // case State::Chart:
                    // case State::Info:
                    // case State::Antigen:
                    // case State::Serum:
                    // case State::Titers:
                    // case State::StringField:
                    // case State::BoolField:
                    // case State::IntField:
                    // case State::AntigenAnnotations:
                    // case State::SerumAnnotations:
                    // case State::Projections:
                    // case State::PlotSpec:
                    // case State::Clades:
                    // case State::TableType:

    enum class AceKey : char {
        Comment='?', Comment_='_',
        Chart='c', Antigens='a', Sera='s', Info='i', Projections='P', PlotSpec='p', Titers='t',
        Assay='A', Virus='v', VirusType='V', Date='D', Name='N', Lab='l', Rbc='r', VirusSubset='s', TableType='T', Sources='S',
        Lineage='L', Passage='P', Reassortant='R', LabId='l', Reference='r', Annotations='a', Clades='c',
        SerumId='I', HomologousAntigen='h', SerumSpecies='s',
        TitersList='l', TitersDict='d', TitersLayers='L',
    };

 public:
    inline ChartReaderEventHandler(Chart* aChart) : chart(aChart), ignore_compound(0), string_to_fill(nullptr), bool_to_fill(nullptr), int_to_fill(nullptr) { state.push(State::Init); }

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
              case State::Projections:
              case State::PlotSpec:
                  break;
              case State::Ignore:
                  ++ignore_compound;
                  break;
              case State::Antigen:
                  state.push(State::Antigen);
                  chart->mAntigens.emplace_back(*chart);
                  break;
              case State::Serum:
                  state.push(State::Serum);
                  chart->mSera.emplace_back(*chart);
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
                          default:
                              r = false;
                              break;
                        }
                        break;
                    case State::Info:
                        push_state = State::StringField;
                        switch (static_cast<AceKey>(str[0])) {
                          case AceKey::Comment:
                          case AceKey::Comment_:
                              push_state = State::Ignore;
                              break;
                          case AceKey::Assay:
                              string_to_fill = &chart->mInfo.mAssay;
                              break;
                          case AceKey::Virus:
                              string_to_fill = &chart->mInfo.mVirus;
                              break;
                          case AceKey::VirusType:
                              string_to_fill = &chart->mInfo.mVirusType;
                              break;
                          case AceKey::Date:
                              string_to_fill = &chart->mInfo.mDate;
                              break;
                          case AceKey::Name:
                              string_to_fill = &chart->mInfo.mName;
                              break;
                          case AceKey::Lab:
                              string_to_fill = &chart->mInfo.mLab;
                              break;
                          case AceKey::Rbc:
                              string_to_fill = &chart->mInfo.mRbc;
                              break;
                          case AceKey::VirusSubset:
                              string_to_fill = &chart->mInfo.mSubset;
                              break;
                          case AceKey::TableType:
                              push_state = State::TableType;
                              break;
                          case AceKey::Sources:
                              r = false;
                              break;
                          default:
                              r = false;
                              break;
                        }
                        state.push(push_state);
                        break;
                    case State::Antigen:
                        push_state = State::StringField;
                        switch (static_cast<AceKey>(str[0])) {
                          case AceKey::Comment:
                          case AceKey::Comment_:
                              push_state = State::Ignore;
                              break;
                          case AceKey::Name:
                              string_to_fill = &chart->mAntigens.back().mName;
                              break;
                          case AceKey::Date:
                              string_to_fill = &chart->mAntigens.back().mDate;
                              break;
                          case AceKey::Lineage:
                              string_to_fill = &chart->mAntigens.back().mLineage;
                              break;
                          case AceKey::Passage:
                              string_to_fill = &chart->mAntigens.back().mPassage;
                              break;
                          case AceKey::Reassortant:
                              string_to_fill = &chart->mAntigens.back().mReassortant;
                              break;
                          case AceKey::LabId:
                              string_to_fill = &chart->mAntigens.back().mLabId;
                              break;
                          case AceKey::Reference:
                              push_state = State::BoolField;
                              bool_to_fill = &chart->mAntigens.back().mReference;
                              break;
                          case AceKey::Annotations:
                              push_state = State::AntigenAnnotations;
                              break;
                          case AceKey::Clades:
                              push_state = State::Clades;
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
                              string_to_fill = &chart->mSera.back().mName;
                              break;
                          case AceKey::Lineage:
                              string_to_fill = &chart->mSera.back().mLineage;
                              break;
                          case AceKey::Passage:
                              string_to_fill = &chart->mSera.back().mPassage;
                              break;
                          case AceKey::Reassortant:
                              string_to_fill = &chart->mSera.back().mReassortant;
                              break;
                          case AceKey::Annotations:
                              push_state = State::AntigenAnnotations;
                              break;
                          case AceKey::SerumId:
                              string_to_fill = &chart->mSera.back().mSerumId;
                              break;
                          case AceKey::HomologousAntigen:
                              push_state = State::IntField;
                              int_to_fill = &chart->mSera.back().mHomologous;
                              break;
                          case AceKey::SerumSpecies:
                              string_to_fill = &chart->mSera.back().mSerumSpecies;
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
                    case State::PlotSpec:
                    case State::StringField:
                    case State::BoolField:
                    case State::IntField:
                    case State::AntigenAnnotations:
                    case State::SerumAnnotations:
                    case State::Projections:
                    case State::Clades:
                    case State::TableType:
                    case State::TitersList:
                    case State::TitersList2:
                    case State::TitersDict:
                    case State::TitersLayers:
                    case State::Ignore:
                        r = false;
                        break;
                  }
                  break;
              default:
                  if (static_cast<AceKey>(str[0]) == AceKey::Comment) {
                      state.push(State::Ignore);
                  }
                  else if (state.top() == State::Root && std::string(str, length) == "  version") {
                      state.push(State::Version);
                  }
                  else if (state.top() != State::Ignore) {
                      r = false;
                  }
                  break;
            }
            return r;
        }

    inline bool StartArray()
        {
            bool r = true;
            switch (state.top()) {
              case State::Antigen:
              case State::Serum:
              case State::AntigenAnnotations:
              case State::SerumAnnotations:
              case State::Clades:
                  break;
              case State::TitersList:
                  state.push(State::TitersList2);
                  break;
              case State::TitersList2:
                  chart->mTiters.mList.emplace_back();
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
              case State::AntigenAnnotations:
              case State::SerumAnnotations:
              case State::Clades:
              case State::TitersList:
              case State::TitersList2:
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
              case State::AntigenAnnotations:
                  chart->mAntigens.back().mAnnotations.emplace_back(str, length);
                  break;
              case State::Clades:
                  chart->mAntigens.back().mClades.emplace_back(str, length);
                  break;
              case State::SerumAnnotations:
                  chart->mSera.back().mAnnotations.emplace_back(str, length);
                  break;
              case State::TableType:
                  switch (str[0]) {
                    case 'A':
                    case 'a':
                        chart->mInfo.mType = ChartInfo::Antigenic;
                        break;
                    case 'G':
                    case 'g':
                        chart->mInfo.mType = ChartInfo::Genetic;
                        break;
                    default:
                        r = false;
                        break;
                  }
                  break;
              case State::TitersList2:
                  chart->mTiters.mList.back().emplace_back(str, length);
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

    inline bool Null() { std::cout << "Null()" << std::endl; return false; }
    bool Uint(unsigned u) { std::cout << "Uint(" << u << ")" << std::endl; return false; }
    bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return false; }
    bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return false; }
    bool Double(double d) { std::cout << "Double(" << d << ")" << std::endl; return false; }

 private:
    Chart* chart;
    std::stack<State> state;
    size_t ignore_compound;
    std::string* string_to_fill;
    bool* bool_to_fill;
    int* int_to_fill;
};

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

Chart* import_chart(std::string buffer)
{
    if (buffer == "-")
        buffer = read_stdin();
    else
        buffer = read_file(buffer);
    Chart* chart = nullptr;
    if (buffer[0] == '{') { // && buffer.find("\"  version\": \"acmacs-ace-v1\"") != std::string::npos) {
        chart = new Chart;
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

bool AntigenSerum::is_egg() const
{
    bool egg = false;
    if (mPassage.size() > 1) {
        size_t pos = mPassage.size() - 1;
        if (mPassage.size() > 14 && mPassage[pos] == ')' && mPassage[pos - 11] == '(' && mPassage[pos - 12] == ' ')
            pos -= 13;
        while (pos && std::isdigit(mPassage[pos]))
            --pos;
        egg = mPassage[pos] == 'E';
    }
    return egg;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
