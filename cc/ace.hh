#pragma once

#include <string>

// ----------------------------------------------------------------------

namespace hidb {
    class Chart;
}

// ----------------------------------------------------------------------

hidb::Chart* import_chart(std::string data);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
