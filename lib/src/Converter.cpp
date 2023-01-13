#include "ConverterImpl.h"
#include <pg_json/Converter.h>
using namespace pg_json;

std::shared_ptr<Converter> Converter::newConverter(PgFormat format)
{
    return std::make_shared<ConverterImpl>(format);
}

void Converter::parseJsonToPg(const PgType & pgType, const json_t & param, PgWriter & writer, Buffer & buffer)
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
            const json_t & jsonElem = param.is_array() ? param[i] : param;
            bool needQuote = writer.needQuote(elemType, jsonElem);
            writer.writeElementStart(buffer, needQuote);
            // recurse
            parseJsonToPg(elemType, jsonElem, writer, buffer);
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
                writer.writeNullField(*field.type_, buffer);
                // writer.writeFieldStart(fieldType, buffer, false);
                // writeDefaultValue(...)
                // writer.writeFieldEnd(buffer);
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
            parseJsonToPg(fieldType, fieldParam, writer, buffer);
            writer.writeFieldEnd(buffer);
        }
        writer.writeCompositeEnd(buffer);
    }
}

json_t Converter::parsePgToJson(const PgType & pgType, PgReader & reader, Cursor & cursor)
{
    if (pgType.isPrimitive())
    {
        return reader.readPrimitive(pgType, cursor);
    }
    else if (pgType.isArray())
    {
        assert(pgType.elem_type_);
        json_t arr(json_t::value_t::array);

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
            json_t elem = parsePgToJson(elemType, reader, cursor);
            arr.push_back(std::move(elem));
            reader.readElementEnd(cursor);
        }
        reader.readArrayEnd(cursor);
        return arr;
    }
    else
    {
        json_t obj(json_t::value_t::object);
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
            json_t val = parsePgToJson(fieldType, reader, cursor);
            obj[name] = std::move(val);
            reader.readFieldEnd(cursor);
        }
        reader.readCompositeEnd(cursor);
        return obj;
    }
}
