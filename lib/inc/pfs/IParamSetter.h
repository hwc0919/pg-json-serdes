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
    virtual void setParameter(size_t idx, const char * data, size_t len, int format) = 0;
};

} // namespace pfs
