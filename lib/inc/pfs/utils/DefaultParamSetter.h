//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include <memory>
#include <pfs/IParamSetter.h>
#include <string>
#include <vector>

namespace pfs
{

/**
 * Default implementation of IParamSetter
 */
struct DefaultParamSetter : public IParamSetter
{
public:
    DefaultParamSetter() = default;

    void setSize(size_t n) override
    {
        paramValues_.resize(n, nullptr);
        paramLens_.resize(n, 0);
        paramFormats_.resize(n, 0);
    }

    void setString(size_t idx, std::string str) override
    {
        std::shared_ptr<std::string> obj = std::make_shared<std::string>(std::move(str));
        objs_.push_back(obj);
        setDataInner(idx, *obj, 0);
    }
    void setLong(size_t idx, int64_t num) override
    {
        std::shared_ptr<std::string> obj = std::make_shared<std::string>(std::to_string(num));
        objs_.push_back(obj);
        setDataInner(idx, *obj, 0);
    }
    void setDouble(size_t idx, double num) override
    {
        std::shared_ptr<std::string> obj = std::make_shared<std::string>(std::to_string(num));
        objs_.push_back(obj);
        setDataInner(idx, *obj, 0);
    }

    void setData(size_t idx, const char * data, size_t len, int format) override
    {
        std::shared_ptr<std::string> obj = std::make_shared<std::string>(data, len);
        objs_.push_back(obj);
        setDataInner(idx, *obj, format);
    }

    const std::vector<const char *> & getParamValues()
    {
        return paramValues_;
    }

    const std::vector<int> & getParamLens()
    {
        return paramLens_;
    }

    const std::vector<int> & getParamFormats()
    {
        return paramFormats_;
    }

private:
    void setDataInner(size_t idx, const std::string & data, int format)
    {
        paramValues_[idx] = data.c_str();
        paramLens_[idx] = static_cast<int>(data.size());
        paramFormats_[idx] = format;
    }

    std::vector<std::shared_ptr<void>> objs_;
    std::vector<const char *> paramValues_;
    std::vector<int> paramLens_;
    std::vector<int> paramFormats_; // 0: text, 1: binary. Text format requires paramValues_ to be '\0' terminated.
};
} // namespace pfs
