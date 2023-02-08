//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include <memory>
#include <string>

namespace pg_json
{
class PgType;
class PgField
{
public:
    virtual ~PgField() = default;
    // pg field name. For function parameters, name_[-1] is a direction indicator: 'i', 'o', 'b', 'v'.
    virtual const std::string & name() const = 0;
    virtual const PgType * type() const = 0;
};

} // namespace pg_json
