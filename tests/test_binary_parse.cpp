//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <pg_json/Catalogue.h>
#include <pg_json/PgFunc.h>
#include <pg_json/PgReader.h>
#include <pg_json/PgWriter.h>
#include <pg_json/utils/GeneralParamSetter.h>
#include <pg_json/utils/PgResultWrapper.h>
#include <pg_json/utils/RawCursor.h>
#include <pg_json/utils/StringBuffer.h>

int main()
{
    auto catalogue = pg_json::Catalogue::createFromDbConnInfo(getTestDbUri());

    auto funcs = catalogue->findFunctions("public", "pj_echo_complex");
    std::cout << "Find " << funcs.size() << " functions by public.pj_echo_complex" << std::endl;
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
    nlohmann::json person{
        { "name", name },
        { "age", age },
        { "birthday", birthday },
        { "hobbies", hobbies }
    };
    nlohmann::json req{
        { "person", person },
        { "people", { person } }
    };
    std::cout << "Input json: " << req.dump() << std::endl;
    pg_json::GeneralParamSetter setter;
    auto writer = pg_json::PgWriter::newBinaryWriter();
    auto buffer = pg_json::StringBuffer();
    pg_json::PgFunc::parseJsonToParams(req, *func, setter, *writer, buffer, 1);
    std::cout << "Pg params:" << std::endl;
    printParams(*func, setter);

    auto res = execSql(*func, setter, 1);
    assert(res->rows() == 1 && res->columns() == 2);
    std::cout << "Pg result:" << std::endl;
    printResults(*func, *res);

    auto reader = pg_json::PgReader::newBinaryReader();
    auto cursor = pg_json::RawCursor();
    auto resJson = pg_json::PgFunc::parseResultToJson(*func, *res, *reader, cursor);
    std::cout << "Result json: " << resJson.dump() << std::endl;
}
