//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include <memory>
#include <string>

namespace pg_json
{
struct PgType;
struct PgField
{
    // pg field name. For function parameters, name_[-1] is a direction indicator: 'i', 'o', 'b', 'v'.
    std::string name_;
    std::shared_ptr<PgType> type_;
};

} // namespace pg_json
