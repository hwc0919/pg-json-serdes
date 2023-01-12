#pragma once
#include <cstdlib>
#include <iostream>
#include <libpq-fe.h>
#include <pg_json/PgResult.h>
#include <pg_json/PgFunc.h>
#include <pg_json/utils/GeneralParamSetter.h>
#include <pg_json/utils/PgResultWrapper.h>
#include <string>

inline std::string getTestDbUri()
{
    const char * dbUrl = std::getenv("PFS_TEST_DB_URI");
    return dbUrl;
}

inline void printPgFunc(const pg_json::PgFunc & func)
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

inline void printParams(const pg_json::PgFunc & func, const pg_json::GeneralParamSetter & setter)
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

inline void printResults(const pg_json::PgFunc & func, const pg_json::PgResult & result)
{
    for (size_t i = 0; i != func.out_size(); ++i)
    {
        std::cout << func.out_name(i) << ": " << std::string(result.getValue(0, i), result.getLength(0, i)) << std::endl;
    }
}

inline std::shared_ptr<pg_json::PgResultWrapper> execSql(const pg_json::PgFunc & func, const pg_json::GeneralParamSetter & setter, int resultFormat = 0)
{
    std::shared_ptr<PGconn> connPtr(PQconnectdb(getTestDbUri().c_str()), [](PGconn * conn) {
        if (conn) {
            PQfinish(conn);
        }
    });
    if (connPtr == nullptr || PQstatus(connPtr.get()) != CONNECTION_OK)
    {
        throw std::runtime_error(std::string("Failed to connect to database. ") + PQerrorMessage(connPtr.get()));
    }
    PGresult * r = PQexecParams(
        connPtr.get(),
        func.statement().c_str(),
        (int)setter.size(),
        func.oids(),
        setter.getParamValues().data(),
        setter.getParamLens().data(),
        setter.getParamFormats().data(),
        resultFormat);
    if (r == nullptr)
    {
        throw std::runtime_error(std::string("sql execute failed. ") + PQerrorMessage(connPtr.get()));
    }
    auto res = pg_json::PgResultWrapper::wrap(r);
    if (PQresultStatus(r) != PGRES_TUPLES_OK)
    {
        throw std::runtime_error(std::string("sql execute failed. ") + PQerrorMessage(connPtr.get()));
    }
    return res;
}
