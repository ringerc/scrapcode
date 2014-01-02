CREATE OR REPLACE FUNCTION hex2dec(hexstr text) RETURNS bigint
	AS 'hex2dec','hex2dec'
	LANGUAGE c IMMUTABLE STRICT;

COMMENT ON FUNCTION hex2dec(hexstr text)
IS 'Decode the hex string passed, which may optionally have a leading 0x, as a bigint. Does not attempt to consider negative hex values.';
