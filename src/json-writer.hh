#pragma once

#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"

// ----------------------------------------------------------------------

class JsonWriter : public rapidjson::PrettyWriter<rapidjson::StringBuffer>
{
 public:
    inline JsonWriter() : rapidjson::PrettyWriter<rapidjson::StringBuffer>(mBuffer)
        {
            SetIndent(' ', 1);
        }

    inline operator std::string() const { return mBuffer.GetString(); }

 private:
    rapidjson::StringBuffer mBuffer;
};

// ----------------------------------------------------------------------

enum _StartArray { StartArray };
enum _EndArray { EndArray };
enum _StartObject { StartObject };
enum _EndObject { EndObject };

inline JsonWriter& operator <<(JsonWriter& writer, _StartArray) { writer.StartArray(); return writer; }
inline JsonWriter& operator <<(JsonWriter& writer, _EndArray) { writer.EndArray(); return writer; }
inline JsonWriter& operator <<(JsonWriter& writer, _StartObject) { writer.StartObject(); return writer; }
inline JsonWriter& operator <<(JsonWriter& writer, _EndObject) { writer.EndObject(); return writer; }

inline JsonWriter& operator <<(JsonWriter& writer, char key) { writer.Key(&key, 1, false); return writer; }
inline JsonWriter& operator <<(JsonWriter& writer, std::string s) { writer.String(s.c_str(), static_cast<unsigned>(s.size())); return writer; }

template <typename T> inline JsonWriter& operator <<(JsonWriter& writer, const std::vector<T>& list)
{
    writer << StartArray;
    for (const auto& e: list)
        writer << e;
    return writer << EndArray;
}

// inline JsonWriter& operator <<(JsonWriter& writer, const std::vector<std::string>& list_strings)
// {
//     writer << StartArray;
//     for (const auto& e: list_strings)
//         writer << e;
//     return writer << EndArray;
// }

inline JsonWriter& operator <<(JsonWriter& writer, const std::vector<std::vector<std::string>>& list_list_strings)
{
    writer << StartArray;
    for (const auto& e: list_list_strings)
        writer << e;
    return writer << EndArray;
}

// ----------------------------------------------------------------------

template <typename Value> class _if_not_empty
{
 public:
    inline _if_not_empty(char key, Value value) : mKey(key), mValue(value) {}

    friend inline JsonWriter& operator <<(JsonWriter& writer, const _if_not_empty<Value>& data)
        {
            if (!data.mValue.empty())
                writer << data.mKey << data.mValue;
            return writer;
        }

 private:
    char mKey;
    Value mValue;
};

template <typename Value> _if_not_empty<Value> if_not_empty(char key, Value value) { return _if_not_empty<Value>(key, value); }

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
