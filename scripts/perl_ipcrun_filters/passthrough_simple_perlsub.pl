#!/usr/bin/perl

use strict;
use warnings;
use IPC::Run qw(start input_avail);
use Data::Dumper;

my @psql_cmd = ('psql', @ARGV);

# Perl sub to filter stdout before passing on to the caller's stdout.
#
# Uses IPC::Run's filter helper functions, see
# https://metacpan.org/pod/IPC::Run#FILTER-IMPLEMENTATION-FUNCTIONS
# An example filter is shown in https://metacpan.org/pod/IPC::Run#new_chunker .
# but filters appear to behave differently based on their attachment
# streams so this uses a simple input and print.
#
# This runs in the main perl interpreter, not a fork()ed one, and can
# share variables with filter_stderr.
#
sub filter_stdout {
    my $in = shift;
    return input_avail && do {
        print STDOUT $in;
        1;
    };
}

# Process stderr separately to stdout
sub filter_stderr {
    my $in = shift;
    return input_avail && do {
        # Discard matching lines:
        $in =~ s/^(?:ERROR|DETAIL|CONTEXT):.*$//mg;
        # Send output
        print STDERR $in;
        1;
    };
}

# It's probably a good idea to request eager pipe flushing
$|++;

# Connect psql stdin, stdout and stderr to ours via filters, then run until
# psql exits and forward its exit code. stdout and stderr are combined into
# a common stream on stdout.
open(my $stdin, '<-') or die "cannot open stdin: $!";
open(my $stdout, '>-') or die "cannot open stdout: $!";
my $h = start( \@psql_cmd, '<', $stdin, '>', \&filter_stdout, '2>', \&filter_stderr );
$h->finish;
exit $h->result(0);
