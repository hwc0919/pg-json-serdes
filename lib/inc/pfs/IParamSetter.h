//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include <stddef.h>

namespace pfs
{

struct IParamSetter
{
    virtual ~IParamSetter() = default;

    virtual void setSize(size_t n) = 0;

    virtual void setString(size_t idx, std::string str) = 0;
    virtual void setLong(size_t idx, int64_t num) = 0;
    virtual void setDouble(size_t idx, double num) = 0;
    virtual void setData(size_t idx, const char * data, size_t len, int format) = 0;
};

} // namespace pfs
