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
    using BufferFactory = std::function<std::shared_ptr<Buffer>()>;
    using CursorFactory = std::function<std::shared_ptr<Cursor>(const char *, size_t)>;

    static std::shared_ptr<Converter> newConverter(PgFormat format);
    static void parseJsonToPg(const PgType & pgType, const json_t & param, PgWriter & writer, Buffer & buffer);
    static json_t parsePgToJson(const PgType & pgType, PgReader & reader, Cursor & cursor);

    virtual ~Converter() = default;
    virtual PgFormat format() const = 0;
    virtual void setFormat(PgFormat format) = 0;
    virtual void setBufferFactory(BufferFactory factory) = 0;
    virtual void setCursorFactory(CursorFactory factory) = 0;

    virtual void parseJsonToParams(const PgFunc & func, const json_t & obj, PgParamSetter & setter) const = 0;
    virtual json_t parseResultToJson(const PgFunc & func, const PgResult & result) const = 0;
};

} // namespace pg_json
