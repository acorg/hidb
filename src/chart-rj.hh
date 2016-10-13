#pragma once

#include <string>

#include "rapidjson/document.h"

// ----------------------------------------------------------------------

namespace RJ {
class Chart;

class AntigenSerum
{
 public:
    std::string name() const;
    inline std::string passage() const { return string_field("P"); }
    inline std::string reassortant() const { return string_field("R"); }
    bool is_egg() const;
    bool is_reassortant() const;

 protected:
    inline AntigenSerum(Chart& aChart, rapidjson::Value& aO) : mChart(aChart), mO(aO) {}
    std::string string_field(const char* name) const;

 private:
    Chart& mChart;
    rapidjson::Value& mO;

};

class Antigen : public AntigenSerum
{
 public:
    inline std::string date() const { return string_field("D"); }

 protected:
    friend class Chart;
    inline Antigen(Chart& aChart, rapidjson::Value& aO) : AntigenSerum(aChart, aO) {}
};

class Serum : public AntigenSerum
{
 protected:
    friend class Chart;
    inline Serum(Chart& aChart, rapidjson::Value& aO) : AntigenSerum(aChart, aO) {}
};

// ----------------------------------------------------------------------

class Chart
{
 public:
    inline Chart(std::shared_ptr<rapidjson::Document> aDoc, rapidjson::Value& aChartData) : mDoc(aDoc), mChartData(aChartData) {}

    inline std::string virus_type() const { return mChartData["info"]["virus_type"].GetString(); }

    inline size_t number_of_antigens() const { return mChartData["antigens"].Capacity(); }
    inline size_t number_of_sera() const { const auto sera = mChartData.FindMember("sera"); return sera == mChartData.MemberEnd() ? 0 : sera->value.Capacity(); }

    inline Antigen antigen(size_t ag_no) { return Antigen(*this, mChartData["antigens"][static_cast<rapidjson::SizeType>(ag_no)]); }
    inline Serum serum(size_t sr_no) { return Serum(*this, mChartData["sera"][static_cast<rapidjson::SizeType>(sr_no)]); }

 private:
    std::shared_ptr<rapidjson::Document> mDoc;
    rapidjson::Value& mChartData;

    inline Chart(const Chart&) = default;
};

// ----------------------------------------------------------------------

Chart* import_chart(std::string data);

// ----------------------------------------------------------------------

inline std::string AntigenSerum::name() const
{
    std::string name = mO["N"].GetString();
    if (std::count(name.begin(), name.end(), '/') >= 2 && name.compare(0, 2, "A(")) {
        name = mChart.virus_type() + "/" + name;
    }
    return name;
}

inline std::string AntigenSerum::string_field(const char* name) const
{
    std::string value;
    const auto value_o = mO.FindMember(name);
    if (value_o != mO.MemberEnd())
        value = value_o->value.GetString();
    return value;
}

inline bool AntigenSerum::is_reassortant() const
{
    const auto reassortant_o = mO.FindMember("R");
    return reassortant_o != mO.MemberEnd() && reassortant_o->value.GetStringLength() > 0;
}

}

// ----------------------------------------------------------------------

namespace RJ_SAX
{
    class Chart {};
    Chart* import_chart(std::string data);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
