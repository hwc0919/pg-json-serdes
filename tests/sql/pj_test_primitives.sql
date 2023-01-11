create function pj_test_primitives
(
    name        text,
    age         int,
    birthday    timestamp,
    data        jsonb,
out user_info   jsonb
)
language plpgsql
as $$
begin
    raise notice 'pj_test_primitives, name = %, age = %, birthday = %, data = %',
        name, age, birthday, data;

    user_info = jsonb_build_object(
        'name', name,
        'age', age,
        'birthday', birthday,
        'data', data
    );
end $$;
