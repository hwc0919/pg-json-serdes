//
// Created by wanchen.he on 2023/1/6.
//

#include "CatalogueImpl.h"
#include <pfs/IResult.h>
#include <stdexcept>

pfs::CatalogueImpl::CatalogueImpl(std::shared_ptr<IResult> meta_res)
    : meta_res_(std::move(meta_res))
{
}

void pfs::CatalogueImpl::parseMeta()
{
    if (parsed_)
        return;
    parsed_ = true;
    if (meta_res_->rows() < 2)
    {
        throw std::runtime_error("Invalid result");
    }

    for (int i = 0; i < meta_res_->rows();)
    {
        MetaRow meta = parseMetaRow(i++);
        int consumedRows = 0;
        switch (meta.type_) {
            case 'T': {
                consumedRows = parseType(meta, i);
                break;
            }
            case 'F': {
                consumedRows = parseFunction(meta, i);
                break;
            }
            default:
                throw std::runtime_error("Bad meta type");
        }
        if (consumedRows < 0)
        {
            throw std::runtime_error("Parse failed");
        }
        i += consumedRows;
    }
}

std::vector<std::shared_ptr<pfs::PgFunc>> pfs::CatalogueImpl::findFunction(const std::string & name)
{
    return {};
}

pfs::CatalogueImpl::MetaRow pfs::CatalogueImpl::parseMetaRow(int row)
{
    MetaRow meta;
    if (!meta_res_->isNull(row, 0))
    {
        if (meta_res_->getLength(row, 0) < 2)
        {
            throw std::runtime_error("Bad row");
        }
        const char * data = meta_res_->getValue(row, 0);
        meta.type_ = data[0];
        meta.category_ = data[1];
    }
    if (!meta_res_->isNull(row, 1))
    {
        std::string number{ meta_res_->getValue(row, 1), (size_t)meta_res_->getLength(row, 1) };
        meta.idx_ = std::stoi(number);
    }

    if (!meta_res_->isNull(row, 2))
    {
        std::string number{ meta_res_->getValue(row, 2), (size_t)meta_res_->getLength(row, 2) };
        meta.oid_ = std::stoi(number);
    }
    if (!meta_res_->isNull(row, 3))
    {
        meta.name_ = std::string(meta_res_->getValue(row, 3), meta_res_->getLength(row, 3));
    }
    if (!meta_res_->isNull(row, 4))
    {
        std::string number{ meta_res_->getValue(row, 4), (size_t)meta_res_->getLength(row, 4) };
        meta.elem_type_idx_ = std::stoi(number);
    }
    if (!meta_res_->isNull(row, 5))
    {
        std::string number{ meta_res_->getValue(row, 5), (size_t)meta_res_->getLength(row, 5) };
        meta.len_ = std::stoi(number);
    }
    if (!meta_res_->isNull(row, 6))
    {
        meta.namespace_ = std::string(meta_res_->getValue(row, 6), meta_res_->getLength(row, 6));
    }

    printf("Load row %d: type: %c%c, idx: %d, oid: %d, name: \"%.*s\", elem_type_idx: %d, len: %d, namespace: \"%.*s\"\n",
           row,
           meta.type_,
           meta.category_,
           meta.idx_,
           meta.oid_,
           (int)meta.name_.size(), meta.name_.c_str(),
           meta.elem_type_idx_,
           meta.len_,
           (int)meta.namespace_.size(), meta.namespace_.c_str());
    return meta;
}

int pfs::CatalogueImpl::parseType(const pfs::CatalogueImpl::MetaRow & meta, int startRow)
{
    // The statistic row for types
    if (meta.oid_ == 0)
    {
        types_ = std::vector<std::shared_ptr<PgType>>(meta.len_, std::make_shared<PgType>());
        return 0;
    }

    PgType & typeInfo = *types_[meta.idx_ - 1];
    typeInfo.name_ = meta.name_;
    typeInfo.oid_ = meta.oid_;
    typeInfo.category_ = meta.category_;
    typeInfo.size_ = static_cast<unsigned short>(meta.len_);

    switch (meta.category_)
    {
        // Udt
        case 'C': {
            return parseUdt(meta, startRow);
        }
        // Array
        case 'A': {
            typeInfo.elemType_ = types_[meta.elem_type_idx_ - 1];
            return 0;
        }
        // primitive types
        default: {
            typeInfo.elemType_ = nullptr;
            return 0;
        }
    }
}

int pfs::CatalogueImpl::parseFunction(const pfs::CatalogueImpl::MetaRow & meta, int startRow)
{
    // The statistic row for functions
    if (meta.oid_ == 0)
    {
        funcs_ = std::vector<std::shared_ptr<PgFuncImpl>>(meta.len_, std::make_shared<PgFuncImpl>());
        return 0;
    }

    PgFuncImpl & func = *funcs_[meta.idx_ - 1];
    func.pronsp_ = meta.namespace_;
    func.proname_ = meta.name_;
    int numParams = meta.len_;
    int numCols = meta.elem_type_idx_; // We reuse type_idx column to store number of output parameters

    int nRows = 0;
    for (; numParams > 0 || numCols > 0; nRows++)
    {
        MetaRow fieldMeta = parseMetaRow(startRow + nRows);

        // Parameter mode (pg_proc.proargmodes)
        char paramMode = fieldMeta.category_;
        // Store to in/out param list
        // i: IN
        // v: VARIADIC
        // b: INOUT
        // o: OUT
        // t: TABLE
        if (paramMode == 'i' || paramMode == 'b' || paramMode == 'v')
        {
            if (numParams <= 0)
            {
                fprintf(stderr, "%s.%s IN parameter \"%s\" out of range\n",
                        func.pronsp_.c_str(), func.proname_.c_str(), fieldMeta.name_.c_str());
                return -1;
            }
            func.inParams_.push_back(PgField{ fieldMeta.name_, types_[fieldMeta.elem_type_idx_ - 1] });
            --numParams;
        }
        if (paramMode == 'o' || paramMode == 'b' || paramMode == 't')
        {
            if (numCols <= 0)
            {
                fprintf(stderr, "%s.%s OUT parameter \"%s\" out of range\n",
                        func.pronsp_.c_str(), func.proname_.c_str(), fieldMeta.name_.c_str());
                return -1;
            }
            func.outParams_.push_back(PgField{
                fieldMeta.name_, types_[fieldMeta.elem_type_idx_ - 1] });
            --numCols;
        }
    }

    return nRows;
}

int pfs::CatalogueImpl::parseUdt(const MetaRow & meta, int startRow)
{
    PgType & typeInfo = *types_[meta.idx_ - 1];

    for (int i = 0; i < typeInfo.size_; i++)
    {
        MetaRow fieldMeta = parseMetaRow(startRow + i);

        typeInfo.fields_.push_back(PgField{
            fieldMeta.name_,
            types_[fieldMeta.elem_type_idx_ - 1] });
    }
    return typeInfo.size_;
}
