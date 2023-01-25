//
// Created by wanchen.he on 2023/1/12.
//

#include "PgBinaryWriter.h"
#include "ByteOrder.h"

using namespace pg_json;

// 1970-1-1 0:0:0 --> 2000-1-1 0:0:0
#define PG_EPOCH_DELTA        946684800L
#define MICRO_SECONDS_PRE_SEC 1000000LL

static int64_t parseDatetimeString(const std::string & datetime);

static void writeIntBE(Buffer & buf, unsigned long long number, size_t size)
{
    size_t offset = buf.size();
    buf.append(size, '\0');
    char * data = buf.data() + offset;
    switch (size)
    {
        case 8: {
            *reinterpret_cast<unsigned long long *>(data) = ByteOrder::hton(number);
            break;
        }
        case 4: {
            *reinterpret_cast<unsigned int *>(data) = ByteOrder::hton((unsigned int)number);
            break;
        }
        case 2: {
            *reinterpret_cast<unsigned short *>(data) = ByteOrder::hton((unsigned short)number);
            break;
        }
        case 1: {
            *reinterpret_cast<unsigned char *>(data) = ByteOrder::hton((unsigned short)number);
            break;
        }
        default:
            throw std::runtime_error("Invalid size of integer");
    }
}

void PgBinaryWriter::writePrimitive(const PgType & pgType, const json_t & jsonParam, Buffer & buf)
{
    switch (pgType.oid_)
    {
        case PG_BOOL: {
            if (jsonParam.is_null() || jsonParam.empty()
                || jsonParam.is_number() && jsonParam == 0
                || jsonParam.is_boolean() && jsonParam == false
                || jsonParam.is_string() && (jsonParam == "f" || jsonParam == "false"))
            {
                buf.append(1, '\0');
            }
            else
            {
                buf.append(1, '\1');
            }
            break;
        }
        case PG_INT2:
        case PG_INT4:
        case PG_INT8: {
            int64_t number;
            if (jsonParam.is_number())
            {
                number = jsonParam.get<int64_t>();
            }
            else if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                number = std::stoll(str);
            }
            else
            {
                throw std::runtime_error("Invalid value for int");
            }
            // Write number in big endian order
            writeIntBE(buf, number, pgType.size_);
            break;
        }
        case PG_FLOAT4:
        case PG_FLOAT8: {
            double number;
            if (jsonParam.is_number_float())
            {
                number = jsonParam.get<double>();
            }
            else if (jsonParam.is_number_unsigned())
            {
                number = (double)jsonParam.get<uint64_t>();
            }
            else if (jsonParam.is_number_integer())
            {
                number = (double)jsonParam.get<int64_t>();
            }
            else if (jsonParam.is_string())
            {
                number = std::stod(jsonParam.get<std::string>());
            }
            else
            {
                throw std::runtime_error("Invalid json for float");
            }
            if (pgType.oid_ == PG_FLOAT4)
            {
                float fNumber = (float)number;
                writeIntBE(buf, *reinterpret_cast<unsigned int *>(&fNumber), 4);
            }
            else
            {
                writeIntBE(buf, *reinterpret_cast<unsigned long long *>(&number), 8);
            }
            break;
        }
        case PG_TEXT:
        case PG_VARCHAR:
        case PG_JSON:
        case PG_JSONB: {
            std::string str = jsonParam.is_string()
                                  ? jsonParam.get<std::string>()
                                  : jsonParam.dump();
            if (pgType.oid_ == PG_JSONB)
            {
                buf.append(1, '\x01');
            }
            buf.append(str);
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
            int64_t epochMicro; // milliseconds

            if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                epochMicro = parseDatetimeString(str);
            }
            else if (jsonParam.is_number())
            {
                epochMicro = jsonParam.get<int64_t>() * 1000;
            }
            else
            {
                throw std::runtime_error("Invalid json for timestamp");
            }
            int64_t pgEpoch = epochMicro - PG_EPOCH_DELTA * 1000 * 1000;
            writeIntBE(buf, pgEpoch, 8);
            break;
        }
        default: {
            throw std::runtime_error("Unsupported pg type oid: " + std::to_string(pgType.oid_));
        }
    }
}

void PgBinaryWriter::writeArrayStart(const PgType & elemType, size_t len, Buffer & buf)
{
    writeIntBE(buf, 1, sizeof(unsigned int));
    writeIntBE(buf, 0, sizeof(unsigned int));
    writeIntBE(buf, elemType.oid_, sizeof(unsigned int));
    writeIntBE(buf, len, sizeof(unsigned int));
    writeIntBE(buf, 1, sizeof(unsigned int));
    scopeStack_.emplace_back(ScopeType::Array);
}

void PgBinaryWriter::writeArrayEnd(Buffer & buf)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Array);
    scopeStack_.pop_back();
}

void PgBinaryWriter::writeElementStart(const PgType &, Buffer & buf)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Array);
    ScopeMark scope(ScopeType::ArrayElement);
    scope.offset = buf.size();              // field start offset
    buf.append(sizeof(unsigned int), '\0'); // Reserve 4 bytes for field length
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryWriter::writeElementEnd(Buffer & buf)
{
    assert(!scopeStack_.empty());
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(scope.type == ScopeType::ArrayElement);

    // Write field length into reserved position
    assert(buf.size() >= scope.offset + sizeof(unsigned));
    char * data = buf.data() + scope.offset;
    size_t len = buf.size() - scope.offset - sizeof(unsigned);
    *reinterpret_cast<unsigned *>(data) = ByteOrder::hton(static_cast<unsigned>(len));
}

void PgBinaryWriter::writeElementSeperator(Buffer & buf)
{
    // no separator needed in binary format
}

void PgBinaryWriter::writeCompositeStart(const PgType & type, Buffer & buf)
{
    scopeStack_.emplace_back(ScopeType::Composite);
    // composite field number is known in advance
    writeIntBE(buf, type.fields_.size(), sizeof(unsigned int));
}

void PgBinaryWriter::writeCompositeEnd(Buffer & buf)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Composite);
    scopeStack_.pop_back();
}

void PgBinaryWriter::writeFieldStart(const PgType & fieldType, Buffer & buf)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Composite);
    writeIntBE(buf, fieldType.oid_, sizeof(unsigned int)); // write field oid
    ScopeMark scope(ScopeType::CompositeField);
    scope.offset = buf.size();
    buf.append(sizeof(unsigned int), '\0'); // Reserve 4 bytes for field length
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryWriter::writeFieldEnd(Buffer & buf)
{
    assert(!scopeStack_.empty());
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    assert(scope.type == ScopeType::CompositeField);

    // Write field length into reserved position
    assert(buf.size() >= scope.offset + sizeof(unsigned));
    char * data = buf.data() + scope.offset;
    size_t len = buf.size() - scope.offset - sizeof(unsigned);
    *reinterpret_cast<unsigned *>(data) = ByteOrder::hton(static_cast<unsigned>(len));
}

void PgBinaryWriter::writeFieldSeparator(Buffer & buf)
{
    // no separator
}

void PgBinaryWriter::writeNullField(const PgType & fieldType, Buffer & buf)
{
    writeIntBE(buf, fieldType.oid_, sizeof(unsigned int));
    writeIntBE(buf, 0xFFFFFFFF, sizeof(unsigned int));
}

static std::vector<std::string> splitString(const std::string & s,
                                            const std::string & delimiter,
                                            bool acceptEmptyString = false)
{
    if (delimiter.empty())
        return std::vector<std::string>{};
    std::vector<std::string> v;
    size_t last = 0;
    size_t next = 0;
    while ((next = s.find(delimiter, last)) != std::string::npos)
    {
        if (next > last || acceptEmptyString)
            v.push_back(s.substr(last, next - last));
        last = next + delimiter.length();
    }
    if (s.length() > last || acceptEmptyString)
        v.push_back(s.substr(last));
    return v;
}

static int64_t parseDatetimeString(const std::string & datetime)
{
    int year = { 0 }, month = { 0 }, day = { 0 },
        hour = { 0 }, minute = { 0 }, second = { 0 }, microSecond = { 0 };

    // Try parse %Y-%m-%d %H:%M:%S
    std::vector<std::string> v = splitString(datetime, " ");
    if (v.size() != 2)
    {
        // Try parse %Y-%m-%d'T'%H:%M:%S
        std::vector<std::string> v1 = splitString(datetime, "T");
        if (v1.size() == 2)
        {
            v = std::move(v1);
        }
    }

    if (v.empty())
    {
        throw std::runtime_error("Invalid datetime string, should be %Y-%m-%d %H:%M:%S");
    }
    // parse time
    std::vector<std::string> date = splitString(v[0], "-");
    if (date.size() != 3)
    {
        throw std::runtime_error("Invalid date string, should be %Y-%m-%d");
    }
    year = std::stoi(date[0]);
    month = std::stoi(date[1]);
    day = std::stoi(date[2]);
    if (v.size() >= 2)
    {
        // parse time
        std::vector<std::string> time = splitString(v[1], ":");
        if (time.size() < 2 || time.size() > 3)
        {
            throw std::runtime_error("Invalid time string, should be %H:%M:%S.xxxxxx");
        }
        hour = std::stoi(time[0]);
        minute = std::stoi(time[1]);
        if (time.size() > 2) // 3
        {
            // %H:%M:%S'Z'
            if (!time[2].empty() && !std::isdigit(time[2].back()))
            {
                time[2].pop_back();
            }
            auto seconds = splitString(time[2], ".");
            second = std::stoi(seconds[0]);
            if (seconds.size() > 1)
            {
                if (seconds[1].length() > 6)
                {
                    seconds[1].resize(6);
                }
                else if (seconds[1].length() < 6)
                {
                    seconds[1].append(6 - seconds[1].length(), '0');
                }
                microSecond = std::stoi(seconds[1]);
            }
        }
    }
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_isdst = -1;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    // time_t epoch = mktime(&tm); // local
    time_t epoch = timegm(&tm); // utc, but not in c++ standard
    int64_t micro = static_cast<int64_t>(epoch) * MICRO_SECONDS_PRE_SEC + microSecond;
    return micro;
}
