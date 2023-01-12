//
// Created by wanchen.he on 2023/1/12.
//
#include "PgTextReader.h"
#include <pg_json/PgReader.h>

using namespace pg_json;

std::shared_ptr<PgReader> pg_json::PgReader::newTextReader()
{
    return std::make_shared<PgTextReader>();
}

std::shared_ptr<PgReader> pg_json::PgReader::newBinaryReader()
{
    return nullptr;
}
