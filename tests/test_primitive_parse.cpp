//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <iostream>
#include <pg_json/Catalogue.h>
#include <pg_json/Converter.h>
#include <pg_json/json.h>
#include <pg_json/utils/GeneralParamSetter.h>

using namespace pg_json;

int main()
{
    auto catalogue = Catalogue::createFromDbConnInfo(getTestDbUri());

    auto funcs = catalogue->findFunctions("pj_test_primitives");
    std::cout << "Find " << funcs.size() << " functions by pj_test_primitives" << std::endl;
    if (funcs.empty())
    {
        return 1;
    }

    auto & func = funcs[0];
    printPgFunc(*func);

    std::string name = R"(Nitro'"\melon)";
    int age = 27;
    std::string birthday = "1949-10-01 11:11:11";
    json_t data{ { "phone", 123456 }, { "email", "nitromelon@foxmail.com" } };
    json_t reqJson{
        { "name", name },
        { "age", age },
        { "birthday", birthday },
        { "data", data }
    };
    GeneralParamSetter setter;
    auto converter = Converter::newConverter(PgFormat::kText);
    converter->parseJsonToParams(*func, reqJson, setter);

    std::cout << "Input json: " << reqJson.dump() << std::endl;
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

    auto resJson = converter->parseResultToJson(*func, *res);
    std::cout << "Result json: " << resJson.dump() << std::endl;
}
