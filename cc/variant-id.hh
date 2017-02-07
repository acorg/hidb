#pragma once

#include "acmacs-base/string.hh"
#include "acmacs-chart/chart.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    inline std::string variant_id(const Antigen& aAntigen)
    {
        return string::join({aAntigen.reassortant(), aAntigen.annotations().join(), aAntigen.passage()});
    }

    inline std::string variant_id(const Serum& aSerum)
    {
        return string::join({aSerum.reassortant(), aSerum.serum_id(), aSerum.annotations().join()});
    }

    inline std::string name_for_exact_matching(const Antigen& aAntigen)
    {
        return string::join({aAntigen.name(), variant_id(aAntigen)});
    }

    inline std::string name_for_exact_matching(const Serum& aSerum)
    {
        return string::join({aSerum.name(), variant_id(aSerum)});
    }

    std::string table_id(const Chart& aChart);

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
