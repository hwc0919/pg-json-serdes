#pragma once

#include <cstddef>
#include <pg_json/PgField.h>
#include <string>

namespace pg_json
{
class PgFunc
{
public:
    using Oid = unsigned int;

    virtual ~PgFunc() = default;
    virtual const std::string & namespace_() const = 0;
    virtual const std::string & name() const = 0;
    virtual const std::string & statement() const = 0;
    virtual const Oid * oids() const = 0;
    virtual size_t in_size() const = 0;
    virtual size_t out_size() const = 0;
    virtual const std::string & in_name(size_t i) const = 0;
    virtual const std::string & out_name(size_t i) const = 0;
    virtual const PgField & in_field(size_t i) const = 0;
    virtual const PgField & out_field(size_t i) const = 0;
    virtual const std::string & in_type_name(size_t i) const = 0;
    virtual const std::string & out_type_name(size_t i) const = 0;
};
} // namespace pg_json
