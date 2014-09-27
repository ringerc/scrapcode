\echo Use "CREATE EXTENSION pair" to load this file. \quit

CREATE FUNCTION replc(text)
RETURNS text
IMMUTABLE STRICT
LANGUAGE c
AS 'string_transform','replc';
