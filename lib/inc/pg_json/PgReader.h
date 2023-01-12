//
// Created by wanchen.he on 2023/1/11.
//
#pragma once
#include <nlohmann/json.hpp>
#include <pg_json/Cursor.h>
#include <pg_json/PgType.h>

namespace pg_json
{
class PgReader
{
public:
    virtual ~PgReader() = default;
    virtual nlohmann::json readPrimitive(const PgType & type, Cursor & cursor) = 0;

    virtual void readArrayStart(const PgType & elemType, Cursor & cursor) = 0;
    virtual void readArrayEnd(Cursor & cursor) = 0;
    virtual bool hasMoreElement(Cursor & cursor) = 0;
    virtual void readElementStart(Cursor & cursor) = 0;
    virtual void readElementSeperator(Cursor & cursor) = 0;
    virtual void readElementEnd(Cursor & cursor) = 0;

    virtual void readCompositeStart(const PgType & type, Cursor & cursor) = 0;
    virtual void readCompositeEnd(Cursor & cursor) = 0;
    virtual void readFieldStart(const PgType & fieldType, Cursor & cursor) = 0;
    virtual void readFieldSeparator(Cursor & cursor) = 0;
    virtual void readFieldEnd(Cursor & cursor) = 0;
};
} // namespace pg_json
