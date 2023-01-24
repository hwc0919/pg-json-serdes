//
// Created by wanchen.he on 2023/1/13.
//
#pragma once
#include <pg_json/Converter.h>

#include <pg_json/Buffer.h>
#include <pg_json/Cursor.h>
#include <pg_json/PgFormat.h>
#include <pg_json/PgFunc.h>
#include <pg_json/PgResult.h>
#include <pg_json/json.h>

namespace pg_json
{
class ConverterImpl : public Converter
{
public:
    ConverterImpl(PgFormat format = PgFormat::kText);

    PgFormat format() const override
    {
        return format_;
    }
    void setFormat(PgFormat format) override
    {
        format_ = format;
    }
    void setBufferFactory(BufferFactory factory) override
    {
        bufferFactory_ = std::move(factory);
    }
    void setCursorFactory(CursorFactory factory) override
    {
        cursorFactory_ = std::move(factory);
    }
    void setNullHandler(NullHandler handler) override
    {
        nullHandler_ = std::move(handler);
    }
    void parseJsonToParams(const PgFunc & func, const json_t & obj, PgParamSetter & setter) const override;
    json_t parseResultToJson(const PgFunc & func, const PgResult & result) const override;

protected:
    PgFormat format_{ PgFormat::kText };
    BufferFactory bufferFactory_;
    CursorFactory cursorFactory_;
    NullHandler nullHandler_;
};
} // namespace pg_json
