//
// Created by wanchen.he on 2023/1/8.
//
#include "PgFuncImpl.h"
#include <iostream>
#include <pfs/IBuffer.h>
#include <pfs/PgType.h>

using namespace pfs;

const std::string & PgFuncImpl::in_type_name(size_t i) const
{
    return in_params_[i].type_->name_;
}

const std::string & PgFuncImpl::out_type_name(size_t i) const
{
    return out_params_[i].type_->name_;
}

class StringBuffer : public IBuffer
{
public:
    void append(const char * data, size_t len) override
    {
        buf_.append(data, len);
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

void PgFuncImpl::setParamsFromJson(const nlohmann::json & paramObj, IParamSetter & setter, IPgWriter & writer)
{
    setter.setSize(in_params_.size());
    for (size_t idx = 0; idx != in_params_.size(); ++idx)
    {
        auto & field = in_params_[idx];
        const std::string & name = field.name_;
        // Ignore non-exist field
        if (!paramObj.contains(name))
        {
            continue;
        }
        auto & jsonParam = paramObj[name];
        // No need to set null
        if (jsonParam.is_null())
        {
            continue;
        }
        StringBuffer buffer;
        serialize(*field.type_, jsonParam, writer, buffer);
        setter.setParameter(idx, buffer.data(), buffer.size(), 0);
    }
}

static bool hasSpecialChar(const std::string & str)
{
    static const std::string specialChars = R"( '"\{},)";
    for (char c : str)
    {
        if (!std::isprint(c))
        {
            return false;
        }
        if (specialChars.find(c) != std::string::npos)
        {
            return false;
        }
    }
    return true;
}

void PgFuncImpl::serialize(const PgType & pgType, const nlohmann::json & jsonParam, IPgWriter & writer, IBuffer & buffer)
{
    if (pgType.isPrimitive())
    {
        writer.writePrimitive(pgType, jsonParam, buffer);
    }
    else if (pgType.isArray())
    {
        size_t len = jsonParam.is_array() ? jsonParam.size() : 1;
        assert(pgType.elem_type_);
        const PgType & elemType = *pgType.elem_type_;

        writer.writeArrayStart(elemType, len, buffer);
        for (size_t i = 0; i < len; ++i)
        {
            if (i > 0)
            {
                writer.writeSeperator(buffer);
            }
            // TODO: how to decide whether we need quote in advance?
            writer.writeElementStart(buffer, true);
            // recurse
            if (jsonParam.is_array())
            {
                serialize(elemType, jsonParam[i], writer, buffer);
            }
            else
            {
                serialize(elemType, jsonParam, writer, buffer);
            }
            writer.writeElementEnd(buffer);
        }
        writer.writeArrayEnd(buffer);
    }
    else
    {
        if (!jsonParam.is_object())
        {
            throw std::runtime_error("Should be object");
        }
        writer.writeCompositeStart(pgType, buffer);
        for (size_t i = 0; i != pgType.fields_.size(); ++i)
        {
            if (i > 0)
            {
                writer.writeFieldSeparator(buffer);
            }
            const PgField & field = pgType.fields_[i];
            assert(field.type_ && !field.name_.empty());
            const PgType & fieldType = *field.type_;
            const std::string & name = field.name_;
            // Field not found, leave it emtpy
            if (!jsonParam.contains(name))
            {
                writer.writeFieldStart(fieldType, buffer, false);
                writer.writeFieldEnd(buffer);
                continue;
            }
            auto & fieldParam = jsonParam[name];
            // explicit null value
            if (jsonParam.is_null())
            {
                writer.writeNullField(*field.type_, buffer);
                continue;
            }
            writer.writeFieldStart(fieldType, buffer, true);
            serialize(fieldType, jsonParam, writer, buffer);
            writer.writeFieldEnd(buffer);
        }
        writer.writeCompositeEnd(buffer);
    }
}
