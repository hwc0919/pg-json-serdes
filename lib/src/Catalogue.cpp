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

#if USE_LIBPQ || true
#include "MetaSql.h"
#include "PgResultWrapper.h"
#include <libpq-fe.h>

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
