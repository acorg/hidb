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
    enum class State { Ignore, Init, Root, Version, Chart, Info, Antigens, Antigen, Sera, Serum, Titers, StringField, BoolField, IntField, AntigenAnnotations, SerumAnnotations };

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
                  break;
              case State::Ignore:
                  ++ignore_compound;
                  break;
              case State::Antigens:
                  state.push(State::Antigen);
                  chart->mAntigens.emplace_back(*chart);
                  break;
              case State::Sera:
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

    inline bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/)
        {
            bool r = true;
            const std::string key(str, length);
            State push_state;
            switch (state.top()) {
              case State::Root:
                  if (length == 0 || key[0] == '?' || key == "_" || key == " created")
                      state.push(State::Ignore);
                  else if (key == "  version")
                      state.push(State::Version);
                  else if (key == "chart")
                      state.push(State::Chart);
                  else
                      r = false;
                  break;
              case State::Chart:
                  if (key == "antigens")
                      state.push(State::Antigens);
                  else if (key == "sera")
                      state.push(State::Sera);
                  else if (key == "info")
                      state.push(State::Info);
                  else if (key == "titers")
                      state.push(State::Ignore); // state.push(State::Titers);
                  else
                      r = false;
                  break;
              case State::Antigen:
                  push_state = State::StringField;
                  if (key == "N")
                      string_to_fill = &chart->mAntigens.back().mName;
                  else if (key == "D")
                      string_to_fill = &chart->mAntigens.back().mDate;
                  else if (key == "L")
                      string_to_fill = &chart->mAntigens.back().mLineage;
                  else if (key == "P")
                      string_to_fill = &chart->mAntigens.back().mPassage;
                  else if (key == "R")
                      string_to_fill = &chart->mAntigens.back().mReassortant;
                  else if (key == "l")
                      string_to_fill = &chart->mAntigens.back().mLabId;
                  else if (key == "zc")
                      string_to_fill = &chart->mAntigens.back().mClade;
                  else if (key == "za")
                      push_state = State::AntigenAnnotations;
                  else if (key == "r") {
                      push_state = State::BoolField;
                      bool_to_fill = &chart->mAntigens.back().mReference;
                  }
                  else
                      r = false;
                  state.push(push_state);
                  break;
              case State::Serum:
                  push_state = State::StringField;
                  if (key == "N")
                      string_to_fill = &chart->mSera.back().mName;
                  else if (key == "L")
                      string_to_fill = &chart->mSera.back().mLineage;
                  else if (key == "P")
                      string_to_fill = &chart->mSera.back().mPassage;
                  else if (key == "R")
                      string_to_fill = &chart->mSera.back().mReassortant;
                  else if (key == "za")
                      push_state = State::SerumAnnotations;
                  else if (key == "I")
                      string_to_fill = &chart->mSera.back().mSerumId;
                  else if (key == "zs")
                      string_to_fill = &chart->mSera.back().mSerumSpecies;
                  else if (key == "h")
                      int_to_fill = &chart->mSera.back().mHomologous;
                  else
                      r = false;
                  state.push(push_state);
                  break;
              case State::Info:
                  push_state = State::StringField;
                  if (key == "assay")
                      string_to_fill = &chart->mInfo.mAssay;
                  else if (key == "date")
                      string_to_fill = &chart->mInfo.mDate;
                  else if (key == "lab")
                      string_to_fill = &chart->mInfo.mLab;
                  else if (key == "rbc")
                      string_to_fill = &chart->mInfo.mRbc;
                  else if (key == "virus_type")
                      string_to_fill = &chart->mInfo.mVirusType;
                  else
                      r = false;
                  state.push(push_state);
                  break;
              case State::Ignore:
                  break;
              default:
                  r = false;
                  break;
            }
            return r;
        }

    inline bool StartArray()
        {
            bool r = true;
            switch (state.top()) {
              case State::Antigens:
              case State::Sera:
              case State::AntigenAnnotations:
              case State::SerumAnnotations:
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
              case State::Antigens:
              case State::Sera:
              case State::AntigenAnnotations:
              case State::SerumAnnotations:
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
                  if (std::string(str, length) != "acmacs-ace-v1")
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
              case State::SerumAnnotations:
                  chart->mSera.back().mAnnotations.emplace_back(str, length);
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

        // std::shared_ptr<rapidjson::Document> d(new rapidjson::Document());
        // d->Parse(buffer.c_str());
        // if (d->HasParseError())
        //     throw std::runtime_error("cannot import chart: data parsing failed at " + std::to_string(d->GetErrorOffset()) + ": " +  GetParseError_En(d->GetParseError()));
        // const auto version = d->FindMember("  version");
        // if (version == d->MemberEnd() || version->value != "acmacs-ace-v1")
        //     throw std::runtime_error("cannot import chart: unrecognized data version");
        // const auto chart_data = d->FindMember("chart");
        // if (chart_data == d->MemberEnd())
        //     throw std::runtime_error("cannot import chart: chart data not found");
        // chart = new Chart(d, chart_data->value);

        // std::cout << "Ag:" << chart->number_of_antigens() << " Sr:" << chart->number_of_sera() << std::endl;
        // for (size_t ag_no = 0; ag_no < chart->number_of_antigens(); ++ag_no) {
        //     std::cout << "AG " << ag_no << " " << chart->antigen(ag_no).name() << " || D:" << chart->antigen(ag_no).date() << " || P:" << chart->antigen(ag_no).passage() << " || E?" << chart->antigen(ag_no).is_egg() << std::endl;
        // }
        // for (size_t sr_no = 0; sr_no < chart->number_of_sera(); ++sr_no) {
        //     std::cout << "SR " << sr_no << " " << chart->serum(sr_no).name() << " || P:" << chart->serum(sr_no).passage() << " || E?" << chart->serum(sr_no).is_egg() << std::endl;
        // }
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
