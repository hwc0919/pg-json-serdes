# pg-json-serdes

Serialize between postgresql encodings and json. Support postgresql primitive types and user-defined composite types.

## Features
- Support postgresql composite types
- Support postgresql binary encoding and text encoding.
- Write pg function parameters from json, result can be used directly in `PQexecParams`
- Read pg function result (`PGresult`) to json.
- No dependencies, no schema files needed. Database metadata is fetched at runtime.
- All component can be customized by inheriting from virtual base class.

## Example

Load metadata from database, and parse json object into pg function parameters.

Database elements:
```sql
create type pj_person_t as (
    name        text,
    age         int,
    score       float4,
    birthday    timestamp,
    hobbies     text[]
);

create or replace function pj_echo_complex(
inout person pj_person_t,
inout people pj_person_t[]
)
language plpgsql
as $$
begin
    -- do nothing
end $$;
```

Cpp code:
```cpp
using namespace pg_json;

// Load catalogue
auto catalogue = pg_json::Catalogue::createFromDbConnInfo(getTestDbUri());
auto func = catalogue->findFunctions("public", "pj_echo_complex")[0];

// Serialize parameters
nlohmann::json req{/* ... */};
GeneralParamSetter setter;
auto writer = PgWriter::newTextWriter();
auto buffer = StringBuffer();
PgFunc::parseJsonToParams(req, *func, setter, *writer, buffer);

// Parse result
auto res = execSql(*func, setter);
auto reader = PgReader::newTextReader();
auto cursor = RawCursor();
auto resJson = PgFunc::parseResultToJson(*func, *res, *reader, cursor);
```

Output:
```text
Input json: 
{
    "people":[
        {
            "age":27,
            "birthday":"1949-10-01 11:11:11",
            "hobbies":[
                "programing",
                "video games",
                "find \\/\"'(){} bug"
            ],
            "name":"Nitromelon"
        }
    ],
    "person":{
        "age":27,
        "birthday":"1949-10-01 11:11:11",
        "hobbies":[
            "programing",
            "video games",
            "find \\/\"'(){} bug"
        ],
        "name":"Nitromelon"
    }
}
Pg params:
person: (Nitromelon,27,,"1949-10-01 11:11:11","{programing,""video games"",""find \\\\/\\""'(){} bug""}")
people: {"(Nitromelon,27,,\"1949-10-01 11:11:11\",\"{programing,\"\"video games\"\",\"\"find \\\\\\\\/\\\\\"\"'(){} bug\"\"}\")"}

Pg result:
person: (Nitromelon,27,,"1949-10-01 11:11:11","{programing,""video games"",""find \\\\/\\""'(){} bug""}")
people: {"(Nitromelon,27,,\"1949-10-01 11:11:11\",\"{programing,\"\"video games\"\",\"\"find \\\\\\\\/\\\\\"\"'(){} bug\"\"}\")"}
Result json:
{
    "people":[
        {
            "age":27,
            "birthday":"1949-10-01 11:11:11",
            "hobbies":[
                "programing",
                "video games",
                "find \\/\"'(){} bug"
            ],
            "name":"Nitromelon",
            "score":0
        }
    ],
    "person":{
        "age":27,
        "birthday":"1949-10-01 11:11:11",
        "hobbies":[
            "programing",
            "video games",
            "find \\/\"'(){} bug"
        ],
        "name":"Nitromelon",
        "score":0
    }
}

```
