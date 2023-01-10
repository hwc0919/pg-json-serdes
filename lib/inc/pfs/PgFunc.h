#pragma once
#include <nlohmann/json.hpp>
#include <pfs/IParamSetter.h>
#include <pfs/IPgWriter.h>

namespace pfs
{

class PgFunc
{
public:
    virtual ~PgFunc() = default;

    virtual const std::string & namespace_() const = 0;
    virtual const std::string & name() const = 0;
    virtual const std::string & statement() const = 0;
    virtual size_t in_size() const = 0;
    virtual size_t out_size() const = 0;
    virtual const std::string & in_name(size_t i) const = 0;
    virtual const std::string & out_name(size_t i) const = 0;
    virtual const std::string & in_type_name(size_t i) const = 0;
    virtual const std::string & out_type_name(size_t i) const = 0;

    virtual void setParamsFromJson(const nlohmann::json & paramObj, IParamSetter & setter, IPgWriter & writer) = 0;
};

} // namespace pfs
