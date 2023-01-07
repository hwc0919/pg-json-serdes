//
// Created by wanchen.he on 2023/1/6.
//

#include "CatalogueImpl.h"
#include <pfs/Catalogue.h>
#include <pfs/IResult.h>
#include <stdexcept>

using namespace pfs;

Catalogue::~Catalogue() = default;

std::shared_ptr<Catalogue> Catalogue::createFromMetaResult(std::shared_ptr<IResult> result)
{
    std::shared_ptr<CatalogueImpl> catalogue = std::make_shared<CatalogueImpl>(std::move(result));
    catalogue->parseMeta();
    return catalogue;
}

#if 1
#include "MetaSql.h"
#include <libpq-fe.h>

class PgResultWrapper : public IResult
{
public:
    static std::shared_ptr<PgResultWrapper> wrap(PGresult * result)
    {
        std::shared_ptr<PGresult> resultPtr(result, [](PGresult * r) {
            PQclear(r);
        });
        return std::make_shared<PgResultWrapper>(resultPtr);
    }

    explicit PgResultWrapper(std::shared_ptr<PGresult> result)
        : result_(std::move(result))
    {
    }
    int rows() const override
    {
        return PQntuples(result_.get());
    }
    int columns() const override
    {
        return PQnfields(result_.get());
    }
    bool isNull(int row, int col) const override
    {
        return PQgetisnull(result_.get(), row, col);
    }
    const char * getValue(int row, int col) const override
    {
        return PQgetvalue(result_.get(), row, col);
    }
    int getLength(int row, int col) const override
    {
        return PQgetlength(result_.get(), row, col);
    }

private:
    std::shared_ptr<PGresult> result_;
};

std::shared_ptr<Catalogue> Catalogue::createFromDbConnInfo(const std::string & connInfo)
{
    std::shared_ptr<PGconn> connPtr(PQconnectdb(connInfo.c_str()), [](PGconn * conn) {
        if (conn) {
            PQfinish(conn);
        }
    });

    if (connPtr == nullptr || PQstatus(connPtr.get()) != CONNECTION_OK)
    {
        throw std::runtime_error(std::string("PgCatalog failed to connect to database. ") + PQerrorMessage(connPtr.get()));
    }

    PGresult * r = PQexec(connPtr.get(), getMetaSql());
    if (r == nullptr)
    {
        throw std::runtime_error(std::string("PgCatalog failed to execute meta sql. ") + PQerrorMessage(connPtr.get()));
    }
    auto res = PgResultWrapper::wrap(r);

    if (PQresultStatus(r) != PGRES_TUPLES_OK)
    {
        throw std::runtime_error(std::string("PgCatalog sql execute failed. ") + PQerrorMessage(connPtr.get()));
    }

    return createFromMetaResult(std::move(res));
}

#endif
