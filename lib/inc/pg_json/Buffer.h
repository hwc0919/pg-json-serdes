//
// Created by wanchen.he on 2023/1/9.
//
#pragma once

#include <cstddef>
#include <string>

namespace pg_json
{
class Buffer
{
public:
    virtual ~Buffer() = default;
    virtual void clear() = 0;
    virtual void append(const std::string & str) = 0;
    virtual void append(const char * data, size_t len) = 0;
    virtual void append(size_t n, char ch) = 0;

    virtual char * data() const = 0;
    virtual size_t size() const = 0;
};

} // namespace pg_json
