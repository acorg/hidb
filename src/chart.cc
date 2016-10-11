#include <iostream>
#include <cctype>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include "chart.hh"
#include "read-file.hh"
#include "xz.hh"

// ----------------------------------------------------------------------

class ChartReaderEventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, ChartReaderEventHandler>
{
 public:
    inline ChartReaderEventHandler(Chart* aChart) : chart(aChart) {}

    bool Null() { std::cout << "Null()" << std::endl; return true; }
    bool Bool(bool b) { std::cout << "Bool(" << std::boolalpha << b << ")" << std::endl; return true; }
    bool Int(int i) { std::cout << "Int(" << i << ")" << std::endl; return true; }
    bool Uint(unsigned u) { std::cout << "Uint(" << u << ")" << std::endl; return true; }
    bool Int64(int64_t i) { std::cout << "Int64(" << i << ")" << std::endl; return true; }
    bool Uint64(uint64_t u) { std::cout << "Uint64(" << u << ")" << std::endl; return true; }
    bool Double(double d) { std::cout << "Double(" << d << ")" << std::endl; return true; }
    bool String(const char* str, rapidjson::SizeType length, bool copy) {
        std::cout << "String(" << str << ", " << length << ", " << std::boolalpha << copy << ")" << std::endl;
        return false;
    }
    bool StartObject() { std::cout << "StartObject()" << std::endl; return true; }
    bool Key(const char* str, rapidjson::SizeType length, bool copy) {
        std::cout << "Key(" << str << ", " << length << ", " << std::boolalpha << copy << ")" << std::endl;
        return true;
    }
    bool EndObject(rapidjson::SizeType memberCount) { std::cout << "EndObject(" << memberCount << ")" << std::endl; return true; }
    bool StartArray() { std::cout << "StartArray()" << std::endl; return true; }
    bool EndArray(rapidjson::SizeType elementCount) { std::cout << "EndArray(" << elementCount << ")" << std::endl; return true; }

 private:
    Chart* chart;
};

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
            throw std::runtime_error("cannot import chart: data parsing failed at " + std::to_string(reader.GetErrorOffset()) + ": " +  GetParseError_En(reader.GetParseErrorCode()));

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
