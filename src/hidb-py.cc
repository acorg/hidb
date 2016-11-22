#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreserved-id-macro" // in Python.h
#pragma GCC diagnostic ignored "-Wextra-semi"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wmissing-noreturn"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wrange-loop-analysis"
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wcovered-switch-default"
#endif
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

namespace py = pybind11;

#include "chart-export.hh"
#include "hidb-export.hh"
#include "ace.hh"

// ----------------------------------------------------------------------

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
// #pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif

// ----------------------------------------------------------------------

PYBIND11_PLUGIN(hidb_backend)
{
    py::module m("hidb_backend", "HiDB access plugin");

      // ----------------------------------------------------------------------
      // Antigen, Serum
      // ----------------------------------------------------------------------

    py::class_<AntigenSerum>(m, "AntigenSerum")
            .def("full_name", &AntigenSerum::full_name)
            .def("variant_id", &AntigenSerum::variant_id)
            .def("name", static_cast<std::string (AntigenSerum::*)() const>(&AntigenSerum::name))
            .def("annotations", [](const AntigenSerum &as) { py::list list; for (const auto& anno: as.annotations()) { list.append(py::str(anno)); } return list; }, py::doc("returns a copy of the annotation list, modifications to the returned list are not applied"))
            ;

    py::class_<Antigen, AntigenSerum>(m, "Antigen")
            .def("lab_id", [](const Antigen &a) { py::list list; for (const auto& li: a.lab_id()) { list.append(py::str(li)); } return list; }, py::doc("returns a copy of the lab_id list, modifications to the returned list are not applied"))
            ;

    py::class_<Serum, AntigenSerum>(m, "Serum")
            ;

      // ----------------------------------------------------------------------
      // Chart
      // ----------------------------------------------------------------------

    py::class_<Chart>(m, "Chart")
            .def("number_of_antigens", &Chart::number_of_antigens)
            .def("number_of_sera", &Chart::number_of_sera)
            .def("antigen", &Chart::antigen, py::arg("no"))
            .def("serum", &Chart::serum, py::arg("no"))
            .def("table_id", &Chart::table_id)
            .def("find_homologous_antigen_for_sera", &Chart::find_homologous_antigen_for_sera)
            ;

    m.def("import_chart", &import_chart, py::arg("data"), py::doc("Imports chart from a buffer or file in the ace format."));

      // ----------------------------------------------------------------------
      // HiDb
      // ----------------------------------------------------------------------

    py::class_<PerTable>(m, "PerTable")
            .def("table_id", static_cast<std::string (PerTable::*)() const>(&PerTable::table_id))
            ;

    py::class_<AntigenData>(m, "AntigenData")
            .def("data", static_cast<const Antigen& (AntigenData::*)() const>(&AntigenData::data))
            .def("number_of_tables", &AntigenData::number_of_tables)
            .def("most_recent_table", &AntigenData::most_recent_table)
            .def("tables", static_cast<const std::vector<PerTable>& (AntigenData::*)() const>(&AntigenData::per_table))
            ;

    py::class_<SerumData>(m, "SerumData")
            .def("data", static_cast<const Serum& (SerumData::*)() const>(&SerumData::data))
            .def("number_of_tables", &SerumData::number_of_tables)
            .def("most_recent_table", &SerumData::most_recent_table)
            .def("tables", static_cast<const std::vector<PerTable>& (SerumData::*)() const>(&SerumData::per_table))
            .def("homologous", &SerumData::homologous)
            ;

    py::class_<HiDb>(m, "HiDb")
            .def(py::init<>())
            .def("add", &HiDb::add, py::arg("chart"))
            .def("export_to", &HiDb::exportTo, py::arg("filename"), py::arg("pretty") = false)
            .def("import_from", &HiDb::importFrom, py::arg("filename"))
            .def("import_locdb", &HiDb::importLocDb, py::arg("filename"))
            .def("list_antigens", &HiDb::list_antigens)
            .def("find_antigens", &HiDb::find_antigens, py::arg("name"), py::return_value_policy::reference)
            .def("find_antigens_with_score", &HiDb::find_antigens_with_score, py::arg("name"), py::return_value_policy::reference)
            .def("list_sera", &HiDb::list_sera)
            .def("find_sera", &HiDb::find_sera, py::arg("name"), py::return_value_policy::reference)
            .def("find_sera_with_score", &HiDb::find_sera_with_score, py::arg("name"), py::return_value_policy::reference)
            .def("all_countries", &HiDb::all_countries)
            .def("find_antigens_from_country", &HiDb::find_antigens_from_country, py::arg("coutnry"), py::return_value_policy::reference)
            .def("unrecognized_locations", &HiDb::unrecognized_locations, py::doc("returns unrecognized locations found in all antigen/serum names"))
            ;

      // ----------------------------------------------------------------------

    m.def("json", &json<Antigen>, py::arg("value"), py::arg("keyword") = "chart", py::arg("pretty") = true);
    m.def("json", &json<AntigenData>, py::arg("value"), py::arg("keyword") = "chart", py::arg("pretty") = true);
    m.def("json", &json<Serum>, py::arg("value"), py::arg("keyword") = "chart", py::arg("pretty") = true);
    m.def("json", &json<SerumData>, py::arg("value"), py::arg("keyword") = "chart", py::arg("pretty") = true);

      // ----------------------------------------------------------------------

    return m.ptr();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
