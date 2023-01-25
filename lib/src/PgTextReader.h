//
// Created by wanchen.he on 2023/1/11.
//
#pragma once
#include <pg_json/PgReader.h>

#include "Scope.h"
#include <vector>

namespace pg_json
{
class PgTextReader : public PgReader
{
public:
    json_t readPrimitive(const PgType & type, Cursor & cursor) override;
    void readArrayStart(const PgType & elemType, Cursor & cursor) override;
    void readArrayEnd(Cursor & cursor) override;
    bool hasMoreElement(Cursor & cursor) override;
    void readElementStart(Cursor & cursor) override;
    void readElementSeperator(Cursor & cursor) override;
    void readElementEnd(Cursor & cursor) override;
    void readCompositeStart(const PgType & type, Cursor & buf) override;
    void readCompositeEnd(Cursor & buf) override;
    void readFieldStart(const PgType & fieldType, Cursor & buf) override;
    void readFieldSeparator(Cursor & buf) override;
    void readFieldEnd(Cursor & buf) override;

protected:
    std::string readWholeField(Cursor & cursor) const;
    std::string readEscapedString(Cursor & cursor) const;

    std::vector<ScopeMark> scopeStack_;
};
} // namespace pg_json
