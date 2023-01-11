//
// Created by wanchen.he on 2023/1/11.
//
#include <cassert>
#include <pg_json/utils/PgTextReader.h>

using namespace pg_json;

#define CHECK_AND_ADVANCE_CH(cursor, ch)                             \
    do                                                               \
    {                                                                \
        if (!(cursor).remains())                                     \
        {                                                            \
            throw std::runtime_error("Cursor exhausted");            \
        }                                                            \
        if (*(cursor).peek() != (ch))                                \
        {                                                            \
            throw std::runtime_error("Invalid char, expected " #ch); \
        }                                                            \
        (cursor).advance(1);                                         \
    } while (0)

#define CHECK_AND_ADVANCE_LEN(cursor, len)                \
    do                                                    \
    {                                                     \
        if ((cursor).remains() < (len))                   \
        {                                                 \
            throw std::runtime_error("Cursor exhausted"); \
        }                                                 \
        (cursor).advance((len));                          \
    } while (0)

std::string readUntilAny(Cursor & cursor, const std::string & stops)
{
    size_t len = 0;
    while (len < cursor.remains() && stops.find(*(cursor.peek() + len)) == std::string::npos)
    {
        ++len;
    }
    return { cursor.peek(), len };
}

std::string readUntilEqual(Cursor & cursor, const std::string & end)
{
    size_t len = 0;
    while (len < cursor.remains() && 0 != strncmp(cursor.peek() + len, end.c_str(), end.size()))
    {
        ++len;
    }
    return { cursor.peek(), len };
}

nlohmann::json PgTextReader::readPrimitive(const PgType & type, Cursor & cursor)
{
    if (!cursor.remains())
    {
        // null
        return nullptr;
    }
    // Special treatments on text
    switch (type.oid_)
    {
        case PG_TEXT:
        case PG_VARCHAR:
        case PG_JSON:
        case PG_JSONB: {
            return readUnescapedString(cursor);
        }
    }

    std::string str = readWholeField(cursor);
    switch (type.oid_)
    {
        case PG_BOOL: {
            if (str.empty() || str.front() == 'f')
            {
                return false;
            }
            return true;
        }
        case PG_INT2:
        case PG_INT4:
        case PG_INT8: {
            return str.empty() ? 0LL : std::stoll(str);
        }
        case PG_FLOAT4:
        case PG_FLOAT8: {
            return str.empty() ? 0.0 : std::stod(str);
        }
        case PG_TIME:
        case PG_DATE:
        case PG_TIMESTAMP: {
            // no escape as long as input is correct
            return str;
        }
        default: {
            throw std::runtime_error("Unsupported pg type oid: " + std::to_string(type.oid_));
        }
    }
}

void PgTextReader::readArrayStart(const PgType & elemType, Cursor & cursor)
{
    CHECK_AND_ADVANCE_CH(cursor, '{');
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
}

void PgTextReader::readArrayEnd(Cursor & cursor)
{
    CHECK_AND_ADVANCE_CH(cursor, '}');
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Array)
    {
        throw std::runtime_error("Invalid array end");
    }
    scopeStack_.pop_back();
}

bool PgTextReader::hasMoreElement(Cursor & cursor)
{
    return cursor.remains() > 0 && *cursor.peek() != '}';
}

void PgTextReader::readElementStart(Cursor & cursor)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Array)
    {
        throw std::runtime_error("Invalid element start");
    }
    const ScopeMark & upperScope = scopeStack_.back();
    ScopeMark scope(ScopeType::ArrayElement);
    scope.quote = upperScope.quote;
    if (cursor.remains() >= upperScope.quote.size() && 0 == strncmp(cursor.peek(), upperScope.quote.c_str(), upperScope.quote.size()))
    {
        cursor.advance(upperScope.quote.size());
        scope.has_quote = true;
    }
    scopeStack_.push_back(std::move(scope));
}

void PgTextReader::readElementSeperator(Cursor & cursor)
{
    CHECK_AND_ADVANCE_CH(cursor, ',');
}

void PgTextReader::readElementEnd(Cursor & cursor)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::ArrayElement)
    {
        throw std::runtime_error("Invalid element end");
    }
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Array);
    if (scope.has_quote)
    {
        CHECK_AND_ADVANCE_LEN(cursor, scope.quote.size());
    }
}

void PgTextReader::readCompositeStart(const PgType & type, Cursor & cursor)
{
    CHECK_AND_ADVANCE_CH(cursor, '(');
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
}

void PgTextReader::readCompositeEnd(Cursor & cursor)
{
    CHECK_AND_ADVANCE_CH(cursor, ')');
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Composite)
    {
        throw std::runtime_error("Invalid composite end");
    }
    scopeStack_.pop_back();
}

void PgTextReader::readFieldStart(const PgType & fieldType, Cursor & cursor)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::Composite)
    {
        throw std::runtime_error("Invalid field start");
    }
    const ScopeMark & upperScope = scopeStack_.back();
    ScopeMark scope(ScopeType::CompositeField);
    scope.quote = upperScope.quote;
    if (cursor.remains() >= upperScope.quote.size()
        && 0 == strncmp(cursor.peek(), upperScope.quote.c_str(), upperScope.quote.size()))
    {
        cursor.advance(upperScope.quote.size());
        scope.has_quote = true;
    }
    scopeStack_.push_back(std::move(scope));
}

void PgTextReader::readFieldSeparator(Cursor & cursor)
{
    CHECK_AND_ADVANCE_CH(cursor, ',');
}

void PgTextReader::readFieldEnd(Cursor & cursor)
{
    if (scopeStack_.empty() || scopeStack_.back().type != ScopeType::CompositeField)
    {
        throw std::runtime_error("Invalid composite end");
    }
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Composite);

    if (scope.has_quote)
    {
        CHECK_AND_ADVANCE_LEN(cursor, scope.quote.size());
    }
}

std::string PgTextReader::readWholeField(Cursor & cursor) const
{
    size_t remains = cursor.remains();
    assert(remains);
    std::string str;
    if (scopeStack_.empty())
    {
        // read till end
        str = std::string(cursor.peek(), remains);
    }
    else
    {
        auto & upperScope = scopeStack_.back();
        if (upperScope.has_quote)
        { // read until quote
            str = readUntilEqual(cursor, upperScope.quote);
        }
        else
        {
            // read until seperator
            if (upperScope.type == ScopeType::ArrayElement)
            {
                str = readUntilAny(cursor, ",}");
            }
            else
            {
                assert(upperScope.type == ScopeType::CompositeField);
                str = readUntilAny(cursor, ",)");
            }
        }
    }
    cursor.advance(str.size());
    return str;
}

std::string PgTextReader::readUnescapedString(Cursor & cursor) const
{
    assert(cursor.remains());
    if (scopeStack_.empty())
    {
        return readWholeField(cursor);
    }
    const ScopeMark & upperScope = scopeStack_.back();
    assert(upperScope.type == ScopeType::ArrayElement || upperScope.type == ScopeType::CompositeField);
    if (!upperScope.has_quote)
    {
        return readWholeField(cursor);
    }

    std::string res;

    // cursor info
    const char * data = cursor.peek();
    size_t remains = cursor.remains();
    // quote info
    const std::string & quote = upperScope.quote;
    size_t quoteSize = quote.size();
    size_t escapeSize = quoteSize * 2;

    // Quote has been read by readXxxStart() methods, don't read them. Read the content directly
    size_t idx = 0;
    while (idx < remains - quoteSize)
    {
        if (data[idx] != '"' && data[idx] != '\\')
        {
            res.append(1, data[idx]);
            ++idx;
            continue;
        }
        // Reach an escaped char, or the end quote
        if (data[idx + quoteSize] != '"' && data[idx + quoteSize] != '\\')
        {
            // Must be the end quote, not enough length for escaped chars
            break;
        }
        assert(idx + escapeSize + quoteSize <= remains);
        char lastCh = data[idx + escapeSize - 1];
        assert(lastCh == '"' || lastCh == '\\');
        // Reverse escaping, the last character is the escaped character
        res.append(1, lastCh);
        idx += escapeSize;
    }
    cursor.advance(idx);
    return res;
}
