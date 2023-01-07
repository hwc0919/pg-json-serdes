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
    virtual int rows() const = 0;
    virtual int columns() const = 0;
    virtual bool isNull(int row, int col) const = 0;
    virtual const char * getValue(int row, int col) const = 0;
    virtual int getLength(int row, int col) const = 0;
};
} // namespace pfs
