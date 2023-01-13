//
// Created by wanchen.he on 2023/1/9.
//
#pragma once
#include <pg_json/Buffer.h>
#include <pg_json/PgType.h>
#include <pg_json/json.h>

namespace pg_json
{
class PgWriter
{
public:
    // Create a writer that parse json to postgres text encodings
    static std::shared_ptr<PgWriter> newTextWriter();
    // Create a writer that parse json to postgres binary encodings
    static std::shared_ptr<PgWriter> newBinaryWriter();

    virtual ~PgWriter() = default;

    virtual void writePrimitive(const PgType & pgType, const json_t & jsonParam, Buffer & buf) = 0;

    /**
     * Check whether the next element or field need to quote.
     * @param pgType
     * @param jsonParam
     */
    virtual bool needQuote(const PgType & pgType, const json_t & jsonParam) = 0;

    virtual void writeArrayStart(const PgType & elemType, size_t len, Buffer & buf) = 0;
    virtual void writeArrayEnd(Buffer & buf) = 0;
    virtual void writeElementStart(Buffer & buf, bool needQuote) = 0;
    virtual void writeElementSeperator(Buffer & buf) = 0;
    virtual void writeElementEnd(Buffer & buf) = 0;

    virtual void writeCompositeStart(const PgType & type, Buffer & buf) = 0;
    virtual void writeCompositeEnd(Buffer & buf) = 0;
    virtual void writeFieldStart(const PgType & fieldType, Buffer & buf, bool needQuote) = 0;
    virtual void writeFieldSeparator(Buffer & buf) = 0;
    virtual void writeFieldEnd(Buffer & buf) = 0;
    virtual void writeNullField(const PgType & fieldType, Buffer & buf) = 0;
};

} // namespace pg_json
