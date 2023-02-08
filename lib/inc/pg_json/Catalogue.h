//
// Created by wanchen.he on 2023/1/6.
//
#pragma once

#include <memory>
#include <string>
#include <vector>

namespace pg_json
{
class PgFunc;
class PgResult;
class Catalogue
{
public:
    static std::shared_ptr<Catalogue> createFromMetaResult(std::shared_ptr<PgResult> result);

#if USE_LIBPQ || true
    static std::shared_ptr<Catalogue> createFromDbConnInfo(const std::string & connInfo = "");
#endif
    static const char * getMetaSql();

    virtual ~Catalogue();
    virtual void setVerbose(bool) = 0;
    virtual std::vector<std::shared_ptr<PgFunc>> findFunctions(const std::string & name) = 0;
    virtual std::vector<std::shared_ptr<PgFunc>> findFunctions(const std::string & name, const std::string & nsp) = 0;
};
} // namespace pg_json
