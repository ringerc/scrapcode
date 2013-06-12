#!/usr/bin/env python3

"""
This simple demo was written for http://stackoverflow.com/questions/17052049/test-script-for-transaction-concurrency-for-postgresql/17055880#17055880

It shows how Python with the multiprocessing module can be used to do race
condition testing without needing to use PostgreSQL-level locking, instead
using Python's Barrier class.
"""

import sys
if sys.version_info[0:2] < (3,3):
	print("Python 3.3 or newer required")
	sys.exit(2)

from multiprocessing import Process, Barrier, Pipe
import psycopg2
import time

dsn = "dbname=regress"

class ConcurrentRunner(Process):

	def __init__(self, name, sql, run_barrier, has_result=False, delay_seconds=0):
		"""Setup run in the parent process before forking to the subprocess"""
		Process.__init__(self)
		self.name = name
		self.sql = sql
		self.run_barrier = run_barrier
		self.result = None
		self.exception = None
		self.has_result = has_result
		self.delay_seconds = delay_seconds
		(self.reader, self.writer) = Pipe(duplex=False)

	def run(self):
		"""The main funtion of the test subprocess"""
		print("Starting worker...")
		conn = psycopg2.connect(dsn)
		curs = conn.cursor()
		print("Waiting...")
		self.run_barrier.wait()
		print("Testing...")
		time.sleep(self.delay_seconds)
		try:
			curs.execute(self.sql)
			if self.has_result:
				self.result = curs.fetchall()
			conn.commit()
		except Exception as ex:
			self.exception = ex
		conn.close()
		print("Sending result...")
		self.writer.send((self.result,self.exception))
		print("Done")

	def get_result(self):
		"""Fetch the outcome from the subprocess and store it in
		the result and exception fields.
		"""
		(self.result, self.exception) = self.reader.recv()

def main():
	print("Doing setup...")
	conn = psycopg2.connect(dsn)
	curs = conn.cursor()
	curs.execute("CREATE TABLE IF NOT EXISTS concurrent_tx(id integer primary key);")
	conn.commit()
	print("done")

	print("Creating workers...")
	run_barrier = Barrier(3, timeout=10)
	# Race to insert a row
	runner1 = ConcurrentRunner("runner1", "INSERT INTO concurrent_tx(id) VALUES (1);", run_barrier, delay_seconds=0)
	runner2 = ConcurrentRunner("runner2", "INSERT INTO concurrent_tx(id) VALUES (1);", run_barrier, delay_seconds=0.01)
	# Start them waiting on the barrier, letting them get their connections set up:
	print("Starting workers...")
	runner1.start()
	runner2.start()
	# and release the barrier. It won't actually get released until all workers
	# have connected and are also waiting on the barrier.
	print("Releasing barrier...")
	run_barrier.wait()
	print("Waiting for results...")
	# OK, we're running. Wait until both workers finish.
	workers = [runner1, runner2]
	for worker in workers:
		worker.get_result()
	# Wait for termination
	for worker in workers:
		worker.join()
	# and report results
	for worker in workers:
		if worker.exception is not None:
			print("Worker {0} got exception: {1}".format(worker.name, worker.exception))
		elif worker.result is not None:
			print("Worker {0} got result: {1}".format(worker.name, worker.result))
		else:
			print("Worker {0} succeeded silently.".format(worker.name))

if __name__ == '__main__':
	main()
