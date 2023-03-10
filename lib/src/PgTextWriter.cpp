//
// Created by wanchen.he on 2023/1/9.
//
#include "PgTextWriter.h"

using namespace pg_json;

void PgTextWriter::writePrimitive(const PgType & pgType, const json_t & jsonParam, Buffer & buf)
{
    switch (pgType.oid())
    {
        case PG_BOOL: {
            if (jsonParam.is_null() || jsonParam.empty()
                || jsonParam.is_number() && jsonParam == 0
                || jsonParam.is_boolean() && jsonParam == false
                || jsonParam.is_string() && (jsonParam == "f" || jsonParam == "false"))
            {
                buf.append(1, 'f');
            }
            else
            {
                buf.append(1, 't');
            }
            break;
        }
        case PG_INT2:
        case PG_INT4:
        case PG_INT8: {
            if (jsonParam.is_number())
            {
                std::string str = std::to_string(jsonParam.get<int64_t>());
                buf.append(str.c_str(), str.size());
            }
            else if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                buf.append(str.c_str(), str.size());
            }
            else
            {
                throw std::runtime_error("Invalid value for int");
            }
            break;
        }
        case PG_FLOAT4:
        case PG_FLOAT8: {
            std::string str;
            if (jsonParam.is_number_unsigned())
            {
                str = std::to_string(jsonParam.get<uint64_t>());
            }
            else if (jsonParam.is_number_integer())
            {
                str = std::to_string(jsonParam.get<int64_t>());
            }
            else if (jsonParam.is_number_float())
            {
                str = std::to_string(jsonParam.get<double>());
            }
            else if (jsonParam.is_string())
            {
                str = jsonParam.get<std::string>();
            }
            else
            {
                throw std::runtime_error("Invalid value for float");
            }
            buf.append(str.c_str(), str.size());
            break;
        }
        case PG_TEXT:
        case PG_VARCHAR:
        case PG_JSON:
        case PG_JSONB: {
            if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                writeUnescapedString(str, buf);
            }
            else
            {
                std::string str = jsonParam.dump();
                writeUnescapedString(str, buf);
            }
            break;
        }
        case PG_TIME: {
            if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                buf.append(str.c_str(), str.size());
            }
            else
            {
                throw std::runtime_error("Invalid value for time");
            }
            break;
        }
        case PG_DATE:
        case PG_TIMESTAMP: {
            if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                buf.append(str.c_str(), str.size());
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
                char timeBuf[32];
                int len = snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
                                   1900 + datetime.tm_year,
                                   1 + datetime.tm_mon,
                                   datetime.tm_mday,
                                   datetime.tm_hour,
                                   datetime.tm_min,
                                   datetime.tm_sec,
                                   static_cast<int>(epochMs % 1000));
                buf.append(timeBuf, len);
            }
            else
            {
                throw std::runtime_error("Invalid value for timestamp");
            }
            break;
        }
        default: {
            throw std::runtime_error("Unsupported pg type oid: " + std::to_string(pgType.oid()));
        }
    }
}

void PgTextWriter::writeArrayStart(const PgType & elemType, size_t len, Buffer & buf)
{
    ScopeMark scope(ScopeType::Array);
    if (scopeStack_.empty())
    {
        scope.quote = "\"";
    }
    else
    {
        const ScopeMark & upperScope = scopeStack_.back();
        if (upperScope.type == ScopeType::CompositeField)
        {
            scope.quote = upperScope.quote + upperScope.quote;
        }
        else if (upperScope.type == ScopeType::Array)
        {
            throw std::runtime_error("Invalid array start, nested array not supported");
        }
        else
        {
            throw std::runtime_error("Invalid array start");
        }
    }
    scopeStack_.push_back(std::move(scope));
    buf.append(1, '{');
}

void PgTextWriter::writeArrayEnd(Buffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Array)
    {
        throw std::runtime_error("Invalid array end");
    }
    scopeStack_.pop_back();
    buf.append(1, '}');
}

void PgTextWriter::writeElementStart(const PgType & elemType, Buffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Array)
    {
        throw std::runtime_error("Invalid element start");
    }
    const ScopeMark & upperScope = scopeStack_.back();

    ScopeMark scope(ScopeType::ArrayElement);
    scope.quote = upperScope.quote;
    if (needQuote(elemType))
    {
        scope.quoted = 1;
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
    scopeStack_.push_back(std::move(scope));
}

void PgTextWriter::writeElementSeperator(Buffer & buf)
{
    buf.append(1, ',');
}

void PgTextWriter::writeElementEnd(Buffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::ArrayElement)
    {
        throw std::runtime_error("Invalid element end");
    }
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Array);

    if (scope.quoted == 1)
    {
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
}

void PgTextWriter::writeCompositeStart(const PgType & type, Buffer & buf)
{
    ScopeMark scope(ScopeType::Composite);
    if (scopeStack_.empty())
    {
        scope.quote = "\"";
    }
    else
    {
        const ScopeMark & upperScope = scopeStack_.back();
        if (upperScope.type == ScopeType::ArrayElement) // array => composite
        {
            scope.quote = std::string(upperScope.quote.size(), '\\')
                          + upperScope.quote;
        }
        else if (upperScope.type == ScopeType::CompositeField) // composite => composite
        {
            scope.quote = upperScope.quote + upperScope.quote;
        }
        else
        {
            throw std::runtime_error("Invalid composite start");
        }
    }
    scopeStack_.push_back(std::move(scope));
    buf.append(1, '(');
}

void PgTextWriter::writeCompositeEnd(Buffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Composite)
    {
        throw std::runtime_error("Invalid composite end");
    }
    scopeStack_.pop_back();
    buf.append(1, ')');
}

void PgTextWriter::writeFieldStart(const PgType & fieldType, Buffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Composite)
    {
        throw std::runtime_error("Invalid field start");
    }
    const ScopeMark & upperScope = scopeStack_.back();

    ScopeMark scope(ScopeType::CompositeField);
    scope.quote = upperScope.quote;
    if (needQuote(fieldType))
    {
        scope.quoted = 1;
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
    scopeStack_.push_back(std::move(scope));
}

void PgTextWriter::writeFieldSeparator(Buffer & buf)
{
    buf.append(1, ',');
}

void PgTextWriter::writeFieldEnd(Buffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::CompositeField)
    {
        throw std::runtime_error("Invalid composite end");
    }
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Composite);

    if (scope.quoted == 1)
    {
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
}

void PgTextWriter::writeNullField(const PgType & fieldType, Buffer & buf)
{
    // nothing to write
}

static bool hasSpecialChar(const std::string & str)
{
    static const std::string specialChars = R"( '"\{},)";
    return std::any_of(str.begin(), str.end(), [](char c) {
        return specialChars.find(c) != std::string::npos;
    });
}

void PgTextWriter::writeUnescapedString(const std::string & str, Buffer & buf)
{
    if (scopeStack_.empty() || !hasSpecialChar(str))
    {
        buf.append(str);
        return;
    }

    ScopeMark & upperScope = scopeStack_.back();
    // Write quote
    assert(upperScope.quoted == 0);
    buf.append(upperScope.quote);
    for (char c : str)
    {
        switch (c) {
            case '"': {
                if (upperScope.type == ScopeType::CompositeField)
                {
                    buf.append(upperScope.quote);
                    buf.append(upperScope.quote);
                }
                else if (upperScope.type == ScopeType::ArrayElement)
                {
                    buf.append(upperScope.quote.size(), '\\');
                    buf.append(upperScope.quote);
                }
                else
                {
                    throw std::runtime_error("Invalid upper scope");
                }
                break;
            }
            case '\\': {
                buf.append(upperScope.quote.size() * 2, c);
                break;
            }
            default: {
                buf.append(1, c);
                break;
            }
        }
    }
    buf.append(upperScope.quote);
    upperScope.quoted = 2;
}

bool PgTextWriter::needQuote(const PgType & pgType)
{
    if (scopeStack_.empty() || pgType.isNumber() || pgType.isString())
    {
        // 1. No need to quote up-most fields
        // 2. No need to quote number as long as user input is correct.
        // 3. We don't know whether the parameter str needs quotes,
        // so we simply return false here, and let writeUnescapedString() handle the quotes.
        return false;
    }
    // Quote any other types
    return true;
}
