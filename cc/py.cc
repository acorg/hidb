#include "acmacs-base/pybind11.hh"
#include "acmacs-chart/ace.hh"
#include "variant-id.hh"
#include "vaccines.hh"
#include "hidb.hh"
#include "hidb-export.hh"

using namespace hidb;

// ----------------------------------------------------------------------

PYBIND11_PLUGIN(hidb_backend)
{
    py::module m("hidb_backend", "HiDB access plugin");

      // ----------------------------------------------------------------------
      // Antigen, Serum
      // ----------------------------------------------------------------------

    py::class_<AntigenSerum>(m, "AntigenSerum")
            .def("full_name", &AntigenSerum::full_name)
            .def("abbreviated_name", &AntigenSerum::abbreviated_name, py::arg("locdb"), py::doc("includes passage, reassortant, annotations"))
            .def("name_abbreviated", &AntigenSerum::name_abbreviated, py::arg("locdb"), py::doc("just name without passage, reassortant, annotations"))
            .def("location_abbreviated", &AntigenSerum::location_abbreviated, py::arg("locdb"))
            .def("name", py::overload_cast<>(&AntigenSerum::name, py::const_))
            .def("lineage", py::overload_cast<>(&AntigenSerum::lineage, py::const_))
            .def("passage", py::overload_cast<>(&AntigenSerum::passage, py::const_))
            .def("passage_type", &AntigenSerum::passage_type)
            .def("reassortant", py::overload_cast<>(&AntigenSerum::reassortant, py::const_))
            .def("semantic", py::overload_cast<>(&AntigenSerum::semantic, py::const_))
            .def("annotations", [](const AntigenSerum &as) { py::list list; for (const auto& anno: as.annotations()) { list.append(py::str(anno)); } return list; }, py::doc("returns a copy of the annotation list, modifications to the returned list are not applied"))
            ;

    py::class_<Antigen, AntigenSerum>(m, "Antigen")
            .def("date", py::overload_cast<>(&Antigen::date, py::const_))
            .def("lab_id", [](const Antigen &a) { py::list list; for (const auto& li: a.lab_id()) { list.append(py::str(li)); } return list; }, py::doc("returns a copy of the lab_id list, modifications to the returned list are not applied"))
            .def("variant_id", [](const Antigen &a) { return variant_id(a); })
            ;

    py::class_<Serum, AntigenSerum>(m, "Serum")
            .def("variant_id", [](const Serum &s) { return variant_id(s); })
            .def("serum_id", py::overload_cast<>(&Serum::serum_id, py::const_))
            .def("serum_species", py::overload_cast<>(&Serum::serum_species, py::const_))
            .def("homologous", py::overload_cast<>(&Serum::homologous, py::const_))
            ;

      // ----------------------------------------------------------------------
      // Chart
      // ----------------------------------------------------------------------

    py::class_<Chart>(m, "Chart")
            .def("number_of_antigens", &Chart::number_of_antigens)
            .def("number_of_sera", &Chart::number_of_sera)
            .def("antigen", &Chart::antigen, py::arg("no"))
            .def("serum", &Chart::serum, py::arg("no"))
            .def("table_id", [](const Chart& aChart) { return table_id(aChart); })
            .def("find_homologous_antigen_for_sera", &Chart::find_homologous_antigen_for_sera)
            ;

    m.def("import_chart", &import_chart, py::arg("data"), py::doc("Imports chart from a buffer or file in the ace format."));
    m.def("import_chart", [](py::bytes data) { return import_chart(data); }, py::arg("data"), py::doc("Imports chart from a buffer or file in the ace format."));
    m.def("export_chart", &export_chart, py::arg("filename"), py::arg("chart"), py::doc("Exports chart into a file in the ace format."));

      // ----------------------------------------------------------------------
      // Vaccines
      // ----------------------------------------------------------------------

    py::class_<Vaccines::HomologousSerum>(m, "Vaccines_HomologousSerum")
            .def_readonly("serum", &Vaccines::HomologousSerum::serum)
            .def_readonly("serum_index", &Vaccines::HomologousSerum::serum_index)
            .def_readonly("most_recent_table", &Vaccines::HomologousSerum::most_recent_table_date)
            .def("number_of_tables", &Vaccines::HomologousSerum::number_of_tables)
            ;

    py::class_<Vaccines::Entry>(m, "Vaccines_Entry")
            .def_readonly("antigen", &Vaccines::Entry::antigen)
            .def_readonly("antigen_index", &Vaccines::Entry::antigen_index)
            .def_readonly("homologous_sera", &Vaccines::Entry::homologous_sera, py::return_value_policy::reference)
            ;

    py::class_<Vaccines>(m, "Vaccines")
            .def("report", &Vaccines::report, py::arg("indent") = 0)
            .def("egg", &Vaccines::egg, py::arg("no") = 0, py::return_value_policy::reference)
            .def("cell", &Vaccines::cell, py::arg("no") = 0, py::return_value_policy::reference)
            .def("reassortant", &Vaccines::reassortant, py::arg("no") = 0, py::return_value_policy::reference)
            .def("number_of_eggs", &Vaccines::number_of_eggs)
            .def("number_of_cells", &Vaccines::number_of_cells)
            .def("number_of_reassortants", &Vaccines::number_of_reassortants)
            ;

    m.def("find_vaccines_in_chart", &find_vaccines_in_chart, py::arg("name"), py::arg("chart"), py::arg("hidb"));

      // ----------------------------------------------------------------------
      // HiDb
      // ----------------------------------------------------------------------

    py::class_<PerTable>(m, "PerTable")
            .def("table_id", static_cast<const std::string (PerTable::*)() const>(&PerTable::table_id))
            ;

    py::class_<AntigenData>(m, "AntigenData")
            .def("data", static_cast<const Antigen& (AntigenData::*)() const>(&AntigenData::data))
            .def("number_of_tables", &AntigenData::number_of_tables)
            .def("most_recent_table", &AntigenData::most_recent_table)
            .def("oldest_table", &AntigenData::oldest_table)
            .def("date", &AntigenData::date)
            .def("tables", static_cast<const std::vector<PerTable>& (AntigenData::*)() const>(&AntigenData::per_table))
            ;

    py::class_<SerumData>(m, "SerumData")
            .def("data", static_cast<const Serum& (SerumData::*)() const>(&SerumData::data))
            .def("number_of_tables", &SerumData::number_of_tables)
            .def("most_recent_table", &SerumData::most_recent_table)
            .def("oldest_table", &SerumData::oldest_table)
            .def("tables", static_cast<const std::vector<PerTable>& (SerumData::*)() const>(&SerumData::per_table))
            .def("homologous", &SerumData::homologous)
            ;

    py::class_<AntigenRefs>(m, "AntigenRefs")
            .def("__len__", [](const AntigenRefs& ar) { return ar.size(); })
            .def("__getitem__", [](const AntigenRefs& ar, size_t i) { if (i >= ar.size()) throw py::index_error(); return ar[i]; }, py::return_value_policy::reference)
            .def("country", &AntigenRefs::country, py::arg("country"))
            .def("date_range", &AntigenRefs::date_range, py::arg("begin") = "", py::arg("end") = "")
            ;

      // --------------------------------------------------
      // lambdas below are to avoid python GC affecting data

    auto pointer_to_copy = [](const std::vector<const AntigenData*>& source) -> std::vector<AntigenData> {
        std::vector<AntigenData> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return *e; });
        return result;
    };

    auto find_antigens_by_name = [&pointer_to_copy](const HiDb& aHiDb, std::string name) -> std::vector<AntigenData> {
        return pointer_to_copy(aHiDb.find_antigens_by_name(name));
    };

    auto find_antigens = [&pointer_to_copy](const HiDb& aHiDb, std::string name) -> std::vector<AntigenData> {
        return pointer_to_copy(aHiDb.find_antigens(name));
    };

    auto find_antigens_fuzzy = [&pointer_to_copy](const HiDb& aHiDb, std::string name) {
        return pointer_to_copy(aHiDb.find_antigens_fuzzy(name));
    };

    auto find_antigens_extra_fuzzy = [&pointer_to_copy](const HiDb& aHiDb, std::string name) {
        return pointer_to_copy(aHiDb.find_antigens_extra_fuzzy(name));
    };

    auto find_antigens_with_score = [](const HiDb& aHiDb, std::string name) {
        const auto source = aHiDb.find_antigens_with_score(name);
        std::vector<std::pair<AntigenData, size_t>> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return std::make_pair(*e.first, e.second); });
        return result;
    };

    auto find_antigens_by_cdcid = [&pointer_to_copy](const HiDb& aHiDb, std::string cdcid) -> std::vector<AntigenData> {
        return pointer_to_copy(aHiDb.find_antigens_by_cdcid(cdcid));
    };

    auto find_sera = [](const HiDb& aHiDb, std::string name) {
        const auto source = aHiDb.find_sera(name);
        std::vector<SerumData> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return *e; });
        return result;
    };

    auto find_homologous_sera = [](const HiDb& aHiDb, const AntigenData& aAntigen) {
        const auto source = aHiDb.find_homologous_sera(aAntigen);
        std::vector<SerumData> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return *e; });
        return result;
    };

    auto find_sera_with_score = [](const HiDb& aHiDb, std::string name) {
        const auto source = aHiDb.find_sera_with_score(name);
        std::vector<std::pair<SerumData, size_t>> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return std::make_pair(*e.first, e.second); });
        return result;
    };

      // --------------------------------------------------

    py::class_<HiDbAntigenStat>(m, "HiDbAntigenStat")
            .def(py::init<>())
            .def("compute_totals", &HiDbAntigenStat::compute_totals)
            .def("as_dict", [](HiDbAntigenStat& aStat) -> HiDbAntigenStatContainer& { return aStat; }, py::return_value_policy::reference)
            ;

      // --------------------------------------------------

    py::class_<HiDb>(m, "HiDb")
            .def(py::init<>())
            .def("add", &HiDb::add, py::arg("chart"))
            .def("export_to", &HiDb::exportTo, py::arg("filename"), py::arg("pretty") = false)
            .def("import_from", &HiDb::importFrom, py::arg("filename"))
            .def("import_locdb", &HiDb::importLocDb, py::arg("filename"))

            .def("all_antigens", &HiDb::all_antigens, py::return_value_policy::reference)
            .def("all_countries", &HiDb::all_countries)
            .def("unrecognized_locations", &HiDb::unrecognized_locations, py::doc("returns unrecognized locations found in all antigen/serum names"))
            .def("stat", &HiDb::stat, py::arg("stat"), py::arg("start_date") = "", py::arg("end_date") = "")

            .def("list_antigen_names", &HiDb::list_antigen_names, py::arg("lab") = "", py::arg("full_name") = false)
            .def("list_antigens", &HiDb::list_antigens, py::arg("lab"))
            .def("find_antigens", find_antigens, py::arg("name"))
            .def("find_antigens_fuzzy", find_antigens_fuzzy, py::arg("name"))
            .def("find_antigens_extra_fuzzy", find_antigens_extra_fuzzy, py::arg("name"))
            .def("find_antigens_with_score", find_antigens_with_score, py::arg("name"))
            .def("find_antigens_by_name", find_antigens_by_name, py::arg("name"))
            .def("find_antigens_by_cdcid", find_antigens_by_cdcid, py::arg("cdcid"))
            .def("list_serum_names", &HiDb::list_serum_names, py::arg("lab") = "", py::arg("full_name") = false)
            .def("list_sera", &HiDb::list_sera, py::arg("lab"))
            .def("find_sera", find_sera, py::arg("name"))
            .def("find_homologous_sera", find_homologous_sera, py::arg("antigen"))
            .def("find_sera_with_score", find_sera_with_score, py::arg("name"))
            .def("find_homologous_antigens_for_sera_of_chart", &HiDb::find_homologous_antigens_for_sera_of_chart, py::arg("chart"))
            ;

      // ----------------------------------------------------------------------

    return m.ptr();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
