CREATE OR REPLACE FUNCTION bytea_to_uuid(bytea) RETURNS uuid
	AS 'uuid_bytea','bytea_to_uuid'
	LANGUAGE c STRICT;

COMMENT ON FUNCTION bytea_to_uuid(bytea) IS 'Take a 128 bit (16 byte) binary uuid as input and convert it to a uuid typed value';

CREATE OR REPLACE FUNCTION uuid_to_bytea(uuid) RETURNS bytea
	AS 'uuid_bytea','uuid_to_bytea'
	LANGUAGE c STRICT;

COMMENT ON FUNCTION uuid_to_bytea(uuid) IS 'Take a uuid value and return a 128 bit (16 byte) binary uuid';

CREATE CAST (uuid AS bytea) WITH FUNCTION uuid_to_bytea(uuid);
CREATE CAST (bytea AS uuid) WITH FUNCTION bytea_to_uuid(bytea);
