#include <stack>

#include "rapidjson/reader.h"

#include "hidb-export.hh"
#include "acmacs-base/read-file.hh"

// ----------------------------------------------------------------------

template <typename RW> inline JsonWriterT<RW>& operator <<(JsonWriterT<RW>& writer, const hidb::HiDb& aHiDb)
{
    return writer << StartObject
                  << JsonObjectKey("  version") << "hidb-v4"
                  << JsonKey::Antigens << aHiDb.antigens()
                  << JsonKey::Sera << aHiDb.sera()
                  << JsonKey::Tables << aHiDb.charts()
                  << EndObject;
}

// ----------------------------------------------------------------------

void hidb_export(std::string aFilename, const hidb::HiDb& aHiDb, size_t aIndent)
{
    export_to_json(aHiDb, "hidb", aFilename, aIndent);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
