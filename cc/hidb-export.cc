#include <stack>

#include "acmacs-base/read-file.hh"
#include "hidb-export.hh"

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const hidb::HiDb& aHiDb, size_t aIndent)
{
    jsw::export_to_json(aHiDb, "hidb", aFilename, aIndent);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
