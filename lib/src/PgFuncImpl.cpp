//
// Created by wanchen.he on 2023/1/8.
//
#include "PgFuncImpl.h"
#include "PgType.h"

const std::string & pfs::PgFuncImpl::in_type_name(size_t i) const
{
    return in_params_[i].type_->name_;
}
const std::string & pfs::PgFuncImpl::out_type_name(size_t i) const
{
    return out_params_[i].type_->name_;
}
