#pragma once
#include <nlohmann/json.hpp>
#include <pg_json/Cursor.h>
#include <pg_json/Buffer.h>
#include <pg_json/PgParamSetter.h>
#include <pg_json/PgReader.h>
#include <pg_json/PgWriter.h>
#include <pg_json/PgResult.h>

namespace pg_json
{

class PgFunc
{
public:
    using Oid = unsigned int;
    virtual ~PgFunc() = default;

    static void parseJsonToParams(const nlohmann::json & obj, const PgFunc & func, PgParamSetter & setter, PgWriter & writer, Buffer & buffer, int format = 0);
    static nlohmann::json parseResultToJson(const PgFunc & func, const PgResult & result, PgReader & reader, Cursor & cursor);

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
