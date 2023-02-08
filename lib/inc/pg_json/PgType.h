//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include <memory>
#include <pg_json/PgField.h>
#include <string>
#include <vector>

namespace pg_json
{
enum OidType
{
    PG_BOOL = 16,
    PG_INT8 = 20,
    PG_INT2 = 21,
    PG_INT4 = 23,
    PG_TEXT = 25,
    PG_JSON = 114,
    PG_FLOAT4 = 700,
    PG_FLOAT8 = 701,
    PG_VARCHAR = 1043,
    PG_DATE = 1082,
    PG_TIME = 1083,
    PG_TIMESTAMP = 1114,
    PG_JSONB = 3802
};

class PgType
{
public:
    using Oid = unsigned int;

    virtual ~PgType() = default;
    virtual Oid oid() const = 0;
    virtual const std::string & name() const = 0;
    virtual char category() const = 0;
    virtual size_t size() const = 0;
    virtual size_t numFields() const = 0;
    virtual const PgType * elemType() const = 0;
    virtual const PgField & field(size_t idx) const = 0;

    virtual bool isPrimitive() const = 0;
    virtual bool isArray() const = 0;
    virtual bool isComposite() const = 0;
    virtual bool isString() const = 0;
    virtual bool isNumber() const = 0;
};

} // namespace pg_json
