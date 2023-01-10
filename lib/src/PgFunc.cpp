//
// Created by wanchen.he on 2023/1/10.
//
#include <pfs/IBuffer.h>
#include <pfs/IParamSetter.h>
#include <pfs/PgFunc.h>
#include <string>

using namespace pfs;

static void serialize(const PgType & pgType, const nlohmann::json & param, IPgWriter & writer, IBuffer & buffer);

void pfs::PgFunc::parseJsonToParams(const nlohmann::json & obj, const pfs::PgFunc & func, pfs::IParamSetter & setter, pfs::IPgWriter & writer, IBuffer & buffer)
{
    setter.setSize(func.in_size());
    for (size_t idx = 0; idx != func.in_size(); ++idx)
    {
        auto & field = func.in_field(idx);
        const std::string & name = field.name_;

        // Ignore non-exist field
        if (!obj.contains(name))
        {
            continue;
        }
        auto & jsonParam = obj[name];
        // No need to set null
        if (jsonParam.is_null())
        {
            continue;
        }
        buffer.clear();
        serialize(*field.type_, jsonParam, writer, buffer);
        setter.setParameter(idx, buffer.data(), buffer.size(), 0);
    }
}

static void serialize(const PgType & pgType, const nlohmann::json & param, IPgWriter & writer, IBuffer & buffer)
{
    if (pgType.isPrimitive())
    {
        writer.writePrimitive(pgType, param, buffer);
    }
    else if (pgType.isArray())
    {
        size_t len = param.is_array() ? param.size() : 1;
        assert(pgType.elem_type_);
        const PgType & elemType = *pgType.elem_type_;

        writer.writeArrayStart(elemType, len, buffer);
        for (size_t i = 0; i < len; ++i)
        {
            if (i > 0)
            {
                writer.writeSeperator(buffer);
            }
            const nlohmann::json & jsonElem = param.is_array() ? param[i] : param;
            bool needQuote = writer.needQuote(elemType, jsonElem);
            writer.writeElementStart(buffer, needQuote);
            // recurse
            serialize(elemType, jsonElem, writer, buffer);
            writer.writeElementEnd(buffer);
        }
        writer.writeArrayEnd(buffer);
    }
    else
    {
        if (!param.is_object())
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
            // TODO: set default handler?
            if (!param.contains(name))
            {
                writer.writeFieldStart(fieldType, buffer, false);
                writer.writeFieldEnd(buffer);
                continue;
            }
            auto & fieldParam = param[name];
            // explicit null value
            if (fieldParam.is_null())
            {
                writer.writeNullField(*field.type_, buffer);
                continue;
            }
            bool needQuote = writer.needQuote(fieldType, fieldParam);
            writer.writeFieldStart(fieldType, buffer, needQuote);
            // recurse
            serialize(fieldType, fieldParam, writer, buffer);
            writer.writeFieldEnd(buffer);
        }
        writer.writeCompositeEnd(buffer);
    }
}
