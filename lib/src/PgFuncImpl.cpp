//
// Created by wanchen.he on 2023/1/8.
//
#include "PgFuncImpl.h"
#include "PgFieldImpl.h"
#include <pg_json/Buffer.h>

using namespace pg_json;

const std::string & PgFuncImpl::in_name(size_t i) const
{
    return in_params_[i].name();
}

const std::string & PgFuncImpl::out_name(size_t i) const
{
    return out_params_[i].name();
}

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
    return in_params_[i].type()->name();
}

const std::string & PgFuncImpl::out_type_name(size_t i) const
{
    return out_params_[i].type()->name();
}
