#pragma once

#include <string>

// ----------------------------------------------------------------------

class HiDb;

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const HiDb& aHiDb);
void hidb_export_pretty(std::string aFilename, const HiDb& aHiDb);
void hidb_import(std::string aFilename, HiDb& aHiDb);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
