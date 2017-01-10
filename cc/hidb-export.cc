#include <stack>

#include "rapidjson/reader.h"

#include "hidb-export.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

template <typename RW> inline jsw::writer<RW>& operator <<(jsw::writer<RW>& writer, const hidb::HiDb& aHiDb)
{
    return writer << jsw::start_object
                  << jsw::key("  version") << "hidb-v4"
                  << JsonKey::Antigens << aHiDb.antigens()
                  << JsonKey::Sera << aHiDb.sera()
                  << JsonKey::Tables << aHiDb.charts()
                  << jsw::end_object;
}

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const hidb::HiDb& aHiDb, size_t aIndent)
{
    jsw::export_to_json(aHiDb, "hidb", aFilename, aIndent);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
