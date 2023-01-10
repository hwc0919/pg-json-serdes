//
// Created by wanchen.he on 2023/1/9.
//
#include <pfs/utils/PgTextWriter.h>
void pfs::PgTextWriter::writePrimitive(const pfs::PgType & pgType, const nlohmann::json & jsonParam, pfs::IBuffer & buf)
{
    switch (pgType.oid_)
    {
        case PG_BOOL: {
            if (jsonParam.is_null() || jsonParam.empty()
                || jsonParam.is_number() && jsonParam == 0
                || jsonParam.is_boolean() && jsonParam == false
                || jsonParam.is_string() && (jsonParam == "f" || jsonParam == "false"))
            {
                buf.append("f", 1);
            }
            else
            {
                buf.append("t", 1);
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
            if (jsonParam.is_number())
            {
                std::string str = std::to_string(jsonParam.get<double>());
                buf.append(str.c_str(), str.size());
            }
            else if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                buf.append(str.c_str(), str.size());
            }
            else
            {
                throw std::runtime_error("Invalid value for float");
            }
            break;
        }
        case PG_TEXT:
        case PG_VARCHAR:
        case PG_JSON:
        case PG_JSONB: {
            if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                buf.append(str.c_str(), str.size());
            }
            else
            {
                std::string str = jsonParam.dump();
                buf.append(str.c_str(), str.size());
            }
            break;
        }
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
            throw std::runtime_error("Unsupported pg type oid: " + std::to_string(pgType.oid_));
        }
    }
}

void pfs::PgTextWriter::writeArrayStart(const pfs::PgType & elemType, size_t len, pfs::IBuffer & buf)
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
    buf.append("{", 1);
}

void pfs::PgTextWriter::writeArrayEnd(pfs::IBuffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Array)
    {
        throw std::runtime_error("Invalid array end");
    }
    scopeStack_.pop_back();
    buf.append("}", 1);
}

void pfs::PgTextWriter::writeElementStart(pfs::IBuffer & buf, bool needQuote)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Array)
    {
        throw std::runtime_error("Invalid element start");
    }
    const ScopeMark & upperScope = scopeStack_.back();

    ScopeMark scope(ScopeType::ArrayElement);
    scope.need_quote = needQuote;
    scope.quote = upperScope.quote;
    if (scope.need_quote)
    {
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
    scopeStack_.push_back(std::move(scope));
}

void pfs::PgTextWriter::writeSeperator(pfs::IBuffer & buf)
{
    buf.append(",", 1);
}

void pfs::PgTextWriter::writeElementEnd(pfs::IBuffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::ArrayElement)
    {
        throw std::runtime_error("Invalid element end");
    }
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Array);

    if (scope.need_quote)
    {
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
}

void pfs::PgTextWriter::writeCompositeStart(const pfs::PgType & type, pfs::IBuffer & buf)
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
            scope.quote = std::string(scopeStack_.back().quote.size(), '\\')
                          + scopeStack_.back().quote;
        }
        else if (upperScope.type == ScopeType::CompositeField) // composite => composite
        {
            scope.quote = upperScope.quote + upperScope.quote;
        }
        else
        {
            throw std::runtime_error("Invalid array start");
        }
    }
    scopeStack_.push_back(std::move(scope));
    buf.append("(", 1);
}

void pfs::PgTextWriter::writeCompositeEnd(IBuffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Composite)
    {
        throw std::runtime_error("Invalid composite end");
    }
    scopeStack_.pop_back();
    buf.append(")", 1);
}

void pfs::PgTextWriter::writeFieldStart(const PgType & fieldType, IBuffer & buf, bool needQuote)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Composite)
    {
        throw std::runtime_error("Invalid field start");
    }
    const ScopeMark & upperScope = scopeStack_.back();

    ScopeMark scope(ScopeType::CompositeField);
    scope.need_quote = needQuote;
    scope.quote = upperScope.quote;

    if (scope.need_quote)
    {
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
    scopeStack_.push_back(std::move(scope));
}

void pfs::PgTextWriter::writeFieldSeparator(pfs::IBuffer & buf)
{
    buf.append(",", 1);
}

void pfs::PgTextWriter::writeFieldEnd(IBuffer & buf)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::CompositeField)
    {
        throw std::runtime_error("Invalid composite end");
    }
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Composite);

    if (scope.need_quote)
    {
        buf.append(scope.quote.c_str(), scope.quote.size());
    }
}

void pfs::PgTextWriter::writeNullField(const pfs::PgType & fieldType, pfs::IBuffer & buf)
{
    buf.append("null", 4);
}
