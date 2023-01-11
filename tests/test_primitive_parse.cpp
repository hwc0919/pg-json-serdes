//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <pg_json/Catalogue.h>
#include <pg_json/PgFunc.h>
#include <pg_json/utils/GeneralParamSetter.h>
#include <pg_json/utils/PgTextReader.h>
#include <pg_json/utils/PgTextWriter.h>
#include <pg_json/utils/RawCursor.h>
#include <pg_json/utils/StringBuffer.h>

int main()
{
    auto catalogue = pg_json::Catalogue::createFromDbConnInfo(getTestDbUri());

    auto funcs = catalogue->findFunctions("public", "pj_test_primitives");
    std::cout << "Find " << funcs.size() << " functions by public.pj_test_primitives" << std::endl;
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
    pg_json::GeneralParamSetter setter;
    pg_json::PgFunc::parseJsonToParams(req, *func, setter, pg_json::PgTextWriter(), pg_json::StringBuffer());

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

    auto resJson = pg_json::PgFunc::parseResultToJson(*func, *res, pg_json::PgTextReader(), pg_json::RawCursor());
    std::cout << "Result json: " << resJson.dump() << std::endl;
}
