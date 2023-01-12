#pragma once

#include <pg_json/Buffer.h>
#include <string>

namespace pg_json
{
class StringBuffer : public Buffer
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
    char * data() const override
    {
        return const_cast<char *>(buf_.data());
    }
    size_t size() const override
    {
        return buf_.size();
    }

private:
    std::string buf_;
};
} // namespace pg_json
