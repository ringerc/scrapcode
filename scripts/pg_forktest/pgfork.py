#!/usr/bin/env python
#
# pgfork.py is a simple hack I wrote to spawn a lot of connections to a
# PostgreSQL database very rapidly, actively using a proportion of the
# connections to do dummy work.
#
# This code uses fork() to set up the workers, POSIX signals to
# communicate with them, and a shared temp file protected by flock()
# to report results. It's amazing that it works at all - and unless
# you signal it during startup, it actually does.
#
# pg_mp.py is a much cleaner approach to the same problem.

import psycopg2
import sys
import os
import time
import signal
import random
import tempfile
import fcntl

dummy_statement = "SELECT 1;"
conn_string = "dbname=regress"
n_conns = 800
n_workers = 100
connections = []
pids = []

conns_per_worker = n_conns/n_workers
result_file = tempfile.NamedTemporaryFile(delete=False)

def create_connections(worker_number):
    for i in xrange(worker_number * conns_per_worker, (worker_number+1) * conns_per_worker):
        connections.append( psycopg2.connect(conn_string) )

def close_connections():
    for conn in connections:
        conn.close()

def handle_term(signo, stackframe):
    global worker_continue
    worker_continue = False

def do_dummy_work(conn):
    curs = conn.cursor()
    curs.execute(dummy_statement)
    curs.fetchall()
    curs.close()

def do_worker(worker_number):
    # Forked child; spawn connections then issue work on a random connection until
    # time's up.
    result_file.close()
    global worker_continue
    worker_continue = True
    signal.signal(signal.SIGTERM, handle_term)
    create_connections(worker_number)
    sys.stderr.write(str(worker_number) + " ")
    sys.stderr.flush()
    n_queries = 0
    while worker_continue:
       conn_idx = random.randint(0,conns_per_worker)
       do_dummy_work(connections[random.randint(0,conns_per_worker-1)])
       n_queries += 1
    close_connections()
    f = open(result_file.name, "r+b")
    fcntl.flock(f, fcntl.LOCK_EX)
    f.seek(0,2)
    f.write("%i\t%i\n" % (worker_number, n_queries))
    f.flush()
    fcntl.flock(f, fcntl.LOCK_UN)
    f.close()
    sys.exit(0)


def kill_children():
    try:
        for pid in pids:
            os.kill(pid, signal.SIGTERM)
    except OSError,e:
        if e.errno == 3:
            # No such process; already gone
            pass
        else:
            raise e

def run_parent():
    # Make sure we can connect
    conn = psycopg2.connect(conn_string)
    curs = conn.cursor()
    curs.execute(dummy_statement)
    curs.fetchall()
    curs.close()
    conn.close()

    # Spawn workers
    for i in xrange(0, n_workers):
        child_pid = os.fork()
        if child_pid == 0:
            try:
                do_worker(i) # Never returns
            except KeyboardInterrupt,e:
                pass
        else:
            pids.append(child_pid)

    # [only parent ever reaches here ]
    time.sleep(2)
    sys.stderr.write("\n\n")

    # wait a while for children to thrash the system
    # then terminate the children
    time.sleep(10)
    kill_children()

    # Now parent waits for children to exit
    try:
        while True:
            (pid, ret) = os.wait()
            if pid == 0:
                break;
            elif pid == -1:
                # Interrupted, keep waiting
                pass
            elif ret != 0:
                sys.stderr.write("Pid %i exited with %i\n" % (pid,ret))
    except OSError,err:
        if err.errno == 10:
            pass
        else:
            raise

    # Read the results
    result_file.seek(0,0)
    total_tx = 0
    for line in result_file:
        (worker_number, num_tx) = tuple(line.strip().split("\t"))
        total_tx += int(num_tx)
    os.unlink(result_file.name)

    sys.stderr.write("Done, executed %i statements\n" % total_tx)

if __name__ == '__main__':
    try:
        run_parent()
    except Exception,e:
        kill_children()
        raise
