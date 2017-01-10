#pragma once

// ----------------------------------------------------------------------
// Forward declarations
// ----------------------------------------------------------------------

#include <vector>
#include <string>

namespace json_writer
{
    template <typename RW> class writer;
}

namespace hidb
{
    class Antigen;
    class Serum;
    class PerTable;
    class HiDb;

    template <typename AS> class AntigenSerumData;
    class ChartData;
    using AgSrRef = std::pair<std::string, std::string>;
    using Titers = std::vector<std::vector<std::string>>;
}

template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::Antigen&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::Serum&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::PerTable&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::HiDb&);
template <typename RW, typename AS> inline json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::AntigenSerumData<AS>&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::ChartData&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::AgSrRef&);
template <typename RW> json_writer::writer<RW>& operator <<(json_writer::writer<RW>&, const hidb::Titers&);

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------

#include "acmacs-base/json-writer.hh"
namespace jsw = json_writer;

#include "chart.hh"
#include "json-keys.hh"

// ----------------------------------------------------------------------

template <typename RW> inline void JsonWriter_output(jsw::writer<RW>& writer, const hidb::AntigenSerum& ag_sr)
{
    writer << JsonKey::Name << ag_sr.name() << jsw::if_not_empty(JsonKey::Lineage, ag_sr.lineage())
           << jsw::if_not_empty(JsonKey::Passage, ag_sr.passage()) << jsw::if_not_empty(JsonKey::Reassortant, ag_sr.reassortant())
           << jsw::if_not_empty(JsonKey::Annotations, ag_sr.annotations());
    if (writer.keyword() == "chart")
        writer << jsw::if_not_empty(JsonKey::SemanticAttributes, ag_sr.semantic());
}

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::Antigen& antigen)
{
    JsonWriter_output(writer, antigen);
    if (writer.keyword() == "chart")
        writer << jsw::if_not_empty(JsonKey::Date, antigen.date()) << jsw::if_not_empty(JsonKey::LabId, antigen.lab_id())
               << jsw::if_not_empty(JsonKey::Clades, antigen.clades());
    return writer;
}

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::Serum& serum)
{
    JsonWriter_output(writer, serum);
    writer << jsw::if_not_empty(JsonKey::SerumId, serum.serum_id()) << jsw::if_not_empty(JsonKey::SerumSpecies, serum.serum_species());
    if (writer.keyword() == "chart") {
        if (serum.has_homologous())
            writer << JsonKey::HomologousAntigen << serum.homologous();
    }
    return writer;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
