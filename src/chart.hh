#pragma once

#include <string>

#include "rapidjson/document.h"

// ----------------------------------------------------------------------

class Chart
{
 public:
    inline Chart(rapidjson::Document* aDoc) : m(aDoc) {}
    ~Chart();

 private:
    rapidjson::Document* m;
};

// ----------------------------------------------------------------------

Chart* import_chart(std::string data);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
