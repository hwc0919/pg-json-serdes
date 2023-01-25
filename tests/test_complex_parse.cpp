//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <iostream>
#include <pg_json/Catalogue.h>
#include <pg_json/Converter.h>
#include <pg_json/json.h>
#include <pg_json/utils/GeneralParamSetter.h>
#include <pg_json/utils/PgResultWrapper.h>

using namespace pg_json;

json_t defaultValueNullHandler(const PgType & pgType, bool /*explicitNull*/)
{
    std::cout << "defaultValueNullHandler pgType: " << pgType.name_ << ", cat: " << pgType.category_ << std::endl;
    if (pgType.isComposite())
    {
        return json_t::object();
    }
    if (pgType.isArray())
    {
        return json_t ::array();
    }
    if (pgType.isNumber())
    {
        return 0;
    }
    return std::string("");
}

int main()
{
    auto catalogue = Catalogue::createFromDbConnInfo(getTestDbUri());

    auto funcs = catalogue->findFunctions("pj_echo_complex");
    std::cout << "Find " << funcs.size() << " functions by pj_echo_complex" << std::endl;
    if (funcs.empty())
    {
        return 1;
    }

    auto & func = funcs[0];
    printPgFunc(*func);

    std::string name = "Nitromelon";
    int age = 27;
    std::string birthday = "1949-10-01 11:11:11";
    std::vector<std::string> hobbies{ "programing", "video games",
                                      R"(find \/"'(){} bug)" };
    json_t person{
        { "name", name },
        { "age", age },
        { "birthday", birthday },
        { "hobbies", hobbies }
    };
    json_t reqJson{
        { "person", person },
        { "people", { person } }
    };
    std::cout << "Input json: " << reqJson.dump() << std::endl;

    GeneralParamSetter setter;
    Converter converter(PgFormat::kText);
    converter.setNullHandler(defaultValueNullHandler);
    converter.parseJsonToParams(*func, reqJson, setter);
    std::cout << "Pg params:" << std::endl;
    printParams(*func, setter);

    auto res = execSql(*func, setter);
    assert(res->rows() == 1 && res->columns() == 2);
    std::cout << "Pg result:" << std::endl;
    printResults(*func, *res);

    auto resJson = converter.parseResultToJson(*func, *res);
    std::cout << "Result json: " << resJson.dump() << std::endl;
}
