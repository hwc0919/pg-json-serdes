//
// Created by wanchen.he on 2023/1/13.
//

#include "ConverterImpl.h"
#include <pg_json/utils/RawCursor.h>
#include <pg_json/utils/StringBuffer.h>

using namespace pg_json;

static auto defaultBufferFactory = []() {
    return std::make_shared<StringBuffer>();
};
static auto defaultCursorFactory = [](const char * data, size_t len) {
    return std::make_shared<RawCursor>(data, len);
};

ConverterImpl::ConverterImpl(PgFormat format)
    : format_(format), bufferFactory_(defaultBufferFactory), cursorFactory_(defaultCursorFactory)
{
}

void ConverterImpl::parseJsonToParams(const PgFunc & func, const json_t & obj, PgParamSetter & setter) const
{
    auto writer = format_ == PgFormat::kText ? PgWriter::newTextWriter()
                                             : PgWriter::newBinaryWriter();
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
        auto buffer = bufferFactory_();
        parseJsonToPg(*field.type_, jsonParam, *writer, *buffer);
        setter.setParameter(idx, buffer->data(), buffer->size(), format_);
    }
}

json_t ConverterImpl::parseResultToJson(const PgFunc & func, const PgResult & result) const
{
    json_t root;
    auto reader = format_ == PgFormat::kText ? PgReader::newTextReader()
                                             : PgReader::newBinaryReader();
    for (size_t idx = 0; idx != func.out_size(); ++idx)
    {
        auto & field = func.out_field(idx);
        const std::string & name = field.name_;

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
        json_t value = parsePgToJson(*field.type_, *reader, *cursor);
        root[name] = value;
    }
    return root;
}
