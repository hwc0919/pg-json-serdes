//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <pfs/Catalogue.h>
#include <pfs/PgFunc.h>
#include <pfs/utils/GeneralParamSetter.h>
#include <pfs/utils/PgTextReader.h>
#include <pfs/utils/PgTextWriter.h>
#include <pfs/utils/RawCursor.h>
#include <pfs/utils/StringBuffer.h>

int main()
{
    auto catalogue = pfs::Catalogue::createFromDbConnInfo(getTestDbUri());

    auto funcs = catalogue->findFunctions("public", "pfs_test_primitives");
    std::cout << "Find " << funcs.size() << " functions by public.pfs_test_primitives" << std::endl;
    if (funcs.empty())
    {
        return 1;
    }

    auto & func = funcs[0];
    printPgFunc(*func);

    std::string name = R"(Nitro'"\melon)";
    int age = 27;
    std::string birthday = "1949-10-01 11:11:11";
    nlohmann::json data{ { "phone", 123456 }, { "email", "nitromelon@foxmail.com" } };
    nlohmann::json req{
        { "name", name },
        { "age", age },
        { "birthday", birthday },
        { "data", data }
    };
    pfs::GeneralParamSetter setter;
    pfs::PgFunc::parseJsonToParams(req, *func, setter, pfs::PgTextWriter(), pfs::StringBuffer());

    std::cout << "Input json: " << req.dump() << std::endl;
    std::cout << "Pg params:" << std::endl;
    printParams(*func, setter);

    auto & params = setter.getParamValues();
    auto & paramLens = setter.getParamLens();
    assert(std::string(params[0], paramLens[0]) == name);
    assert(std::string(params[1], paramLens[1]) == std::to_string(age));
    assert(std::string(params[2], paramLens[2]) == birthday);
    assert(std::string(params[3], paramLens[3]) == data.dump());

    auto res = execSql(*func, setter);
    std::cout << "Pg result:" << std::endl;
    printResults(*func, *res);

    pfs::PgTextReader reader;
    pfs::RawCursor cursor;
    auto resJson = pfs::PgFunc::parseResultToJson(*func, *res, reader, cursor);
    std::cout << "Result json: " << resJson.dump() << std::endl;
}
