//
// Created by wanchen.he on 2023/1/12.
//

#pragma once
#include <pg_json/PgReader.h>

#include "Scope.h"
#include <vector>

namespace pg_json
{
class PgBinaryReader : public PgReader
{
public:
    json_t readPrimitive(const PgType & type, Cursor & cursor) override;
    void readArrayStart(const PgType & elemType, Cursor & cursor) override;
    void readArrayEnd(Cursor & cursor) override;
    bool hasMoreElement(Cursor & cursor) override;
    void readElementStart(Cursor & cursor) override;
    void readElementSeperator(Cursor & cursor) override;
    void readElementEnd(Cursor & cursor) override;
    void readCompositeStart(const PgType & type, Cursor & cursor) override;
    void readCompositeEnd(Cursor & cursor) override;
    void readFieldStart(const PgType & fieldType, Cursor & cursor) override;
    void readFieldSeparator(Cursor & cursor) override;
    void readFieldEnd(Cursor & cursor) override;

private:
    std::vector<ScopeMark> scopeStack_;
};
} // namespace pg_json
