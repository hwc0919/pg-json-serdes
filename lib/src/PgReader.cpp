//
// Created by wanchen.he on 2023/1/12.
//
#include <pg_json/PgReader.h>

#include "PgBinaryReader.h"
#include "PgTextReader.h"

using namespace pg_json;

std::shared_ptr<PgReader> pg_json::PgReader::newTextReader()
{
    return std::make_shared<PgTextReader>();
}

std::shared_ptr<PgReader> pg_json::PgReader::newBinaryReader()
{
    return std::make_shared<PgBinaryReader>();
}
