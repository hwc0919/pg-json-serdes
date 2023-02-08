//
// Created by wanchen.he on 2023/1/13.
//
#pragma once
#include <pg_json/PgFormat.h>
#include <pg_json/PgFunc.h>
#include <pg_json/PgParamSetter.h>
#include <pg_json/PgReader.h>
#include <pg_json/PgResult.h>
#include <pg_json/PgWriter.h>
#include <pg_json/json.h>

namespace pg_json
{
class Converter
{
public:
    using NullHandler = std::function<json_t(const PgType & pgType, bool explicitNull)>;

    explicit Converter(PgFormat format = PgFormat::kText);
    ~Converter() = default;

    PgFormat format() const
    {
        return format_;
    }
    void setFormat(PgFormat format)
    {
        format_ = format;
    }
    void setNullHandler(NullHandler handler)
    {
        nullHandler_ = std::move(handler);
    }
    void parseJsonToParams(const PgFunc & func, const json_t & obj, PgParamSetter & setter) const;
    json_t parseResultToJson(const PgFunc & func, const PgResult & result) const;

    void parseJsonToPg(const PgType & pgType, const json_t & param, PgWriter & writer, Buffer & buffer) const;
    json_t parsePgToJson(const PgType & pgType, PgReader & reader, Cursor & cursor) const;

protected:
    PgFormat format_{ PgFormat::kText };
    NullHandler nullHandler_;
};

} // namespace pg_json
