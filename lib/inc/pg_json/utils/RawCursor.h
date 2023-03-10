//
// Created by wanchen.he on 2023/1/11.
//
#pragma once

#include <cassert>
#include <pg_json/Cursor.h>

namespace pg_json
{
class RawCursor : public Cursor
{
public:
    RawCursor(const char * data, size_t len)
        : data_(data), len_(len), readOffset_(0)
    {
    }
    const char * peek() const override
    {
        return data_ + readOffset_;
    }
    size_t offset() const override
    {
        return readOffset_;
    }
    size_t remains() const override
    {
        return len_ - readOffset_;
    }
    void seek(size_t offset) override
    {
        assert(offset < len_);
        readOffset_ = offset;
    }
    void advance(size_t step) override
    {
        assert(readOffset_ + step <= len_);
        readOffset_ += step;
    }

protected:
    const char * data_;
    size_t len_;
    size_t readOffset_{ 0 };
};
} // namespace pg_json
