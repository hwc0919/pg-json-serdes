#pragma once
#include <cstdlib>
#include <iostream>
#include <pfs/PgFunc.h>
#include <pfs/utils/GeneralParamSetter.h>
#include <string>

inline std::string getTestDbUri()
{
    const char * dbUrl = std::getenv("PFS_TEST_DB_URI");
    return dbUrl;
}

inline void printPgFunc(const pfs::PgFunc & func)
{
    std::cout << func.namespace_() << "." << func.name() << "(";
    for (size_t i = 0; i != func.in_size(); ++i)
    {
        std::cout << (i == 0 ? "\n    " : ",\n    ") << func.in_name(i) << " " << func.in_type_name(i);
    }
    std::cout << "\n) returns (";
    for (size_t i = 0; i != func.out_size(); ++i)
    {
        std::cout << (i == 0 ? "\n    " : ",\n    ") << func.out_name(i) << " " << func.out_type_name(i);
    }
    std::cout << "\n)" << std::endl;
}

inline std::string binaryStringToHex(const unsigned char * ptr, size_t length)
{
    std::string hexStr;
    for (size_t i = 0; i < length; ++i)
    {
        int value = (ptr[i] & 0xf0) >> 4;
        if (value < 10)
        {
            hexStr.append(1, char(value + 48));
        }
        else
        {
            hexStr.append(1, char(value + 55));
        }

        value = (ptr[i] & 0x0f);
        if (value < 10)
        {
            hexStr.append(1, char(value + 48));
        }
        else
        {
            hexStr.append(1, char(value + 55));
        }
    }
    return hexStr;
}

inline void printParams(const pfs::PgFunc & func, const pfs::GeneralParamSetter & setter)
{
    auto & params = setter.getParamValues();
    auto & paramLens = setter.getParamLens();
    auto & paramFormats = setter.getParamFormats();

    for (size_t i = 0; i < params.size(); ++i)
    {
        if (paramFormats[i] == 0)
        {
            std::cout << func.in_name(i) << ": " << std::string(params[i], paramLens[i]) << std::endl;
        }
        else
        {
            std::cout << func.in_name(i) << ": (binary)" << binaryStringToHex((const unsigned char *)params[i], paramLens[i]) << std::endl;
        }
    }
}
