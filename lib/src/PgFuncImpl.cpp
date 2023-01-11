//
// Created by wanchen.he on 2023/1/8.
//
#include "PgFuncImpl.h"
#include <pg_json/IBuffer.h>
#include <pg_json/PgType.h>

using namespace pg_json;

const PgField & PgFuncImpl::in_field(size_t i) const
{
    return in_params_[i];
}

const PgField & PgFuncImpl::out_field(size_t i) const
{
    return out_params_[i];
}

const std::string & PgFuncImpl::in_type_name(size_t i) const
{
    return in_params_[i].type_->name_;
}

const std::string & PgFuncImpl::out_type_name(size_t i) const
{
    return out_params_[i].type_->name_;
}
