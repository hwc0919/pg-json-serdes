//
// Created by wanchen.he on 2023/1/6.
//
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace pfs
{
class PgFunc;
class IResult;
class Catalogue
{
public:
    static std::shared_ptr<Catalogue> createFromMetaResult(std::shared_ptr<IResult> result);

#if 1
    static std::shared_ptr<Catalogue> createFromDbConnInfo(const std::string & connInfo);
#endif

    virtual ~Catalogue();
    virtual std::vector<std::shared_ptr<PgFunc>> findFunction(const std::string & name) = 0;
};
} // namespace pfs
