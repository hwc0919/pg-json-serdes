//
// Created by wanchen.he on 2023/1/9.
//
#pragma once
#include <pg_json/PgWriter.h>

#include "Scope.h"
#include <vector>

namespace pg_json
{
class PgTextWriter : public PgWriter
{
public:
    void writePrimitive(const PgType & pgType, const json_t & jsonParam, Buffer & buf) override;
    void writeArrayStart(const PgType & elemType, size_t len, Buffer & buf) override;
    void writeArrayEnd(Buffer & buf) override;
    void writeElementStart(const PgType & elemType, Buffer & buf) override;
    void writeElementEnd(Buffer & buf) override;
    void writeElementSeperator(Buffer & buf) override;
    void writeCompositeStart(const PgType & type, Buffer & buf) override;
    void writeCompositeEnd(Buffer & buf) override;
    void writeFieldStart(const PgType & fieldType, Buffer & buf) override;
    void writeFieldEnd(Buffer & buf) override;
    void writeFieldSeparator(Buffer & buf) override;
    void writeNullField(const PgType & fieldType, Buffer & buf) override;

protected:
    void writeUnescapedString(const std::string &, Buffer &);
    bool needQuote(const PgType & pgType);

    std::vector<ScopeMark> scopeStack_;
};

} // namespace pg_json
