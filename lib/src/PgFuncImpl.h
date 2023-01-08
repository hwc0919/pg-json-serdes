//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#include "PgField.h"
#include <pfs/PgFunc.h>
#include <string>
#include <vector>

namespace pfs
{
class PgFuncImpl : public PgFunc
{
public:
    using Oid = unsigned int;

    std::string namespace_;
    std::string name_;

    std::vector<PgField> in_params_;
    std::vector<PgField> out_params_;

    // Derived data
    std::string stmt_;      // Statement to send to DB
    std::vector<Oid> oids_; // Parameter OID array
};

} // namespace pfs
