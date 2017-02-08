#pragma once

#include <string>

// ----------------------------------------------------------------------

namespace hidb
{
    class HiDb;
};

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const hidb::HiDb& aHiDb, size_t aIndent);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
