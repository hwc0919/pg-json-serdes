#include <iostream>

#include <pfs/Catalogue.h>
#include <pfs/PgFunc.h>

int main()
{
    auto catalogue = pfs::Catalogue::createFromDbConnInfo("postgresql://test@192.168.66.66:21001/test");

    auto funcs = catalogue->findFunctions("public", "test1");
    std::cout << "Find " << funcs.size() << " functions by public.test1";

    for (auto & func : funcs)
    {
        std::cout << "Find function public.test1(";
        for (int i = 0; i != func->in_size(); ++i)
        {
            std::cout << (i == 0 ? "" : ",") << func->in_name(i) << " " << func->in_type_name(i);
        }
        std::cout << ") -> (";
        for (int i = 0; i != func->out_size(); ++i)
        {
            std::cout << (i == 0 ? "" : ",") << func->out_name(i) << " " << func->out_type_name(i);
        }
        std::cout << ")" << std::endl;
    }
}
