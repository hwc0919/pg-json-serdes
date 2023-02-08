//
// Created by wanchen.he on 2023/2/8.
//

#pragma once

#include "PgFieldImpl.h"
#include <cstddef>
#include <pg_json/PgType.h>

namespace pg_json
{
class PgTypeImpl : public PgType
{
    friend class CatalogueImpl;

public:
    Oid oid() const override
    {
        return oid_;
    }
    const std::string & name() const override
    {
        return name_;
    }
    char category() const override
    {
        return category_;
    }
    size_t size() const override
    {
        return size_;
    }
    size_t numFields() const override
    {
        return fields_.size();
    }
    const PgType * elemType() const override
    {
        return elemType_.get();
    }
    const PgField & field(size_t idx) const override
    {
        return fields_[idx];
    }

    bool isPrimitive() const override
    {
        return category_ != 'A' && category_ != 'C';
    }
    bool isArray() const override
    {
        return category_ == 'A';
    }
    bool isComposite() const override
    {
        return category_ == 'C';
    }
    bool isString() const override
    {
        return category_ == 'S';
    }
    bool isNumber() const override
    {
        return category_ == 'N';
    }

private:
    std::string name_;         // Name of type is not important, because we do parameter matching by parameter name not parameter type name.
    Oid oid_{ 0 };             // Oid of the type in DB
    char category_{ 0 };       // 'A'=Array, 'C'=Composite, 'S'=String, 'N'=Number, ...
    unsigned short size_{ 0 }; // If category_ == 'C', this is length of `fields_`; otherwise it is size of the type in bytes, 0 if variable sized.

    std::vector<PgFieldImpl> fields_;
    std::shared_ptr<PgTypeImpl> elemType_; // Link to element type if this is an array type, or nullptr if this is a base type.
};
} // namespace pg_json
