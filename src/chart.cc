#include <iostream>
#include <cctype>
#include <stack>
#include <cassert>
#include <regex>

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
                       StringField, BoolField, IntField, DoubleField, VectorStringField,
                       Sources, ColumnBases };

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
                    // case State::Projections:
                    // case State::PlotSpec:
                    // case State::Clades:
                    // case State::TableType:

    enum class AceKey : char {
        Comment='?', Comment_='_',
        Chart='c', Antigens='a', Sera='s', Info='i', Projections='P', PlotSpec='p', Titers='t',
        Assay='A', Virus='v', VirusType='V', Date='D', Name='N', Lab='l', Rbc='r', VirusSubset='s', TableType='T', Sources='S',
        Lineage='L', Passage='P', Reassortant='R', LabId='l', Annotations='a', Clades='c', SemanticAttributes='S',
        SerumId='I', HomologousAntigen='h', SerumSpecies='s',
        TitersList='l', TitersDict='d', TitersLayers='L',
        DrawingOrder='d', ErrorLinePositive='E', ErrorLineNegative='e', Grid='g', PointIndex='p', PointStyles='P', ProcrustesIndex='l', ProcrustesStyle='L', ShownOnAll='s', Title='t',
        ColumnBases='C',
    };

 public:
    inline ChartReaderEventHandler(Chart* aChart)
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
                  chart->mAntigens.emplace_back();
                  break;
              case State::Serum:
                  state.push(State::Serum);
                  chart->mSera.emplace_back();
                  break;
              case State::Sources:
                  chart->mInfo.mSources.emplace_back();
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
                        r = infoField(static_cast<AceKey>(str[0]), chart->mInfo, false);
                        break;
                    case State::Sources:
                        r = infoField(static_cast<AceKey>(str[0]), chart->mInfo.mSources.back(), true);
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
                          case AceKey::SemanticAttributes:
                              string_to_fill = &chart->mAntigens.back().mSemanticAttributes;
                              break;
                          case AceKey::LabId:
                              vector_string_to_fill = &chart->mAntigens.back().mLabId;
                              push_state = State::VectorStringField;
                              break;
                          case AceKey::Annotations:
                              vector_string_to_fill = &chart->mAntigens.back().mAnnotations;
                              push_state = State::VectorStringField;
                              break;
                          case AceKey::Clades:
                              vector_string_to_fill = &chart->mAntigens.back().mClades;
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
                              vector_string_to_fill = &chart->mSera.back().mAnnotations;
                              push_state = State::VectorStringField;
                              break;
                          case AceKey::SemanticAttributes:
                              string_to_fill = &chart->mSera.back().mSemanticAttributes;
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

    inline bool infoField(AceKey key, ChartInfo& info, bool aSources)
        {
            bool r = true;
            State push_state = State::StringField;
            switch (key) {
              case AceKey::Comment:
              case AceKey::Comment_:
                  push_state = State::Ignore;
                  break;
              case AceKey::Assay:
                  string_to_fill = &info.mAssay;
                  break;
              case AceKey::Virus:
                  string_to_fill = &info.mVirus;
                  break;
              case AceKey::VirusType:
                  string_to_fill = &info.mVirusType;
                  break;
              case AceKey::Date:
                  string_to_fill = &info.mDate;
                  break;
              case AceKey::Name:
                  string_to_fill = &info.mName;
                  break;
              case AceKey::Lab:
                  string_to_fill = &info.mLab;
                  break;
              case AceKey::Rbc:
                  string_to_fill = &info.mRbc;
                  break;
              case AceKey::VirusSubset:
                  string_to_fill = &info.mSubset;
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
                  chart->mTiters.mList.emplace_back();
                  break;
              case State::TitersLayers:
                  chart->mTiters.mLayers.emplace_back();
                  layer_to_fill = &chart->mTiters.mLayers.back();
                  state.push(State::TitersDict);
                  break;
              case State::TitersDict:
                  if (layer_to_fill == nullptr) {
                      layer_to_fill = &chart->mTiters.mDict;
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
              case State::TitersList2:
              case State::TitersLayers:
              case State::Projections:
              case State::Sources:
              case State::ColumnBases:
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
                  chart->mColumnBases.push_back(d);
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
    Chart* chart;
    std::stack<State> state;
    size_t ignore_compound;
    std::string* string_to_fill;
    bool* bool_to_fill;
    int* int_to_fill;
    double* double_to_fill;
    std::vector<std::string>* vector_string_to_fill;
    ChartTiters::Dict* layer_to_fill;
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

AntigenSerum::~AntigenSerum()
{

} // AntigenSerum::~AntigenSerum

// ----------------------------------------------------------------------

std::string AntigenSerum::full_name() const
{
    std::string n = mName;
    if (!mReassortant.empty()) {
        n.append(1, ' ');
        n.append(mReassortant);
    }
    if (!mAnnotations.empty()) {
        for (const auto& ann: mAnnotations) {
            n.append(1, ' ');
            n.append(ann);
        }
    }
    if (!mPassage.empty()) {
        n.append(1, ' ');
        n.append(mPassage);
    }
    return n;

} // AntigenSerum::full_name

// ----------------------------------------------------------------------

std::string AntigenSerum::passage_without_date() const
{
    if (mPassage.size() > 13 && mPassage[mPassage.size() - 1] == ')' && mPassage[mPassage.size() - 12] == '(' && mPassage[mPassage.size() - 13] == ' ' && mPassage[mPassage.size() - 4] == '-' && mPassage[mPassage.size() - 7] == '-')
        return std::string(mPassage, 0, mPassage.size() - 13);
    else
        return mPassage;

} // AntigenSerum::passage_without_date

// ----------------------------------------------------------------------

std::string Antigen::full_name() const
{
    std::string n = AntigenSerum::full_name();
    return n;

} // Antigen::full_name

// ----------------------------------------------------------------------

std::string Serum::full_name() const
{
    std::string n = AntigenSerum::full_name();
    if (!mSerumId.empty()) {
        n.append(1, ' ');
        n.append(mSerumId);
    }
    if (!mSerumSpecies.empty()) {
        n.append(1, ' ');
        n.append(mSerumSpecies);
    }
    return n;

} // Serum::full_name

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

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

AntigenSerumMatch AntigenSerum::match(const AntigenSerum& aNother) const
{
      // fields not used for matching
      // lineage, semantic-attributes, annotations

    AntigenSerumMatch match;
    if (!distinct() && !aNother.distinct() && name() == aNother.name()) {
        if (reassortant() == aNother.reassortant()) {
            if (typeid(*this) == typeid(aNother)) {
                  // antigens vs. antigens or serum vs.serum
                  // annotations must match completely
                if (annotations() != aNother.annotations()) {
                    match.add(AntigenSerumMatch::AnnotationMismatch);
                }
            }
            else {
                  // antigen vs. serum
                  // ignore serum specific annotations (CONC*, BOOSTED*, *BLEED)
                Annotations self_filtered, another_filtered;
                static std::regex serum_specific {"(CONC|BOOSTED|BLEED)"};
                static auto filter = [](const auto& anno) -> bool { return !std::regex_search(anno, serum_specific); };
                std::copy_if(annotations().begin(), annotations().end(), std::back_inserter(self_filtered), filter);
                std::copy_if(aNother.annotations().begin(), aNother.annotations().end(), std::back_inserter(another_filtered), filter);
                if (self_filtered != another_filtered) {
                    match.add(AntigenSerumMatch::AnnotationMismatch);
                }
            }

            if (has_passage() && aNother.has_passage()) {
                if (passage() != aNother.passage()) {
                    match.add(AntigenSerumMatch::PassageMismatch);
                    if (passage_without_date() != aNother.passage_without_date()) {
                        match.add(AntigenSerumMatch::PassageWithoutDateMismatch);
                        if (is_egg() != aNother.is_egg()) {
                            match.add(AntigenSerumMatch::EggCellMismatch);
                        }
                    }
                }
            }
            else {
                match.add(AntigenSerumMatch::EggCellUnknown);
            }
        }
        else {
            match.add(AntigenSerumMatch::ReassortantMismatch);
        }
    }
    else {
        match.add(AntigenSerumMatch::NameMismatch);
    }
    return match;

} // AntigenSerum::match

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------

// AntigenSerumMatch Antigen::match(const Antigen& aNother) const
// {
//       // fields not used for matching
//       // date, clades, lab_id

//     AntigenSerumMatch m = AntigenSerum::match(aNother);
//     return m;

// } // Antigen::match

// ----------------------------------------------------------------------

AntigenSerumMatch Serum::match(const Serum& aNother) const
{
      // fields not used for matching
      // homologous

    AntigenSerumMatch m = AntigenSerum::match(aNother);
    if (m < AntigenSerumMatch::Mismatch) {
        if (serum_id() != aNother.serum_id())
            m.add(AntigenSerumMatch::SerumIdMismatch);
        if (serum_species() != aNother.serum_species())
            m.add(AntigenSerumMatch::SerumSpeciesMismatch);
    }
    return m;

} // Serum::match

// ----------------------------------------------------------------------

void Chart::find_homologous_antigen_for_sera()
{
    for (auto& serum: mSera) {
          // std::cout << serum.full_name() << std::endl;
        std::vector<std::pair<size_t, AntigenSerumMatch>> antigen_match;
        for (auto antigen = mAntigens.begin(); antigen != mAntigens.end(); ++antigen) {
            antigen_match.emplace_back(static_cast<size_t>(antigen - mAntigens.begin()), serum.match(*antigen));
        }
          // sort by AntigenSerumMatch but prefer reference antigens if matches are equal
        std::sort(antigen_match.begin(), antigen_match.end(), [this](const auto& a, const auto& b) -> bool { return a.second == b.second ? mAntigens[a.first].reference() > mAntigens[b.first].reference() : a.second < b.second; });
        assert(!antigen_match.empty());
        if (antigen_match.front().second.homologous_match()) {
            serum.set_homologous(antigen_match.front().first);
            if (antigen_match.size() > 1 && antigen_match[0].second == antigen_match[1].second) {
                std::cerr << "Warning: Multiple homologous antigen candidates for " << serum.full_name() << " (the first one chosen)" << std::endl;
                for (const auto ag: antigen_match) {
                    if (ag.second != antigen_match.front().second)
                        break;
                    std::cerr << "    " << mAntigens[ag.first].full_name() << " Ref:" << mAntigens[ag.first].reference() << " Level:" << ag.second << std::endl;
                }

            }
            // else if (!antigen_match.front().second.perfect()) {
            //     std::cerr << "Info: Not a perfect match for " << serum.full_name() << " chosen" << std::endl;
            //     std::cerr << "    " << mAntigens[antigen_match.front().first].full_name() << " Ref:" << mAntigens[antigen_match.front().first].reference() << " Level:" << antigen_match.front().second << std::endl;
            // }
        }
        else {
            std::cerr << "Warning: No homologous antigen for " << serum.full_name() << std::endl;
        }
    }

} // Chart::find_homologous_antigen_for_sera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
