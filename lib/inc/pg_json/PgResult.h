//
// Created by wanchen.he on 2023/1/7.
//
#pragma once

namespace pg_json
{
class PgResult
{
public:
    virtual ~PgResult() = default;
    virtual size_t rows() const noexcept = 0;
    virtual size_t columns() const noexcept = 0;
    virtual bool isNull(size_t row, size_t col) const noexcept = 0;
    virtual const char * getValue(size_t row, size_t col) const noexcept = 0;
    virtual size_t getLength(size_t row, size_t col) const noexcept = 0;
};
} // namespace pg_json
