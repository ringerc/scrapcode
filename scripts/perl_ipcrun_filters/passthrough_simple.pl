#!/usr/bin/perl

use strict;
use warnings;
use IPC::Run;

my @psql_cmd = ('psql', @ARGV);

# Pipeline command to filter psql stdout and stderr streams. This one
# deletes HINT, DETAIL and CONTEXT lines as well as LINE markers.
my @psql_filter_out = ('awk', q~
	/^(ERROR|DETAIL|CONTEXT):/ {
		next;
	};
	/^LINE [0-9]+:/ {
		# Consume LINE
		getline;
		# consume the position-marker caret and keep going
		next;
	}
	{
		print;
	}
~);

# It's probably a good idea to request eager pipe flushing
$|++;

# Connect psql stdin, stdout and stderr to ours via filters, then run until
# psql exits and forward its exit code. stdout and stderr are combined into
# a common stream on stdout.
open(my $stdin, '<-') or die "cannot open stdin: $!";
open(my $stdout, '>-') or die "cannot open stdout: $!";
my $h = IPC::Run::start( \@psql_cmd, '<', $stdin, '2>&1', '|', \@psql_filter_out, '>&', $stdout, '2>&1' );
$h->finish;
exit $h->result(0);
