//
// Created by wanchen.he on 2023/1/9.
//
#pragma once
#include <nlohmann/json.hpp>
#include <pfs/IBuffer.h>
#include <pfs/PgType.h>

namespace pfs
{
class IPgWriter
{
public:
    virtual ~IPgWriter() = default;

    virtual void writePrimitive(const PgType & pgType, const nlohmann::json & jsonParam, IBuffer & buf) = 0;

    /**
     * Check whether the next element or field need to quote.
     * @param pgType
     * @param jsonParam
     */
    virtual bool needQuote(const PgType & pgType, const nlohmann::json & jsonParam) = 0;

    virtual void writeArrayStart(const PgType & elemType, size_t len, IBuffer & buf) = 0;
    virtual void writeArrayEnd(IBuffer & buf) = 0;
    virtual void writeElementStart(IBuffer & buf, bool needQuote) = 0;
    virtual void writeElementSeperator(IBuffer & buf) = 0;
    virtual void writeElementEnd(IBuffer & buf) = 0;

    virtual void writeCompositeStart(const PgType & type, IBuffer & buf) = 0;
    virtual void writeCompositeEnd(IBuffer & buf) = 0;
    virtual void writeFieldStart(const PgType & fieldType, IBuffer & buf, bool needQuote) = 0;
    virtual void writeFieldSeparator(IBuffer & buf) = 0;
    virtual void writeFieldEnd(IBuffer & buf) = 0;
    virtual void writeNullField(const PgType & fieldType, IBuffer & buf) = 0;
};

} // namespace pfs
