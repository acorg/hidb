#include <iostream>
#include <cctype>

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"

#include "chart-rj.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/xz.hh"

// ----------------------------------------------------------------------

RJ::Chart* RJ::import_chart(std::string buffer)
{
    if (buffer == "-")
        buffer = acmacs_base::read_stdin();
    else
        buffer = acmacs_base::read_file(buffer);
    Chart* chart = nullptr;
    if (buffer[0] == '{') { // && buffer.find("\"  version\": \"acmacs-ace-v1\"") != std::string::npos) {
        std::shared_ptr<rapidjson::Document> d(new rapidjson::Document());
        d->Parse(buffer.c_str());
        if (d->HasParseError())
            throw std::runtime_error("cannot import chart: data parsing failed at " + std::to_string(d->GetErrorOffset()) + ": " +  GetParseError_En(d->GetParseError()));
        const auto version = d->FindMember("  version");
        if (version == d->MemberEnd() || version->value != "acmacs-ace-v1")
            throw std::runtime_error("cannot import chart: unrecognized data version");
        const auto chart_data = d->FindMember("c");
        if (chart_data == d->MemberEnd())
            throw std::runtime_error("cannot import chart: chart data not found");
        chart = new Chart(d, chart_data->value);

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

bool RJ::AntigenSerum::is_egg() const
{
    bool egg = false;
    const auto passage_o = mO.FindMember("P");
    if (passage_o != mO.MemberEnd()) {
        const size_t passage_len = passage_o->value.GetStringLength();
        if (passage_len > 1) {
            const char* passage = passage_o->value.GetString();
            size_t pos = passage_len - 1;
            if (passage_len > 14 && passage[pos] == ')' && passage[pos - 11] == '(' && passage[pos - 12] == ' ')
                pos -= 13;
            while (pos && std::isdigit(passage[pos]))
                --pos;
            egg = passage[pos] == 'E';
        }
    }
    return egg;
}

// ----------------------------------------------------------------------

class EventHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, EventHandler>
{
};

RJ_SAX::Chart* RJ_SAX::import_chart(std::string buffer)
{
    if (buffer == "-")
        buffer = acmacs_base::read_stdin();
    else
        buffer = acmacs_base::read_file(buffer);
    Chart* chart = nullptr;
    if (buffer[0] == '{') { // && buffer.find("\"  version\": \"acmacs-ace-v1\"") != std::string::npos) {
        chart = new Chart;
        EventHandler handler;
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


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
