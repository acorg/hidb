#include "acmacs-base/pybind11.hh"
#include "acmacs-chart/ace.hh"
#include "variant-id.hh"
#include "vaccines.hh"
#include "hidb.hh"
#include "hidb-export.hh"

using namespace hidb;

// ----------------------------------------------------------------------

// template <typename AS> inline std::vector<AS*>* copy_AntigenSerumData(std::vector<const AS*>&& src)
// {
//     std::vector<AS*>* result  = new std::vector<AS*>{src.size(), nullptr};
//     std::transform(src.begin(), src.end(), result->begin(), [](const AS* d) { return new AS{*d}; });
//     return result;
// }

PYBIND11_PLUGIN(hidb_backend)
{
    py::module m("hidb_backend", "HiDB access plugin");

      // ----------------------------------------------------------------------
      // acmacs_chart_backend
      // ----------------------------------------------------------------------

    auto acmacs_chart_backend = py::module::import("acmacs_chart_backend");
    class hidb_Antigen : public Antigen {};
    py::class_<hidb_Antigen>(m, "Antigen", acmacs_chart_backend.attr("Antigen"));
    class hidb_Serum : public Serum {};
    py::class_<hidb_Serum>(m, "Serum", acmacs_chart_backend.attr("Serum"));

      // ----------------------------------------------------------------------
      // Vaccines
      // ----------------------------------------------------------------------

    py::class_<Vaccines::HomologousSerum>(m, "hidb_Vaccines_HomologousSerum")
            .def_readonly("serum", &Vaccines::HomologousSerum::serum)
            .def_readonly("serum_index", &Vaccines::HomologousSerum::serum_index)
            .def_readonly("most_recent_table", &Vaccines::HomologousSerum::most_recent_table_date)
            .def("number_of_tables", &Vaccines::HomologousSerum::number_of_tables)
            ;

    py::class_<Vaccines::Entry>(m, "hidb_Vaccines_Entry")
            .def_readonly("antigen", &Vaccines::Entry::antigen)
            .def_readonly("antigen_data", &Vaccines::Entry::antigen_data)
            .def_readonly("antigen_index", &Vaccines::Entry::antigen_index)
            .def_readonly("homologous_sera", &Vaccines::Entry::homologous_sera, py::return_value_policy::reference)
            ;

    py::class_<Vaccines>(m, "hidb_Vaccines")
            .def("report", py::overload_cast<size_t>(&Vaccines::report, py::const_), py::arg("indent") = 0)
            .def("type", &Vaccines::type)
            .def("name", &Vaccines::name)
            .def("egg", &Vaccines::egg, py::arg("no") = 0, py::return_value_policy::reference)
            .def("cell", &Vaccines::cell, py::arg("no") = 0, py::return_value_policy::reference)
            .def("reassortant", &Vaccines::reassortant, py::arg("no") = 0, py::return_value_policy::reference)
            .def("number_of_eggs", &Vaccines::number_of_eggs)
            .def("number_of_cells", &Vaccines::number_of_cells)
            .def("number_of_reassortants", &Vaccines::number_of_reassortants)
            // .def("match", &Vaccines::match, py::arg("name") = "", py::arg("type") = "")
            // .def("remove", &Vaccines::remove, py::arg("passage_type") = "")
            ;

    m.def("find_vaccines_in_chart", &find_vaccines_in_chart, py::arg("name"), py::arg("chart"), py::arg("hidb"));

    py::class_<VaccinesOfChart>(m, "hidb_VaccinesOfChart")
            .def("report", &VaccinesOfChart::report, py::arg("indent") = 0)
            // .def("remove", &VaccinesOfChart::remove, py::arg("name") = "", py::arg("type") = "", py::arg("passage_type") = "")
            .def("__len__", &VaccinesOfChart::size)
            .def("__iter__", [](VaccinesOfChart& v) { return py::make_iterator(v.begin(), v.end()); }, py::keep_alive<0, 1>())
            ;

    py::class_<Vaccine>(m, "hidb_Vaccine")
            .def_property_readonly("name", [](const Vaccine& aVaccine) -> std::string { return aVaccine.name; })
            .def_property_readonly("type", [](const Vaccine& aVaccine) { return aVaccine.type_as_string(); })
            ;

    m.def("vaccines", py::overload_cast<std::string, std::string>(&vaccines), py::arg("subtype"), py::arg("lineage") = "", py::return_value_policy::reference);
    m.def("vaccines", py::overload_cast<const Chart&>(&vaccines), py::arg("chart"), py::return_value_policy::reference);
    m.def("vaccines", py::overload_cast<const Chart&, const hidb::HiDb&>(&vaccines), py::arg("chart"), py::arg("hidb")); // -> VaccinesOfChart*

      // ----------------------------------------------------------------------
      // HiDb
      // ----------------------------------------------------------------------

    py::class_<PerTable>(m, "PerTable")
            .def("table_id", static_cast<const std::string (PerTable::*)() const>(&PerTable::table_id))
            ;

    py::class_<AntigenData>(m, "AntigenData")
              // .def("data", py::overload_cast<>(&AntigenData::data))
            .def("data", [](AntigenData& antigen_data) -> hidb_Antigen& { return static_cast<hidb_Antigen&>(antigen_data.data()); })
            .def("number_of_tables", &AntigenData::number_of_tables)
            .def("most_recent_table", &AntigenData::most_recent_table)
            .def("oldest_table", &AntigenData::oldest_table)
            .def("date", &AntigenData::date)
            .def("tables", static_cast<const std::vector<PerTable>& (AntigenData::*)() const>(&AntigenData::per_table))
            .def("has_lab", &AntigenData::has_lab, py::arg("hidb"), py::arg("lab"))
            .def("in_hi_assay", &AntigenData::in_hi_assay, py::arg("hidb"))
            .def("in_neut_assay", &AntigenData::in_neut_assay, py::arg("hidb"))
            ;

    py::class_<SerumData>(m, "SerumData")
              //.def("data", py::overload_cast<>(&SerumData::data))
            .def("data", [](SerumData& serum_data) -> hidb_Serum& { return static_cast<hidb_Serum&>(serum_data.data()); })
            .def("number_of_tables", &SerumData::number_of_tables)
            .def("most_recent_table", &SerumData::most_recent_table)
            .def("oldest_table", &SerumData::oldest_table)
            .def("tables", static_cast<const std::vector<PerTable>& (SerumData::*)() const>(&SerumData::per_table))
            .def("homologous", &SerumData::homologous)
            .def("has_lab", &AntigenData::has_lab, py::arg("hidb"), py::arg("lab"))
            .def("in_hi_assay", &SerumData::in_hi_assay, py::arg("hidb"))
            .def("in_neut_assay", &SerumData::in_neut_assay, py::arg("hidb"))
            ;

    py::class_<AntigenRefs>(m, "AntigenRefs")
            .def("__len__", [](const AntigenRefs& ar) { return ar.size(); })
            .def("__getitem__", [](const AntigenRefs& ar, size_t i) { if (i >= ar.size()) throw py::index_error(); return ar[i]; }, py::return_value_policy::reference)
            .def("country", &AntigenRefs::country, py::arg("country"))
            .def("date_range", &AntigenRefs::date_range, py::arg("begin") = "", py::arg("end") = "")
            ;

      // --------------------------------------------------
      // lambdas below are to avoid python GC affecting data

    auto pointer_to_copy_antigen = [](const std::vector<const AntigenData*>& source) -> std::vector<AntigenData> {
        std::vector<AntigenData> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return *e; });
        return result;
    };

    auto list_antigens = [&pointer_to_copy_antigen](const HiDb& aHiDb, std::string lab, std::string assay) -> std::vector<AntigenData> {
        return pointer_to_copy_antigen(aHiDb.list_antigens(lab, assay));
    };

    auto find_antigens_by_name = [&pointer_to_copy_antigen](const HiDb& aHiDb, std::string name) -> std::vector<AntigenData> {
        return pointer_to_copy_antigen(aHiDb.find_antigens_by_name(name));
    };

    auto find_antigens = [&pointer_to_copy_antigen](const HiDb& aHiDb, std::string name) -> std::vector<AntigenData> {
        return pointer_to_copy_antigen(aHiDb.find_antigens(name));
    };

    auto find_antigens_fuzzy = [&pointer_to_copy_antigen](const HiDb& aHiDb, std::string name) {
        return pointer_to_copy_antigen(aHiDb.find_antigens_fuzzy(name));
    };

    auto find_antigens_extra_fuzzy = [&pointer_to_copy_antigen](const HiDb& aHiDb, std::string name) {
        return pointer_to_copy_antigen(aHiDb.find_antigens_extra_fuzzy(name));
    };

    auto find_antigens_with_score = [](const HiDb& aHiDb, std::string name) {
        const auto source = aHiDb.find_antigens_with_score(name);
        std::vector<std::pair<AntigenData, size_t>> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return std::make_pair(*e.first, e.second); });
        return result;
    };

    auto find_antigens_by_cdcid = [&pointer_to_copy_antigen](const HiDb& aHiDb, std::string cdcid) -> std::vector<AntigenData> {
        return pointer_to_copy_antigen(aHiDb.find_antigens_by_cdcid(cdcid));
    };

    auto pointer_to_copy_serum = [](const std::vector<const SerumData*>& source) -> std::vector<SerumData> {
        std::vector<SerumData> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return *e; });
        return result;
    };

    auto list_sera = [&pointer_to_copy_serum](const HiDb& aHiDb, std::string lab) {
        return pointer_to_copy_serum(aHiDb.list_sera(lab));
    };

    auto find_sera = [&pointer_to_copy_serum](const HiDb& aHiDb, std::string name) {
        return pointer_to_copy_serum(aHiDb.find_sera(name));
    };

    auto find_homologous_sera = [&pointer_to_copy_serum](const HiDb& aHiDb, const AntigenData& aAntigen) {
        return pointer_to_copy_serum(aHiDb.find_homologous_sera(aAntigen));
    };

    auto find_sera_with_score = [](const HiDb& aHiDb, std::string name) {
        const auto source = aHiDb.find_sera_with_score(name);
        std::vector<std::pair<SerumData, size_t>> result;
        std::transform(source.begin(), source.end(), std::back_inserter(result), [](const auto& e) { return std::make_pair(*e.first, e.second); });
        return result;
    };

      // --------------------------------------------------

    py::class_<HiDbStat>(m, "HiDbStat")
            .def(py::init<>())
            .def("compute_totals", &HiDbStat::compute_totals)
            .def("as_dict", [](HiDbStat& aStat) -> HiDbStatContainer& { return aStat; }, py::return_value_policy::reference)
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

            .def("stat_antigens", &HiDb::stat_antigens, py::arg("stat"), py::arg("start_date") = "", py::arg("end_date") = "")
            .def("stat_sera", &HiDb::stat_sera, py::arg("stat"), py::arg("stat_unique"), py::arg("start_date") = "", py::arg("end_date") = "")

            .def("list_antigen_names", &HiDb::list_antigen_names, py::arg("lab") = "", py::arg("full_name") = false)
            .def("list_antigens", list_antigens, py::arg("lab"), py::arg("assay") = "", py::doc("assay: \"hi\", \"neut\", \"\""))
            .def("find_antigens", find_antigens, py::arg("name"))
            .def("find_antigens_fuzzy", find_antigens_fuzzy, py::arg("name"))
            .def("find_antigens_extra_fuzzy", find_antigens_extra_fuzzy, py::arg("name"))
            .def("find_antigens_with_score", find_antigens_with_score, py::arg("name"))
            .def("find_antigens_by_name", find_antigens_by_name, py::arg("name"), py::return_value_policy::reference)
            .def("find_antigens_by_cdcid", find_antigens_by_cdcid, py::arg("cdcid"))
            .def("list_serum_names", &HiDb::list_serum_names, py::arg("lab") = "", py::arg("full_name") = false)
            .def("list_sera", list_sera, py::arg("lab"))
            .def("find_sera", find_sera, py::arg("name"))
            .def("find_homologous_sera", find_homologous_sera, py::arg("antigen"))
            .def("find_sera_with_score", find_sera_with_score, py::arg("name"))
            .def("find_homologous_antigens_for_sera_of_chart", &HiDb::find_homologous_antigens_for_sera_of_chart, py::arg("chart"))
            ;

      // ----------------------------------------------------------------------

    py::class_<hidb::HiDbSet>(m, "HiDbSet")
            .def(py::init<std::string>(), py::arg("hidb_dir"))
            .def("get", &hidb::HiDbSet::get, py::arg("virus_type"), py::return_value_policy::reference)
            ;

      // ----------------------------------------------------------------------

    return m.ptr();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
