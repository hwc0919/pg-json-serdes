//
// Created by wanchen.he on 2023/1/8.
//
#pragma once

#if USE_LIBPQ || true
#include <libpq-fe.h>
#include <memory>
#include <pg_json/IResult.h>

namespace pg_json
{
/**
 * Wrap raw `PGresult` pointer.
 * Take the ownership of allocated memery, and provide eazy data access.
 */
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
    size_t rows() const noexcept override
    {
        return PQntuples(result_.get());
    }
    size_t columns() const noexcept override
    {
        return PQnfields(result_.get());
    }
    bool isNull(size_t row, size_t col) const noexcept override
    {
        return PQgetisnull(result_.get(), (int)row, (int)col);
    }
    const char * getValue(size_t row, size_t col) const noexcept override
    {
        return PQgetvalue(result_.get(), (int)row, (int)col);
    }
    size_t getLength(size_t row, size_t col) const noexcept override
    {
        return PQgetlength(result_.get(), (int)row, (int)col);
    }

private:
    std::shared_ptr<PGresult> result_;
};

} // namespace pg_json

#endif
