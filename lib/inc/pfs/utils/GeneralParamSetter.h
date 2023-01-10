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
struct GeneralParamSetter : public IParamSetter
{
public:
    void setSize(size_t n) override
    {
        paramValues_.resize(n, nullptr);
        paramLens_.resize(n, 0);
        paramFormats_.resize(n, 0);
    }

    void setParameter(size_t idx, const char * data, size_t len, int format) override
    {
        assert(idx < paramValues_.size());
        std::shared_ptr<std::string> obj = std::make_shared<std::string>(data, len);
        objs_.push_back(obj);
        paramValues_[idx] = obj->data();
        paramLens_[idx] = static_cast<int>(len);
        paramFormats_[idx] = format;
    }

    size_t size() const
    {
        return paramValues_.size();
    }

    const std::vector<const char *> & getParamValues() const
    {
        return paramValues_;
    }

    const std::vector<int> & getParamLens() const
    {
        return paramLens_;
    }

    const std::vector<int> & getParamFormats() const
    {
        return paramFormats_;
    }

private:
    std::vector<std::shared_ptr<void>> objs_;
    std::vector<const char *> paramValues_;
    std::vector<int> paramLens_;
    std::vector<int> paramFormats_; // 0: text, 1: binary. Text format requires paramValues_ to be '\0' terminated.
};
} // namespace pfs
