//
// Created by wanchen.he on 2023/1/12.
//

#include "PgBinaryWriter.h"
#include "ByteOrder.h"

using namespace pg_json;

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
        case PG_FLOAT4: {
            float number;
            if (jsonParam.is_number())
            {
                number = jsonParam.get<float>();
            }
            else if (jsonParam.is_string())
            {
                number = std::stof(jsonParam.get<std::string>());
            }
            else
            {
                throw std::runtime_error("Invalid json for float");
            }
            writeIntBE(buf, *reinterpret_cast<unsigned int *>(&number), 4);
            break;
        }
        case PG_FLOAT8: {
            double number;
            if (jsonParam.is_number())
            {
                number = jsonParam.get<double>();
            }
            else if (jsonParam.is_string())
            {
                number = std::stod(jsonParam.get<std::string>());
            }
            else
            {
                throw std::runtime_error("Invalid json for double");
            }
            writeIntBE(buf, *reinterpret_cast<unsigned long long *>(&number), 8);
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
            int64_t epochMs;
            if (jsonParam.is_string())
            {
                std::string str = jsonParam.get<std::string>();
                // TODO: parse datetime string to epochMs
                epochMs = 1577808000000;
            }
            else if (jsonParam.is_number())
            {
                epochMs = jsonParam.get<int64_t>();
            }
            else
            {
                throw std::runtime_error("Invalid json for timestamp");
            }
            // 1970-1-1 0:0:0 --> 2000-1-1 0:0:0
            static int64_t PG_EPOCH_DELTA = 946684800LL;
            int64_t pgEpoch = (epochMs - PG_EPOCH_DELTA * 1000) * 1000;
            writeIntBE(buf, pgEpoch, 8);
            break;
        }
        default: {
            throw std::runtime_error("Unsupported pg type oid: " + std::to_string(pgType.oid_));
        }
    }
}

bool PgBinaryWriter::needQuote(const PgType & pgType, const json_t & jsonParam)
{
    return false;
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

void PgBinaryWriter::writeElementStart(Buffer & buf, bool)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Array);
    ScopeMark scope(ScopeType::ArrayElement);
    scope.offset = buf.size();              // field start offset
    buf.append(sizeof(unsigned int), '\0'); // Reserve 4 bytes for field length
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryWriter::writeElementSeperator(Buffer & buf)
{
    // no separator needed in binary format
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
    printf("Write element end, len = %zu\n", len);
    *reinterpret_cast<unsigned *>(data) = ByteOrder::hton(static_cast<unsigned>(len));
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

void PgBinaryWriter::writeFieldStart(const PgType & fieldType, Buffer & buf, bool)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Composite);
    writeIntBE(buf, fieldType.oid_, sizeof(unsigned int)); // write field oid
    ScopeMark scope(ScopeType::CompositeField);
    scope.offset = buf.size();
    buf.append(sizeof(unsigned int), '\0'); // Reserve 4 bytes for field length
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryWriter::writeFieldSeparator(Buffer & buf)
{
    // no separator
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

void PgBinaryWriter::writeNullField(const PgType & fieldType, Buffer & buf)
{
    writeIntBE(buf, fieldType.oid_, sizeof(unsigned int));
    writeIntBE(buf, 0xFFFFFFFF, sizeof(unsigned int));
}
