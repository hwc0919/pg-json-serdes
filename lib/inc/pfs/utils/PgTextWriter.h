//
// Created by wanchen.he on 2023/1/9.
//
#pragma once
#include <pfs/IPgWriter.h>

namespace pfs
{
class PgTextWriter : public IPgWriter
{
public:
    void writePrimitive(const PgType & pgType, const nlohmann::json & jsonParam, IBuffer & buf) override;
    bool needQuote(const PgType & pgType, const nlohmann::json & jsonParam) override;
    void writeArrayStart(const PgType & elemType, size_t len, IBuffer & buf) override;
    void writeArrayEnd(IBuffer & buf) override;
    void writeElementStart(IBuffer & buf, bool needQuote) override;
    void writeSeperator(IBuffer & buf) override;
    void writeElementEnd(IBuffer & buf) override;
    void writeCompositeStart(const PgType & type, IBuffer & buf) override;
    void writeCompositeEnd(IBuffer & buf) override;
    void writeFieldStart(const PgType & fieldType, IBuffer & buf, bool needQuote) override;
    void writeFieldSeparator(IBuffer & buf) override;
    void writeFieldEnd(IBuffer & buf) override;
    void writeNullField(const PgType & fieldType, IBuffer & buf) override;

protected:
    void writeUnescapedString(const std::string &, IBuffer &);

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
        bool need_quote{ false };
        std::string quote;
    };

    std::vector<ScopeMark> scopeStack_;
};

} // namespace pfs
