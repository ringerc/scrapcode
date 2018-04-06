#!/usr/bin/env python

import psycopg2
import os
import sys

conn = psycopg2.connect("dbname=postgres")
curs = conn.cursor()

curs.execute("""
CREATE TABLE fsync_torture(
    id serial primary key,
    padding text
);
""")

conn.commit()

chars = 0
while True:
    # To avoid fsync()s from commits, we try to write huge txns and force
    # checkpoints in the middle to fsync our relation and remove old WAL.
    #
    # Rememember, psycopg2 opens explicit txns by default and needs explicit
    # commit. No autocommit here.
    #
    for i in range(100):
        curs.execute("""INSERT INTO fsync_torture (padding) SELECT repeat('1234567891-', 1000) FROM generate_series(1,100);""")

    try:
        curs.execute("CHECKPOINT")
    except psycopg2.Error as e:
        print("Checkpoint failed with {}".format(e))
        print("Retrying")
        conn.rollback();
        try:
            curs.execute("CHECKPOINT")
            print("Ooops, it worked! We ignored the error and checkpointed OK.")
            sys.exit(1)
        except psycopg2.Error as e:
            print("Good, checkpoint failed again with {}".format(e))
            sys.exit(0)

    print(".", end='', flush=True)
    chars += 1

    if (chars == 80):
        curs.execute("SELECT pg_relation_size('fsync_torture'::regclass);")
        (size,) = curs.fetchone()
        print(" {:12d}".format(size))
        chars = 0
