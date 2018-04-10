#!/usr/bin/env python

import psycopg2
import os
import sys
import time

connstr = "dbname=postgres"
conn = psycopg2.connect(connstr)
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
    except psycopg2.OperationalError as e:
        print("Checkpoint failed with {}".format(e))
        print("Retrying")
        for i in range(1,10):
            conn.close();
            time.sleep(0.5);
            try:
                conn = psycopg2.connect(connstr)
                curs = conn.cursor()
            except psycopg2.OperationalError as e2:
                # Cannot connect now: FATAL: the database system is starting up
                if e2.pgcode != '57P03':
                    print("Unexpected error code {} from exception {}".format(e2.pgcode, e2))
                continue

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
