/* intarrayadd--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION intarrayadd" to load this file. \quit

CREATE OR REPLACE FUNCTION 
int4_array_add(int4[], int4[])
RETURNS int4[]
AS 'intarrayadd','int4_array_add'
LANGUAGE c;
