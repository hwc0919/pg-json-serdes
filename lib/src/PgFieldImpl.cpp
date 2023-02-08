//
// Created by wanchen.he on 2023/2/8.
//

#include "PgFieldImpl.h"
#include "PgTypeImpl.h"

using namespace pg_json;

PgFieldImpl::PgFieldImpl(std::string name, std::shared_ptr<pg_json::PgTypeImpl> type)
    : name_(std::move(name))
    , type_(std::move(type))
{
}

const PgType * PgFieldImpl::type() const
{
    return type_.get();
}
