//
// Created by wanchen.he on 2023/1/11.
//
#pragma once
#include <pg_json/json.h>
#include <pg_json/Cursor.h>
#include <pg_json/PgType.h>

namespace pg_json
{
class PgReader
{
public:
    // Create a reader that reads postgres text encodings to json
    static std::shared_ptr<PgReader> newTextReader();
    // Create a reader that reads postgres binary encodings to json
    static std::shared_ptr<PgReader> newBinaryReader();

    virtual ~PgReader() = default;
    virtual json_t readPrimitive(const PgType & type, Cursor & cursor) = 0;

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
