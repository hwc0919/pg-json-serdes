#include <iostream>

#include <pfs/Catalogue.h>

int main()
{
    auto catalogue = pfs::Catalogue::createFromDbConnInfo("postgresql://test@192.168.66.66:21001/test");
}
