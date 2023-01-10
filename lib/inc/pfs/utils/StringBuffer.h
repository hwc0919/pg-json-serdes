#pragma once

#include <pfs/IBuffer.h>
#include <string>

namespace pfs
{
class StringBuffer : public IBuffer
{
public:
    void clear() override
    {
        buf_.clear();
    }
    void append(const std::string & str) override
    {
        buf_.append(str);
    }
    void append(const char * data, size_t len) override
    {
        buf_.append(data, len);
    }
    void append(size_t n, char ch) override
    {
        buf_.append(n, ch);
    }
    const char * data() const override
    {
        return buf_.c_str();
    }
    size_t size() const override
    {
        return buf_.size();
    }

private:
    std::string buf_;
};
} // namespace pfs