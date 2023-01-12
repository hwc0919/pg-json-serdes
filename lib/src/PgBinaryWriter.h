//
// Created by wanchen.he on 2023/1/12.
//

#pragma once
#include <pg_json/PgWriter.h>

#include "Scope.h"
#include <vector>

namespace pg_json
{
class PgBinaryWriter : public PgWriter
{
public:
    void writePrimitive(const PgType & pgType, const nlohmann::json & jsonParam, Buffer & buf) override;
    bool needQuote(const PgType & pgType, const nlohmann::json & jsonParam) override;
    void writeArrayStart(const PgType & elemType, size_t len, Buffer & buf) override;
    void writeArrayEnd(Buffer & buf) override;
    void writeElementStart(Buffer & buf, bool needQuote) override;
    void writeElementSeperator(Buffer & buf) override;
    void writeElementEnd(Buffer & buf) override;
    void writeCompositeStart(const PgType & type, Buffer & buf) override;
    void writeCompositeEnd(Buffer & buf) override;
    void writeFieldStart(const PgType & fieldType, Buffer & buf, bool needQuote) override;
    void writeFieldSeparator(Buffer & buf) override;
    void writeFieldEnd(Buffer & buf) override;
    void writeNullField(const PgType & fieldType, Buffer & buf) override;

private:
    std::vector<ScopeMark> scopeStack_;
};
} // namespace pg_json