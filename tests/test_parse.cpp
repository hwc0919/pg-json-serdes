//
// Created by wanchen.he on 2023/1/8.
//
#include <iostream>
#include <nlohmann/json.hpp>
#include <pfs/Catalogue.h>
#include <pfs/PgFunc.h>
#include <pfs/utils/DefaultParamSetter.h>

int main()
{
    auto catalogue = pfs::Catalogue::createFromDbConnInfo("postgresql://test@192.168.66.66:21001/test");

    auto funcs = catalogue->findFunctions("public", "pfs_test_primitives");
    std::cout << "Find " << funcs.size() << " functions by public.pfs_test_primitives" << std::endl;
    if (funcs.empty())
    {
        return 1;
    }

    auto & func = funcs[0];
    std::cout << "public.pfs_test_primitives(";
    for (size_t i = 0; i != func->in_size(); ++i)
    {
        std::cout << (i == 0 ? "\n    " : ",\n    ") << func->in_name(i) << " " << func->in_type_name(i);
    }
    std::cout << "\n) returns (";
    for (size_t i = 0; i != func->out_size(); ++i)
    {
        std::cout << (i == 0 ? "\n    " : ",\n    ") << func->out_name(i) << " " << func->out_type_name(i);
    }
    std::cout << "\n)" << std::endl;

    pfs::DefaultParamSetter setter;
    nlohmann::json req{};
    func->setParamsFromJson(req, setter);
}
