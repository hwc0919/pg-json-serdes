//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include "PgField.h"
#include <pfs/PgFunc.h>
#include <string>
#include <vector>

namespace pfs
{
class PgFuncImpl : public PgFunc
{
public:
    using Oid = unsigned int;

    PgFuncImpl(std::string nsp, std::string name)
        : nsp_(std::move(nsp)), name_(std::move(name))
    {
        cmp_name_ = nsp_ + "." + name_;
    }
    const std::string & namespace_() const override
    {
        return nsp_;
    }
    const std::string & name() const override
    {
        return name_;
    }
    size_t in_size() const override
    {
        return in_params_.size();
    }
    size_t out_size() const override
    {
        return out_params_.size();
    }
    const std::string & in_name(size_t i) const override
    {
        return in_params_[i].name_;
    }
    const std::string & out_name(size_t i) const override
    {
        return out_params_[i].name_;
    }
    const std::string & in_type_name(size_t i) const override;
    const std::string & out_type_name(size_t i) const override;
    std::string nsp_;
    std::string name_;
    std::string cmp_name_;

    std::vector<PgField> in_params_;
    std::vector<PgField> out_params_;

    // Derived data
    std::string stmt_;      // Statement to send to DB
    std::vector<Oid> oids_; // Parameter OID array
};

inline bool operator<(const std::shared_ptr<PgFuncImpl> & lhs, const std::shared_ptr<PgFuncImpl> & rhs)
{
    return lhs->cmp_name_ < rhs->cmp_name_;
}

inline bool operator==(const std::shared_ptr<PgFuncImpl> & lhs, const std::shared_ptr<PgFuncImpl> & rhs)
{
    return lhs->cmp_name_ < rhs->cmp_name_;
}

inline bool operator<(const std::shared_ptr<PgFuncImpl> & lhs, const std::string & cmp_name)
{
    return lhs->cmp_name_ < cmp_name;
}

inline bool operator==(const std::shared_ptr<PgFuncImpl> & lhs, const std::string & cmp_name)
{
    return lhs->cmp_name_ == cmp_name;
}

inline bool operator<(const std::string & cmp_name, const std::shared_ptr<PgFuncImpl> & rhs)
{
    return cmp_name < rhs->cmp_name_;
}

inline bool operator==(const std::string & cmp_name, const std::shared_ptr<PgFuncImpl> & rhs)
{
    return cmp_name == rhs->cmp_name_;
}

} // namespace pfs
