//
// Created by wanchen.he on 2023/1/8.
//
#include "common.h"
#include <pg_json/Catalogue.h>

int main()
{
    auto catalogue = pg_json::Catalogue::createFromDbConnInfo(getTestDbUri());
}
