//
// Created by wanchen.he on 2023/1/12.
//
#pragma once
#include <string>

namespace pg_json
{
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
    bool quoted{ false };
    std::string quote;
    size_t offset{ 0 }; // for binary
    size_t len{ 0 };    // for binary array
};
} // namespace pg_json
