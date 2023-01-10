//
// Created by wanchen.he on 2023/1/7.
//
#pragma once

namespace pfs
{
class IResult
{
public:
    virtual ~IResult() = default;
    virtual size_t rows() const noexcept = 0;
    virtual size_t columns() const noexcept = 0;
    virtual bool isNull(size_t row, size_t col) const noexcept = 0;
    virtual const char * getValue(size_t row, size_t col) const noexcept = 0;
    virtual size_t getLength(size_t row, size_t col) const noexcept = 0;
};
} // namespace pfs
