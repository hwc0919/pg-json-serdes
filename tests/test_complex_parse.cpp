//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <pfs/Catalogue.h>
#include <pfs/PgFunc.h>
#include <pfs/utils/GeneralParamSetter.h>
#include <pfs/utils/PgResultWrapper.h>
#include <pfs/utils/PgTextWriter.h>
#include <pfs/utils/StringBuffer.h>

int main()
{
    auto catalogue = pfs::Catalogue::createFromDbConnInfo(getTestDbUri());

    auto funcs = catalogue->findFunctions("public", "pfs_echo_complex");
    std::cout << "Find " << funcs.size() << " functions by public.pfs_echo_complex" << std::endl;
    if (funcs.empty())
    {
        return 1;
    }

    auto & func = funcs[0];
    printPgFunc(*func);

    pfs::GeneralParamSetter setter;

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
    pfs::PgFunc::parseJsonToParams(req, *func, setter, pfs::PgTextWriter(), pfs::StringBuffer());
    std::cout << "Pg params:" << std::endl;
    printParams(*func, setter);

    auto res = execSql(*func, setter);
    assert(res->rows() == 1 && res->columns() == 2);
    std::cout << "Pg result:" << std::endl;
    printResults(*func, *res);
}
