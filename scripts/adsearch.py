#!/usr/bin/env python3
#
# Very quick and dirty CGI script for using `locate` as a file system index
# for a search, spitting results out in HTML. Yes, it's ugly, but I only had an
# hour and I don't do much CGI in Python these days.
#

# requires Python 3.1

import sys
import subprocess
import urllib.parse
import cgi
import cgitb
cgitb.enable()

def printhttpheaders():
	print("Content-Type: text/html")
	print("")

def printheading():
	print("""
	<html>
	<head><title>Search Old Ads</title></head>
	<body>
	  <h1>Search Old Ads</h1>
	""")

def printform(searchterm):
	print("""
	  <form method="get">
		Search for:
		<input type="text" value="%s" name="searchterm"/>
		<input type="submit" value="Search"/>
	  </form>
	""" % cgi.escape(searchterm))

def print_results(res):
	origreslen= len(res)
	res.sort(reverse=True)
	truncated = False
	if len(res) > 400:
		res = res[:400]
		truncated = True
	print('<ul>')
	for r in [ x[20:] for x in res if not "/._" in x ]:
		if r.strip():
			print('<li>'+cgi.escape(r)+'</li>')
	print('</ul>')
	if truncated:
		print('<h2>Only displayed first %s results of %s. Please choose a more specific search term.</h2>' % (len(res), origreslen))
	else:
		print('<h2>Found %s results</h2>' % len(res))

def printfooter():
	print("""
	</body>
	</html>
	""")

def search(searchterm):
	if not searchterm:
		return False
	try:
		results = subprocess.check_output(['/usr/bin/locate', '/srv/archive/old_ads/*'+searchterm+'*']).decode("utf-8").split("\n")
		print_results(results)
		return True
	except subprocess.CalledProcessError as ex:
		if ex.returncode == 1:
			print("<h2>No results found for search term " + searchterm + "</h2>")
			return False
		else:
			raise

if __name__ == '__main__':
	printhttpheaders()
	printheading()
	form = cgi.FieldStorage()
	searchterm = ""
	if 'searchterm' in form:
			searchterm = form['searchterm'].value
	printform(searchterm)
	has_result = search(searchterm)
	if has_result:
		printform(searchterm)
	printfooter()
