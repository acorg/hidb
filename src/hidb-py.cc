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
#endif
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

namespace py = pybind11;

#include "chart.hh"

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
      // Chart
      // ----------------------------------------------------------------------

    py::class_<Chart>(m, "Chart")
            ;

    m.def("import_chart", &import_chart, py::arg("data"), py::doc("Imports chart from a buffer or file in the ace format."));


      // ----------------------------------------------------------------------

    return m.ptr();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
