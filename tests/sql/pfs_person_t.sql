create type pfs_person_t as
(
    name        text,
    age         int,
    score       float4,
    birthday    timestamp,
    hobbies     text[]
);
