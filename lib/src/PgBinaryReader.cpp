//
// Created by wanchen.he on 2023/1/12.
//
#include "PgBinaryReader.h"
#include "ByteOrder.h"

using namespace pg_json;

static int64_t readIntBE(Cursor & cursor, size_t size);

json_t PgBinaryReader::readPrimitive(const PgType & type, Cursor & cursor)
{
    size_t len;
    if (scopeStack_.empty())
    {
        len = cursor.remains();
    }
    else
    {
        assert(scopeStack_.back().type == ScopeType::ArrayElement
               || scopeStack_.back().type == ScopeType::CompositeField);
        if (scopeStack_.back().len == 0xffffffffu)
        {
            return nullptr;
        }
        len = scopeStack_.back().len;
    }

    switch (type.oid_)
    {
        case PG_BOOL: {
            return readIntBE(cursor, 1) != 0;
        }
        case PG_INT2:
        case PG_INT4:
        case PG_INT8: {
            return readIntBE(cursor, type.size_);
        }
        case PG_FLOAT4: {
            int32_t val = static_cast<int32_t>(readIntBE(cursor, 4));
            return *(reinterpret_cast<float *>(&val));
        }
        case PG_FLOAT8: {
            int64_t val = readIntBE(cursor, 8);
            return *(reinterpret_cast<double *>(&val));
        }
        case PG_TEXT:
        case PG_VARCHAR:
        case PG_JSON:
        case PG_JSONB: {
            if (type.oid_ == PG_JSONB)
            {
                if (len < 1 || *cursor.peek() != '\x01')
                {
                    throw std::runtime_error("Invalid jsonb");
                }
                cursor.advance(1);
            }
            if (len == 0)
            {
                return std::string{};
            }
            std::string str(cursor.peek(), len);
            cursor.advance(len);
            return str;
        }
        // case PG_TIME:
        // case PG_DATE:
        case PG_TIMESTAMP: {
            int64_t pgEpoch = readIntBE(cursor, 8);
            // 1970-1-1 0:0:0 --> 2000-1-1 0:0:0
            static unsigned long long PG_EPOCH_DELTA = 946684800L;
            // TODO: convert to string? or give a setting
            return pgEpoch / 1000 + PG_EPOCH_DELTA * 1000;
            //      ^us to ms      ^shift EPOCH from 2000 to 1970
        }
        default: {
            throw std::runtime_error("Unsupported pg type oid: " + std::to_string(type.oid_));
        }
    }
}

void PgBinaryReader::readArrayStart(const PgType & elemType, Cursor & cursor)
{
    using Oid = unsigned int;

    unsigned int dimension = readIntBE(cursor, sizeof(unsigned int));
    unsigned int nullMap = readIntBE(cursor, sizeof(unsigned int));
    Oid oid = readIntBE(cursor, sizeof(unsigned int));
    if (oid != elemType.oid_)
    {
        throw std::runtime_error("Oid mismatch");
    }
    size_t nElems = dimension == 0 ? 0 : 1;
    for (unsigned int i = 0; i < dimension; ++i)
    {
        unsigned int dimSize = readIntBE(cursor, sizeof(unsigned int));
        nElems *= dimSize;
    }
    for (unsigned int i = 0; i < dimension; ++i)
    {
        unsigned int _dimStartIdx = readIntBE(cursor, sizeof(unsigned int));
    }
    ScopeMark scope(ScopeType::Array);
    scope.len = nElems;
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryReader::readArrayEnd(Cursor & cursor)
{
    assert(!scopeStack_.empty()
           && scopeStack_.back().type == ScopeType::Array
           && scopeStack_.back().len == 0);
    scopeStack_.pop_back();
}

bool PgBinaryReader::hasMoreElement(Cursor & cursor)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::Array);
    return scopeStack_.back().len > 0;
}

void PgBinaryReader::readElementStart(Cursor & cursor)
{
    ScopeMark scope(ScopeType::ArrayElement);
    scope.len = readIntBE(cursor, sizeof(unsigned int));
    scope.offset = cursor.offset();
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryReader::readElementSeperator(Cursor & cursor)
{
    // do nothing
}

void PgBinaryReader::readElementEnd(Cursor & cursor)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::ArrayElement);
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    if (scope.offset + (scope.len == 0xffffffff ? 0 : scope.len) != cursor.offset())
    {
        throw std::runtime_error("Element length error");
    }

    assert(!scopeStack_.empty()
           && scopeStack_.back().type == ScopeType::Array
           && scopeStack_.back().len > 0);
    --scopeStack_.back().len;
}

void PgBinaryReader::readCompositeStart(const PgType & type, Cursor & cursor)
{
    ScopeMark scope(ScopeType::Composite);
    scope.len = readIntBE(cursor, sizeof(unsigned int));
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryReader::readCompositeEnd(Cursor & cursor)
{
    assert(!scopeStack_.empty()
           && scopeStack_.back().type == ScopeType::Composite
           && scopeStack_.back().len == 0);
    scopeStack_.pop_back();
}

void PgBinaryReader::readFieldStart(const PgType & fieldType, Cursor & cursor)
{
    if (fieldType.oid_ != readIntBE(cursor, sizeof(unsigned int)))
    {
        throw std::runtime_error("Field oid mismatch");
    }
    ScopeMark scope(ScopeType::CompositeField);
    scope.len = readIntBE(cursor, sizeof(unsigned int)); // 0xffffffff means null
    scope.offset = cursor.offset();
    scopeStack_.push_back(std::move(scope));
}

void PgBinaryReader::readFieldSeparator(Cursor & cursor)
{
    // do nothing
}

void PgBinaryReader::readFieldEnd(Cursor & cursor)
{
    assert(!scopeStack_.empty() && scopeStack_.back().type == ScopeType::CompositeField);
    ScopeMark scope = std::move(scopeStack_.back());
    scopeStack_.pop_back();
    if (scope.offset + (scope.len == 0xffffffff ? 0 : scope.len) != cursor.offset())
    {
        throw std::runtime_error("Field length error");
    }

    assert(!scopeStack_.empty()
           && scopeStack_.back().type == ScopeType::Composite
           && scopeStack_.back().len > 0);
    --scopeStack_.back().len;
}

static int64_t readIntBE(Cursor & cursor, size_t size)
{
    if (cursor.remains() < size)
    {
        throw std::runtime_error("Insufficient bytes to read");
    }
    const void * data = cursor.peek();
    cursor.advance(size);
    switch (size)
    {
        case 8: {
            return ByteOrder::hton(*static_cast<const int64_t *>(data));
        }
        case 4: {
            return ByteOrder::hton(*static_cast<const uint32_t *>(data));
        }
        case 2: {
            return ByteOrder::hton(*static_cast<const uint16_t *>(data));
        }
        case 1: {
            return *static_cast<const uint8_t *>(data);
        }
        default: {
            throw std::runtime_error("Invalid integer size, should be 1, 2, 4 or 8");
        }
    }
}
