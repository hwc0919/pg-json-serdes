//
// Created by wanchen.he on 2023/1/12.
//
#include "PgBinaryReader.h"

using namespace pg_json;

nlohmann::json PgBinaryReader::readPrimitive(const PgType & type, Cursor & cursor)
{
    return nullptr;
}

void PgBinaryReader::readArrayStart(const PgType & elemType, Cursor & cursor)
{
}

void PgBinaryReader::readArrayEnd(Cursor & cursor)
{
}

bool PgBinaryReader::hasMoreElement(Cursor & cursor)
{
    return false;
}

void PgBinaryReader::readElementStart(Cursor & cursor)
{
}

void PgBinaryReader::readElementSeperator(Cursor & cursor)
{
}

void PgBinaryReader::readElementEnd(Cursor & cursor)
{
}

void PgBinaryReader::readCompositeStart(const PgType & type, Cursor & cursor)
{
}

void PgBinaryReader::readCompositeEnd(Cursor & cursor)
{
}

void PgBinaryReader::readFieldStart(const PgType & fieldType, Cursor & cursor)
{
}

void PgBinaryReader::readFieldSeparator(Cursor & cursor)
{
}

void PgBinaryReader::readFieldEnd(Cursor & cursor)
{
}
