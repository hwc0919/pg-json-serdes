//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#if USE_LIBPQ || true
#include <libpq-fe.h>
#include <memory>
#include <pfs/IResult.h>

namespace pfs
{

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
    int rows() const noexcept override
    {
        return PQntuples(result_.get());
    }
    int columns() const noexcept override
    {
        return PQnfields(result_.get());
    }
    bool isNull(int row, int col) const noexcept override
    {
        return PQgetisnull(result_.get(), row, col);
    }
    const char * getValue(int row, int col) const noexcept override
    {
        return PQgetvalue(result_.get(), row, col);
    }
    int getLength(int row, int col) const noexcept override
    {
        return PQgetlength(result_.get(), row, col);
    }

private:
    std::shared_ptr<PGresult> result_;
};

} // namespace pfs

#endif
