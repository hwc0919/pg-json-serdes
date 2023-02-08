with base(ns, cat, object_oid, att_ordinal, att_name, type_oid, numFields, pronsp) as
(
    -- Functions (excluding parameters)
    select
        'F'             as ns,
        ' '             as cat,
        p.oid           as object_oid,
        0               as att_ordinal,
        p.proname       as att_name,
        -- when att_ordinal=0: null=proc, 0=type, N=array(where N is the element OID)
        null            as type_oid,
        0               as numFields,
        n.nspname       as pronsp
    from
        pg_proc p
    left outer join pg_namespace n
    on
        p.pronamespace = n.oid
    left outer join pg_language l
    on
        p.prolang = l.oid
    where
        l.lanname = 'plpgsql' and
        n.nspname not like 'pg_%' and
        n.nspname not in ('information_schema', 'storm_catalog')

    union all

    -- Function parameters/columns
    select
        'P'                                 as ns,
        coalesce(
            p.proargmodes[x.arg_ordinal],   -- proargmodes is null, means all params are IN params
            'i'
        )                                   as cat,
        p.oid                               as object_oid,
        x.arg_ordinal                       as att_ordinal,
        p.proargnames[x.arg_ordinal]        as att_name,
        x.arg_type_oid                      as type_oid,
        null                                as numFields,
        null                                as pronsp
    from
        pg_proc p
    left outer join pg_namespace n
    on
        p.pronamespace = n.oid
    left outer join pg_language l
    on
        p.prolang = l.oid,
    lateral unnest(
        coalesce(p.proallargtypes, p.proargtypes::oid[])
    ) with ordinality as x(arg_type_oid, arg_ordinal)
    where
        l.lanname = 'plpgsql' and
        n.nspname not like 'pg_%' and
        n.nspname not in ('information_schema', 'storm_catalog')

    union all

    -- Types (excluding attributes)
    select
        'T'                                 as ns,
        t.typcategory                       as cat,
        t.oid                               as object_oid,
        0                                   as att_ordinal,
        t.typname                           as att_name,
        t.typelem                           as type_oid,
        case t.typcategory
            when 'C' then 0
            -- Some Composite type has no member, which leaves len as -1 and no aggregation result to overwrite it.
            else t.typlen
        end                                 as numFields,
        null                                as pronsp
    from
        pg_type t
    inner join pg_namespace n
    on
        t.typnamespace = n.oid
    left outer join pg_class c
    on
        t.typrelid = c.oid
    -- link array and element type
    left join pg_type pt2
    on
        t.typelem = pt2.oid
    -- pg_class for array type
    left join pg_class pc2
    on
        pt2.typrelid = pc2.oid
    where
        n.nspname not like all(ARRAY['pg_temp%', 'pg_toast%']) and
        n.nspname not in ('information_schema', 'storm_catalog') and
        -- pg_type.typtype:
        -- b = base type, c = composite type (e.g., a table's row type), d = domain, e = enum type, p = pseudo-type, r = range type, m = multirange type
        -- pg_class.relkind:
        -- r = ordinary table, i = index, S = sequence, t = TOAST table, v = view,
        -- m = materialized view, c = composite type, f = foreign table, p = partitioned table
        (
            t.typtype = 'b' and t.typcategory != 'A'                        -- base type, non-array
         or t.typtype = 'b' and t.typcategory = 'A' and pt2.typtype = 'b'   -- base type array
         or t.typtype = 'c' and t.typcategory != 'A' and c.relkind = 'c'    -- composite type, non-array
         or t.typtype = 'b' and t.typcategory = 'A' and pt2.typtype = 'c' and pc2.relkind = 'c' -- composite type array
        )

    union all

    -- User-defined type attributes
    select
        'A'                                 as ns,
        case
            when a.attnotnull then 'm'
            else 'o'
        end                                 as cat,
        c.reltype                           as object_oid,
        a.attnum                            as att_ordinal,
        a.attname                           as att_name,
        a.atttypid                          as type_oid,
        a.attlen                            as numFields,
        null                                as pronsp
    from
        pg_attribute a
    inner join pg_class c
    on
        a.attrelid = c.oid
    inner join pg_namespace n
    on
        c.relnamespace = n.oid
    where
        c.relkind = 'c' and
        a.attnum > 0 and
        not a.attisdropped
        and n.nspname not like 'pg_%'
        and n.nspname not in ('information_schema', 'storm_catalog')
),
oids(ns, oid, idx) as
(
    select
        b.ns,
        b.object_oid,
        row_number() over (partition by b.ns order by b.object_oid)
    from
        base b
    where
        b.ns in ('F', 'T')
),
attrs(oid, in_param_count, out_param_count) as
(
    select
        b.object_oid,
        count(*) filter (where
            b.ns = 'A' or
            b.cat in ('i', 'b', 'v')
        ),
        count(*) filter (where
            b.ns = 'P' and
            b.cat in ('o', 'b', 't')
        )
    from
        base b
    where
        b.ns in ('A', 'P')
    group by
        b.object_oid
)
select
    m.ns || m.cat           as type,
    m.object_idx::int       as object_idx,
    m.object_oid            as object_oid,
    m.att_name              as att_name,
    m.type_idx::int         as type_idx,
    m.numFields::int              as numFields,
    m.pronsp                as pronsp
from
(
    -- Two statistic rows
    select
        b.ns,
        ' ',                    -- cat
        null,                   -- object_idx
        0,                      -- object_oid
        0,                      -- attr_ordinal
        null,                   -- att_name
        null,                   -- type_oid
        null,                   -- type_idx
        count(*),               -- len
        null                    -- pronsp
    from
        base b
    where
        b.ns in ('T', 'F')
    group by
        b.ns

    union all

    (
    -- Meta data rows
    select
        b.ns,
        b.cat,
        o0.idx,
        b.object_oid,
        b.att_ordinal,
        b.att_name,
        b.type_oid,
        coalesce(o1.idx, a.out_param_count, 0), -- type_idx or out_param_count
        coalesce(a.in_param_count, b.numFields),      -- len      or input_param_count
        b.pronsp
    from
        base b
    left outer join oids o0
    on
        b.object_oid = o0.oid and
        b.ns = o0.ns
    left outer join oids o1
    on
        b.type_oid = o1.oid and
        o1.ns = 'T'
    left outer join attrs a
    on
        b.object_oid = a.oid and
        b.att_ordinal = 0
    order by
        b.object_oid, b.att_ordinal
    )

) as m(ns, cat, object_idx, object_oid, att_ordinal, att_name, type_oid, type_idx, numFields, pronsp);
