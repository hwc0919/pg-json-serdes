//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include "PgField.h"
#include <memory>
#include <string>
#include <vector>

namespace pfs
{
struct PgType
{
    using Oid = unsigned int;

    std::string name_;    // Name of type is not important, because we do parameter matching by parameter name not parameter type name.
    Oid oid_;             // Oid of the type in DB
    char category_;       // 'A'=Array, 'C'=Composite, 'S'=String, 'N'=Number, ...
    unsigned short size_; // If category_ == 'C', this is length of `fields_`; otherwise it is size of the type in bytes, 0 if variable sized.

    std::vector<PgField> fields_;
    std::shared_ptr<PgType> elem_type_; // Link to element type if this is an array type, or nullptr if this is a base type.
};

} // namespace pfs
