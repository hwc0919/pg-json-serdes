//
// Created by wanchen.he on 2023/1/6.
//
#pragma once

#include <memory>
#include <pg_json/Catalogue.h>
#include <vector>

namespace pg_json
{
class PgTypeImpl;
class PgFuncImpl;
class CatalogueImpl : public Catalogue
{
public:
    explicit CatalogueImpl(std::shared_ptr<PgResult> meta_res);
    ~CatalogueImpl() override;
    void parseMeta();

    std::vector<std::shared_ptr<PgFunc>> findFunctions(const std::string & name) override
    {
        static const std::string defaultNamespace("public");
        return findFunctions(name, defaultNamespace);
    }
    std::vector<std::shared_ptr<PgFunc>> findFunctions(const std::string & name, const std::string & nsp) override;

private:
    struct MetaRow
    {
        char type_{ 0 };
        char category_{ 0 };
        int idx_{ 0 };
        int oid_{ 0 };
        int len_{ 0 };
        int elem_type_idx_{ 0 }; // For array
        std::string nsp_;
        std::string name_;
    };
    MetaRow parseMetaRow(int row);
    int parseType(const MetaRow & meta, int row);
    int parseFunction(const MetaRow & meta, int row);
    // parse user-defined composite types
    int parseUdt(const MetaRow & meta, int row);

    void prepareFunctions();

    std::shared_ptr<PgResult> meta_res_;
    bool parsed_{ false };

    std::vector<std::shared_ptr<PgTypeImpl>> types_;
    std::vector<std::shared_ptr<PgFuncImpl>> funcs_;
};
} // namespace pg_json
