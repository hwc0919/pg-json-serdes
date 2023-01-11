# pg-json-serdes

Serialize between postgresql encodings and json. Support postgresql primitive types and user-defined composite types.

## Example

Load metadata from database, and parse json object into pg function parameters.

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

```cpp
using namespace pg_json;

// Load catalogue
auto catalogue = pg_json::Catalogue::createFromDbConnInfo(getTestDbUri());
auto func = catalogue->findFunctions("public", "pj_echo_complex")[0];

// Serialize parameters
nlohmann::json req{/* ... */};
GeneralParamSetter setter;
PgFunc::parseJsonToParams(req, *func, setter, PgTextWriter(), StringBuffer());

// Parse result
auto res = execSql(*func, setter);
assert(res->rows() == 1 && res->columns() == 2);
auto resJson = pg_json::PgFunc::parseResultToJson(*func, *res, PgTextReader(), RawCursor());
```

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
