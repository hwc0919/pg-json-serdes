//
// Created by wanchen.he on 2023/1/8.
//
#include "PgFuncImpl.h"
#include "PgType.h"
#include <iostream>

using namespace pfs;

const std::string & PgFuncImpl::in_type_name(size_t i) const
{
    return in_params_[i].type_->name_;
}

const std::string & PgFuncImpl::out_type_name(size_t i) const
{
    return out_params_[i].type_->name_;
}

void PgFuncImpl::setParamsFromJson(const nlohmann::json & paramObj, IParamSetter & setter)
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
        setParam(idx, jsonParam, setter);
    }
}

void PgFuncImpl::setParam(size_t idx, const nlohmann::json & jsonParam, IParamSetter & setter)
{
    auto & field = in_params_[idx];
    if (field.type_->isPrimitive())
    {
        setPrimitiveParam(idx, jsonParam, setter);
    }
    else if (field.type_->isArray())
    {
        setArrayParam(idx, jsonParam, setter);
    }
    else // Is composite
    {
        assert(field.type_->isComposite());
        setCompositeParam(idx, jsonParam, setter);
    }
}

void PgFuncImpl::setPrimitiveParam(size_t idx, const nlohmann::json & jsonParam, IParamSetter & setter)
{
    auto & field = in_params_[idx];
    switch (field.type_->oid_)
    {
        case PG_BOOL: {
            if (jsonParam.is_number() && jsonParam == 0
                || jsonParam.is_boolean() && jsonParam == false
                || jsonParam.is_string() && (jsonParam == "f" || jsonParam == "false"))
            {
                setter.setString(idx, "f");
            }
            else
            {
                setter.setString(idx, "t");
            }
            break;
        }
        case PG_INT2:
        case PG_INT4:
        case PG_INT8: {
            if (jsonParam.is_number())
            {
                setter.setLong(idx, jsonParam.get<int64_t>());
            }
            else if (jsonParam.is_string())
            {
                setter.setString(idx, jsonParam.get<std::string>());
            }
            else
            {
                std::cout << "Invalid value for pg int: " << jsonParam << std::endl;
            }
            break;
        }
        case PG_FLOAT4:
        case PG_FLOAT8: {
            if (jsonParam.is_number())
            {
                setter.setDouble(idx, jsonParam.get<double>());
            }
            else if (jsonParam.is_string())
            {
                setter.setString(idx, jsonParam.get<std::string>());
            }
            else
            {
                std::cout << "Invalid value for pg float: " << jsonParam << std::endl;
            }
            break;
        }
        case PG_TEXT:
        case PG_VARCHAR:
        case PG_JSON:
        case PG_JSONB: {
            if (jsonParam.is_string())
            {
                setter.setString(idx, jsonParam.get<std::string>());
            }
            else
            {
                setter.setString(idx, jsonParam.dump());
            }
            break;
        }
        case PG_TIMESTAMP: {
            if (jsonParam.is_string())
            {
                setter.setString(idx, jsonParam.get<std::string>());
            }
            else if (jsonParam.is_number())
            {
                int64_t epochMs = jsonParam.get<int64_t>();
                time_t epoch = (epochMs / 1000);
                struct tm datetime
                {
                };
                if (gmtime_r(&epoch, &datetime) == nullptr)
                {
                    throw std::runtime_error("gmtime_r failed");
                }

                // Print as ISO 8601 format
                char buf[32];
                int len = snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
                                   1900 + datetime.tm_year,
                                   1 + datetime.tm_mon,
                                   datetime.tm_mday,
                                   datetime.tm_hour,
                                   datetime.tm_min,
                                   datetime.tm_sec,
                                   static_cast<int>(epochMs % 1000));
                setter.setData(idx, buf, len, 0);
            }
            else
            {
                std::cout << "Unsupported json value for timestamp: " << jsonParam << std::endl;
            }
            break;
        }
        default: {
            // TODO: set default converter
            std::cout << "Unsupported pg type oid: " << field.type_->oid_ << std::endl;
        }
    }
}

void PgFuncImpl::setArrayParam(size_t idx, const nlohmann::json & jsonParam, IParamSetter & setter)
{
}

void PgFuncImpl::setCompositeParam(size_t idx, const nlohmann::json & jsonParam, IParamSetter & setter)
{
}
