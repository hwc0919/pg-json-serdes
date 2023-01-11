//
// Created by wanchen.he on 2023/1/11.
//
#pragma once

#include <cstddef>

namespace pg_json
{
class Cursor
{
public:
    virtual ~Cursor() = default;
    // Reset cursor to a new piece of data
    virtual void reset(const char * data, size_t len) = 0;
    // current position
    virtual const char * peek() const = 0;
    // current read offset
    virtual size_t offset() const = 0;
    // remaining bytes
    virtual size_t remains() const = 0;
    // set read offset
    virtual void seek(size_t offset) = 0;
    // advance read cursor
    virtual void advance(size_t step) = 0;
};
} // namespace pg_json
