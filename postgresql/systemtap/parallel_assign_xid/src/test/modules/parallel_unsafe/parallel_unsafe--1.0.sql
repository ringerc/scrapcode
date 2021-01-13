-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION parallel_unsafe" to load this file. \quit

CREATE FUNCTION parallel_unsafe(pg_catalog.int4)
RETURNS pg_catalog.int4
PARALLEL SAFE
AS 'MODULE_PATHNAME','parallel_unsafe' LANGUAGE C;
