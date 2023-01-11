//
// Created by wanchen.he on 2023/1/10.
//
#include <pg_json/IBuffer.h>
#include <pg_json/IParamSetter.h>
#include <pg_json/PgFunc.h>
#include <string>

using namespace pg_json;

class RawCursor
{
public:
    RawCursor(const char * data, size_t len)
        : data_(data), len_(len)
    {
    }

    const char * peek() const
    {
        return data_ + readOffset_;
    }
    size_t offset() const
    {
        return readOffset_;
    }
    size_t remains() const
    {
        return len_ - readOffset_;
    }
    void seek(size_t offset)
    {
        assert(offset < len_);
        readOffset_ = offset;
    }
    void advance(size_t step)
    {
        assert(readOffset_ + step <= len_);
        readOffset_ += step;
    }

protected:
    const char * data_;
    size_t len_;
    size_t readOffset_{ 0 };
};

static void serialize(const PgType & pgType, const nlohmann::json & param, IPgWriter & writer, IBuffer & buffer);

void pg_json::PgFunc::parseJsonToParams(const nlohmann::json & obj, const pg_json::PgFunc & func, pg_json::IParamSetter & setter, pg_json::IPgWriter & writer, IBuffer & buffer)
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
                writer.writeElementSeperator(buffer);
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

nlohmann::json deserialize(const PgType & pgType, IPgReader & reader, Cursor & cursor)
{
    if (pgType.isPrimitive())
    {
        return reader.readPrimitive(pgType, cursor);
    }
    else if (pgType.isArray())
    {
        assert(pgType.elem_type_);
        nlohmann::json arr(nlohmann::json::value_t::array);

        const PgType & elemType = *pgType.elem_type_;
        reader.readArrayStart(elemType, cursor);
        size_t nElems = 0;
        // Element count can not be decided in text result
        while (reader.hasMoreElement(cursor))
        {
            if (nElems++ > 0)
            {
                reader.readElementSeperator(cursor);
            }
            // recurse
            reader.readElementStart(cursor);
            nlohmann::json elem = deserialize(elemType, reader, cursor);
            arr.push_back(std::move(elem));
            reader.readElementEnd(cursor);
        }
        reader.readArrayEnd(cursor);
        return arr;
    }
    else
    {
        nlohmann::json obj(nlohmann::json::value_t::object);
        reader.readCompositeStart(pgType, cursor);
        // Field count is known from metadata
        for (size_t i = 0; i != pgType.fields_.size(); ++i)
        {
            if (i > 0)
            {
                reader.readFieldSeparator(cursor);
            }
            const PgField & field = pgType.fields_[i];
            assert(field.type_ && !field.name_.empty());
            const PgType & fieldType = *field.type_;
            const std::string & name = field.name_;

            // recurse
            reader.readFieldStart(fieldType, cursor);
            nlohmann::json val = deserialize(fieldType, reader, cursor);
            obj[name] = std::move(val);
            reader.readFieldEnd(cursor);
        }
        reader.readCompositeEnd(cursor);
        return obj;
    }
}

nlohmann::json PgFunc::parseResultToJson(const PgFunc & func, const IResult & result, IPgReader & reader, Cursor & cursor)
{
    nlohmann::json root;
    for (size_t idx = 0; idx != func.out_size(); ++idx)
    {
        auto & field = func.out_field(idx);
        const std::string & name = field.name_;

        // Field is null
        if (result.isNull(0, idx))
        {
            root[name] = nlohmann::json(nlohmann::json::value_t::null);
            continue;
        }
        const char * data = result.getValue(0, idx);
        size_t len = result.getLength(0, idx);
        if (data == nullptr)
        {
            // Should not happen, but in case
            root[name] = nlohmann::json(nlohmann::json::value_t::null);
            continue;
        }

        cursor.reset(data, len);
        nlohmann::json value = deserialize(*field.type_, reader, cursor);
        root[name] = value;
    }
    return root;
}
