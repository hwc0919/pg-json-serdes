//
// Created by wanchen.he on 2023/1/6.
//

#pragma once

#include <pfs/Catalogue.h>
#include <pfs/PgFunc.h>

namespace pfs
{

struct PgType;
struct PgField
{
    std::string name_; // CFieldInfo name. For parameters, [-1] is a direction indicator: 'i', 'o', 'b', 'v'.
    std::shared_ptr<PgType> type_;
};

struct PgType
{
    using Oid = unsigned int;

    std::string name_;    // Name of type is not important, because we do parameter matching by parameter name not parameter type name.
    Oid oid_;             // Oid of the type in DB
    char category_;       // 'A'=Array, 'C'=Composite, 'S'=String, 'N'=Number, ...
    unsigned short size_; // If category_ == 'C', this is length of `fields_`; otherwise it is size of the type in bytes, 0 if variable sized.

    std::vector<PgField> fields_;
    std::shared_ptr<PgType> elemType_; // Link to element type if this is an array type, or nullptr if this is a base type.
};

class PgFuncImpl : public PgFunc
{
public:
    using Oid = unsigned int;

    std::string pronsp_;
    std::string proname_;

    std::vector<PgField> inParams_;
    std::vector<PgField> outParams_;

    // Derived data
    std::string stmt_;      // Statement to send to DB
    std::vector<Oid> oids_; // Parameter OID array
};

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
