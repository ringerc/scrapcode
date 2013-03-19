#!/usr/bin/env python3

import psycopg2

conn = psycopg2.connect("")
curs = conn.cursor()

curs.execute("""
CREATE OR REPLACE FUNCTION exceptiondemo() RETURNS void AS $$
BEGIN
  RAISE SQLSTATE 'UE001' USING MESSAGE = 'error message';
END;
$$ LANGUAGE plpgsql;
""");

try:
	curs.execute("SELECT exceptiondemo()");
except Exception as ex:
	print("SQLSTATE is " + ex.pgcode + ", error message is: " + ex.pgerror);
