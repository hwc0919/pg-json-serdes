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

struct PgType
{
    using Oid = unsigned int;

    std::string name_;    // Name of type is not important, because we do parameter matching by parameter name not parameter type name.
    Oid oid_;             // Oid of the type in DB
    char category_;       // 'A'=Array, 'C'=Composite, 'S'=String, 'N'=Number, ...
    unsigned short size_; // If category_ == 'C', this is length of `fields_`; otherwise it is size of the type in bytes, 0 if variable sized.

    std::vector<PgField> fields_;
    std::shared_ptr<PgType> elem_type_; // Link to element type if this is an array type, or nullptr if this is a base type.

    bool isPrimitive() const
    {
        return category_ != 'A' && category_ != 'C';
    }
    bool isArray() const
    {
        return category_ == 'A';
    }
    bool isComposite() const
    {
        return category_ == 'C';
    }
    bool isString() const
    {
        return category_ == 'S';
    }
    bool isNumber() const
    {
        return category_ == 'N';
    }
};

} // namespace pg_json
