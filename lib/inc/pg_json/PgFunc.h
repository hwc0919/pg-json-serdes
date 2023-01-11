#pragma once
#include <nlohmann/json.hpp>
#include <pg_json/Cursor.h>
#include <pg_json/IBuffer.h>
#include <pg_json/IParamSetter.h>
#include <pg_json/IPgReader.h>
#include <pg_json/IPgWriter.h>
#include <pg_json/IResult.h>

namespace pg_json
{

class PgFunc
{
public:
    using Oid = unsigned int;
    virtual ~PgFunc() = default;

    static void parseJsonToParams(const nlohmann::json & obj, const PgFunc & func, IParamSetter & setter, IPgWriter & writer, IBuffer & buffer);
    // overload to take right values
    static void parseJsonToParams(const nlohmann::json & obj, const PgFunc & func, IParamSetter & setter, IPgWriter && writer, IBuffer && buffer)
    {
        parseJsonToParams(obj, func, setter, writer, buffer);
    }
    static nlohmann::json parseResultToJson(const PgFunc & func, const IResult & result, IPgReader & reader, Cursor & cursor);

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
