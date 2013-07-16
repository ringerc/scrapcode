CREATE OR REPLACE FUNCTION makedate("year" integer, "month" integer, "day" integer) RETURNS date
	AS 'makedate','makedate'
	LANGUAGE c STRICT;

COMMENT ON FUNCTION makedate("year" integer, "month" integer, "day" integer)
IS 'From a date in year/month/day form, create a native date element';
