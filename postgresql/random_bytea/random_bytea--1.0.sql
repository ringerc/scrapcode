CREATE OR REPLACE FUNCTION random_bytea(integer) RETURNS bytea
	AS 'random_bytea','random_bytea'
	LANGUAGE C STRICT;

COMMENT ON FUNCTION random_bytea(integer) IS 'Generate n random bytes of garbage, returned as bytea';
