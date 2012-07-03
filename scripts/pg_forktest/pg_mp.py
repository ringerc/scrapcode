#!/usr/bin/env python
#
# pg_mp.py is a cleaned up version of pgfork.py that uses
# the Python multiprocessing framework instead of home
# rolled fork, signalling, and communication via a
# shared tempfile.
#
# It works a lot better and is much easier to understand.

import psycopg2
from multiprocessing import Process, Queue, Event
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

conns_per_worker = n_conns/n_workers

def create_connections(worker_number):
    for i in xrange(worker_number * conns_per_worker, (worker_number+1) * conns_per_worker):
        connections.append( psycopg2.connect(conn_string) )

def close_connections():
    for conn in connections:
        conn.close()

def do_dummy_work(conn):
    curs = conn.cursor()
    curs.execute(dummy_statement)
    curs.fetchall()
    curs.close()

def child_run(worker_number, result_queue, start_lock, done_event):
    try:
        # Forked child; spawn connections then issue work on a random connection until
        # time's up.
        global worker_continue
        worker_continue = True
        # Would prefer to be able to acquire in shared mode, but this'll do...
        start_lock.wait()
        create_connections(worker_number)
        sys.stderr.write(str(worker_number) + " ")
        sys.stderr.flush()
        n_queries = 0
        while start_lock.is_set():
           conn_idx = random.randint(0,conns_per_worker)
           do_dummy_work(connections[random.randint(0,conns_per_worker-1)])
           n_queries += 1
        close_connections()
        result_queue.put( (worker_number, n_queries) )
        done_event.set()
    except KeyboardInterrupt,e:
        pass


def parent_run():

    # Make sure we can connect
    conn = psycopg2.connect(conn_string)
    curs = conn.cursor()
    curs.execute(dummy_statement)
    curs.fetchall()
    curs.close()
    conn.close()

    # Spawn workers
    children = []
    result_queue = Queue()
    start_lock = Event()
    for i in xrange(0, n_workers):
        done_event = Event()
        child = Process(target=child_run, args=(i,result_queue,start_lock,done_event))
        children.append(child)
        child.done_event = done_event

    for child in children:
        child.start()

    # and start them working
    start_lock.set()

    # wait a while for children to thrash the system
    # then ask them to finish
    time.sleep(10)
    start_lock.clear()

    # Wait until all children have reported in
    sys.stderr.write("Waiting for children to complete...")
    sys.stderr.flush()
    for child in children:
        child.done_event.wait()
    sys.stderr.write(" done\n")

    # Read the results
    results = []
    for i in range(0, len(children)):
        (worker_number, num_tx) = result_queue.get(False)
        results.append(num_tx)
    total_tx = sum(results)
    mean_tx = float(total_tx) / float(len(results))

    # and wait for children to exit
    for child in children:
        child.join()


    sys.stderr.write("Done, executed %i statements with %i conns and %i workers\n" % (total_tx, n_conns, n_workers))
    sys.stderr.write("Mean tx per worker was %.2f\n" % mean_tx)

if __name__ == '__main__':
    try:
        parent_run()
    except KeyboardInterrupt,e:
        pass
