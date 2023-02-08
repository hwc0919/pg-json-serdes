#include <pg_json/Converter.h>
#include <pg_json/utils/RawCursor.h>
#include <pg_json/utils/StringBuffer.h>

using namespace pg_json;

static std::shared_ptr<Buffer> defaultBufferFactory()
{
    return std::make_shared<StringBuffer>();
};
static std::shared_ptr<Cursor> defaultCursorFactory(const char * data, size_t len)
{
    return std::make_shared<RawCursor>(data, len);
};
static json_t defaultNullHandler(const PgType & pgType, bool explicitNull)
{
    return json_t(nullptr);
}

Converter::Converter(PgFormat format)
    : format_(format)
    , bufferFactory_(defaultBufferFactory)
    , cursorFactory_(defaultCursorFactory)
    , nullHandler_(defaultNullHandler)
{
}

void Converter::parseJsonToParams(const PgFunc & func, const json_t & obj, PgParamSetter & setter) const
{
    auto writer = format_ == PgFormat::kText ? PgWriter::newTextWriter()
                                             : PgWriter::newBinaryWriter();
    setter.setSize(func.in_size());
    for (size_t idx = 0; idx != func.in_size(); ++idx)
    {
        auto & field = func.in_field(idx);
        const std::string & name = field.name();

        json_t tmp;
        const json_t * tmpPtr;
        if (!obj.contains(name))
        {
            tmp = nullHandler_(*field.type(), false);
            tmpPtr = &tmp;
        }
        else if (obj[name].is_null())
        {
            tmp = nullHandler_(*field.type(), true);
            tmpPtr = &tmp;
        }
        else
        {
            tmpPtr = &(obj[name]);
        }

        const json_t & jsonParam = *tmpPtr;
        // No need to set null
        if (jsonParam.is_null())
        {
            continue;
        }
        auto buffer = bufferFactory_();
        parseJsonToPg(*field.type(), jsonParam, *writer, *buffer);
        setter.setParameter(idx, buffer->data(), buffer->size(), format_);
    }
}

json_t Converter::parseResultToJson(const PgFunc & func, const PgResult & result) const
{
    json_t root;
    auto reader = format_ == PgFormat::kText ? PgReader::newTextReader()
                                             : PgReader::newBinaryReader();
    for (size_t idx = 0; idx != func.out_size(); ++idx)
    {
        auto & field = func.out_field(idx);
        const std::string & name = field.name();

        // Field is null
        if (result.isNull(0, idx))
        {
            root[name] = json_t(json_t::value_t::null);
            continue;
        }
        const char * data = result.getValue(0, idx);
        size_t len = result.getLength(0, idx);
        if (data == nullptr)
        {
            // Should not happen, but in case
            root[name] = json_t(json_t::value_t::null);
            continue;
        }

        auto cursor = cursorFactory_(data, len);
        json_t value = parsePgToJson(*field.type(), *reader, *cursor);
        root[name] = value;
    }
    return root;
}

void Converter::parseJsonToPg(const PgType & pgType, const json_t & param, PgWriter & writer, Buffer & buffer) const
{
    if (pgType.isPrimitive())
    {
        writer.writePrimitive(pgType, param, buffer);
    }
    else if (pgType.isArray())
    {
        size_t len = param.is_array() ? param.size() : 1;
        assert(pgType.elemType());
        const PgType & elemType = *pgType.elemType();

        writer.writeArrayStart(elemType, len, buffer);
        for (size_t i = 0; i < len; ++i)
        {
            if (i > 0)
            {
                writer.writeElementSeperator(buffer);
            }
            json_t tmp;
            const json_t * tmpPtr = param.is_array() ? &param[i] : &param;
            if (tmpPtr->is_null())
            {
                tmp = nullHandler_(elemType, true);
                tmpPtr = &tmp;
            }
            const json_t & jsonElem = *tmpPtr;
            if (jsonElem.is_null())
            {
                throw std::runtime_error("Null array element is not supported yet.");
            }
            writer.writeElementStart(elemType, buffer);
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
        for (size_t i = 0; i != pgType.numFields(); ++i)
        {
            if (i > 0)
            {
                writer.writeFieldSeparator(buffer);
            }
            const PgField & field = pgType.field(i);
            assert(field.type() && !field.name().empty());
            const PgType & fieldType = *field.type();
            const std::string & name = field.name();
            // Field not found, leave it emtpy

            json_t tmp;
            const json_t * tmpPtr;
            if (!param.contains(name))
            {
                tmp = nullHandler_(*field.type(), false);
                tmpPtr = &tmp;
            }
            else if (param[name].is_null())
            {
                tmp = nullHandler_(*field.type(), true);
                tmpPtr = &tmp;
            }
            else
            {
                tmpPtr = &(param[name]);
            }

            auto & fieldParam = *tmpPtr;
            if (fieldParam.is_null())
            {
                writer.writeNullField(*field.type(), buffer);
                continue;
            }
            writer.writeFieldStart(fieldType, buffer);
            // recurse
            parseJsonToPg(fieldType, fieldParam, writer, buffer);
            writer.writeFieldEnd(buffer);
        }
        writer.writeCompositeEnd(buffer);
    }
}

json_t Converter::parsePgToJson(const PgType & pgType, PgReader & reader, Cursor & cursor) const
{
    if (pgType.isPrimitive())
    {
        return reader.readPrimitive(pgType, cursor);
    }
    else if (pgType.isArray())
    {
        assert(pgType.elemType());
        json_t arr(json_t::value_t::array);

        const PgType & elemType = *pgType.elemType();
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
        for (size_t i = 0; i != pgType.numFields(); ++i)
        {
            if (i > 0)
            {
                reader.readFieldSeparator(cursor);
            }
            const PgField & field = pgType.field(i);
            assert(field.type() && !field.name().empty());
            const PgType & fieldType = *field.type();
            const std::string & name = field.name();

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
