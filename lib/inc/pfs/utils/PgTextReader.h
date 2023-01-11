//
// Created by wanchen.he on 2023/1/11.
//
#pragma once

#include <pfs/IPgReader.h>

namespace pfs
{
class PgTextReader : public IPgReader
{
public:
    nlohmann::json readPrimitive(const PgType & type, Cursor & cursor) override;
    void readArrayStart(const PgType & elemType, Cursor & cursor) override;
    void readArrayEnd(Cursor & cursor) override;
    bool hasMoreElement(Cursor & cursor) override;
    void readElementStart(Cursor & cursor) override;
    void readElementSeperator(Cursor & cursor) override;
    void readElementEnd(Cursor & cursor) override;
    void readCompositeStart(const PgType & type, Cursor & buf) override;
    void readCompositeEnd(Cursor & buf) override;
    void readFieldStart(const PgType & fieldType, Cursor & buf) override;
    void readFieldSeparator(Cursor & buf) override;
    void readFieldEnd(Cursor & buf) override;

protected:
    std::string readWholeField(Cursor & cursor) const;
    std::string readUnescapedString(Cursor & cursor) const;

    enum class ScopeType
    {
        None,
        Array,
        ArrayElement,
        Composite,
        CompositeField
    };

    struct ScopeMark
    {
        explicit ScopeMark(ScopeType typ)
            : type(typ){};
        ScopeType type{ ScopeType::None };
        bool has_quote{ false };
        std::string quote;
    };

    std::vector<ScopeMark> scopeStack_;
};
} // namespace pfs
