//
// Created by wanchen.he on 2023/1/12.
//
#include <pg_json/PgWriter.h>

#include "PgBinaryWriter.h"
#include "PgTextWriter.h"

using namespace pg_json;

std::shared_ptr<PgWriter> PgWriter::newTextWriter()
{
    return std::make_shared<PgTextWriter>();
}

std::shared_ptr<PgWriter> PgWriter::newBinaryWriter()
{
    return std::make_shared<PgBinaryWriter>();
}
