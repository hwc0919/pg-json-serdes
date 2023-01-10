//
// Created by wanchen.he on 2023/1/9.
//
#pragma once

#include <cstddef>

namespace pfs
{
class IBuffer
{
public:
    virtual void append(const char * data, size_t len) = 0;

    virtual const char * data() const = 0;
    virtual size_t size() const = 0;
};

} // namespace pfs
