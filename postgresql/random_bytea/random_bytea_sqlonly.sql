--
-- See http://dba.stackexchange.com/a/22571/7788
--
--
-- This is a simple standalone SQL-only function to generate
-- random bytea values. It's fine for generating a few hundred kb.
--

CREATE OR REPLACE FUNCTION random_bytea(bytea_length integer)
RETURNS bytea AS $body$
    SELECT decode(string_agg(lpad(to_hex(width_bucket(random(), 0, 1, 256)-1),2,'0') ,''), 'hex')
    FROM generate_series(1, $1);
$body$
LANGUAGE 'sql'
VOLATILE
SET search_path = 'pg_catalog';

COMMENT ON random_bytea(integer) IS 'Generate n random bytes of garbage, returned as bytea. Slow for large values of n.';
