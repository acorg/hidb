#pragma once

#include "acmacs-base/string.hh"
#include "acmacs-chart-2/chart.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    inline std::string variant_id(const acmacs::chart::Antigen& aAntigen)
    {
        return string::join(" ", {aAntigen.reassortant(), string::join(" ", aAntigen.annotations()), aAntigen.passage()});
    }

    inline std::string variant_id(const acmacs::chart::Serum& aSerum)
    {
        return string::join({aSerum.reassortant(), aSerum.serum_id(), string::join(" ", aSerum.annotations())});
    }

    inline std::string name_for_exact_matching(const acmacs::chart::Antigen& aAntigen)
    {
        return string::join({aAntigen.name(), variant_id(aAntigen)});
    }

    inline std::string name_for_exact_matching(const acmacs::chart::Serum& aSerum)
    {
        return string::join({aSerum.name(), variant_id(aSerum)});
    }

    std::string table_id(const acmacs::chart::Chart& aChart);

} // namespace hidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
