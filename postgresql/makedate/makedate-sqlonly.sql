-- SQL-only version of makedate(...)
-- (much, much slower)

CREATE OR REPLACE FUNCTION makedate("year" integer, "month" integer, "day" integer)
RETURNS date
AS $$
SELECT CAST(
    DATE '0001-01-01'
    + ("year"-1) * INTERVAL '1' YEAR
    + ("month"-1) * INTERVAL '1' MONTH
    + ("day"-1) * INTERVAL '1' DAY
AS date);
$$ LANGUAGE sql IMMUTABLE STRICT;
