//
// Created by wanchen.he on 2023/2/8.
//
#pragma once
#include <pg_json/PgField.h>

namespace pg_json
{
class PgTypeImpl;
class PgFieldImpl : public PgField
{
public:
    PgFieldImpl(std::string name, std::shared_ptr<PgTypeImpl> type);

    const std::string & name() const override
    {
        return name_;
    }
    const PgType * type() const override;

private:
    std::string name_;
    std::shared_ptr<PgTypeImpl> type_;
};
} // namespace pg_json
