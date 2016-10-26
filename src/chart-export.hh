#pragma once

#include "json-writer.hh"
#include "chart.hh"

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const Antigen& antigen)
{
    return writer << 'N' << antigen.name() << if_not_empty('L', antigen.lineage())
                  << if_not_empty('P', antigen.passage()) << if_not_empty('R', antigen.reassortant())
                  << if_not_empty('a', antigen.annotations()); // << if_not_empty('S', antigen.semantic());
}

// ----------------------------------------------------------------------

inline JsonWriter& operator <<(JsonWriter& writer, const Serum& serum)
{
    return writer << 'N' << serum.name() << if_not_empty('L', serum.lineage())
                  << if_not_empty('P', serum.passage()) << if_not_empty('R', serum.reassortant())
                  << if_not_empty('I', serum.serum_id()) << if_not_empty('a', serum.annotations()) // << if_not_empty('S', antigen.semantic())
                  << if_not_empty('s', serum.serum_species());
}

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
