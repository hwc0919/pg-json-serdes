//
// Created by wanchen.he on 2023/1/6.
//
#pragma once

#include <memory>
#include <pfs/Catalogue.h>
#include <vector>

namespace pfs
{
class PgType;
class PgFuncImpl;
class CatalogueImpl : public Catalogue
{
public:
    explicit CatalogueImpl(std::shared_ptr<IResult> meta_res);
    void parseMeta();

    std::vector<std::shared_ptr<PgFunc>> findFunction(const std::string & name) override;

private:
    struct MetaRow
    {
        char type_{ 0 };
        char category_{ 0 };
        int idx_{ 0 };
        int oid_{ 0 };
        int len_{ 0 };
        int elem_type_idx_{ 0 }; // For array
        std::string name_;
        std::string namespace_;
    };
    MetaRow parseMetaRow(int row);
    int parseType(const MetaRow & meta, int row);
    int parseFunction(const MetaRow & meta, int row);

    // parse user-defined composite types
    int parseUdt(const MetaRow & meta, int row);

    std::shared_ptr<IResult> meta_res_;
    bool parsed_{ false };

    std::vector<std::shared_ptr<PgType>> types_;
    std::vector<std::shared_ptr<PgFuncImpl>> funcs_;
};
} // namespace pfs
