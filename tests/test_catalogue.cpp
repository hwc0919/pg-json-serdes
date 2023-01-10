//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <pfs/Catalogue.h>

int main()
{
    auto catalogue = pfs::Catalogue::createFromDbConnInfo(getTestDbUri());
}
