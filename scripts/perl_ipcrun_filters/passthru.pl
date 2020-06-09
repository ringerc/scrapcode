use strict;
use warnings;

my $sel_timeout_s = 10;
my $debug_wrapper = $ENV{'DEBUG_PSQL_WRAPPER'} // 0;

use IPC::Run;
use IO::Select;


sub wrap_debug {
    my $msg = shift;
    print STDERR "wrapper: " . $msg if ($debug_wrapper);
}

my $psql_in;
my $psql_out;
my $psql_err;

# Launch a psql with its stdin, stdout  and stderr connected to pipes we
# control. Line-buffer psql interaction.
#
# We're not likely to need 8-bit clean binary but if we do, we'd have
# to call 'binmode' and we'd need to use
#
#   '<pipe', IPC::Run::binary, \$psql_in,
#   '>pipe', IPC::Run::binary, \$psql_out,
#   '2>pipe', IPC::Run::binary, \$psql_err
#
# instead.

my $h = IPC::Run::start(['psql'],
    '<pipe', IPC::Run::binary, \$psql_in,
    '>pipe', IPC::Run::binary, \$psql_out,
    '2>pipe', IPC::Run::binary, \$psql_err
);

# Set up lists of file handles to select() when doing I/O on our child psql
# process.
my $sel_readers = IO::Select->new();
$sel_readers->add($psql_out, $psql_err);
$sel_readers->add(\*STDIN);
my $sel_writers = IO::Select->new();
$sel_writers->add($psql_in);
$sel_writers->add(\*STDOUT, \*STDERR);
my $sel_errs = IO::Select->new();
$sel_errs->add($psql_in, $psql_out, $psql_err);

# Forward our own stdin to psql's stdin, and forward its stdout and stderr back
# onto our own stdout/stderr. Don't block except in select(), where we can
# handle any combination of I/O availability.
#
do {
    $! = 0;
    my $sel = IO::Select::select($sel_readers, $sel_writers, $sel_errs, $sel_timeout_s);
    if ($sel) {
        # Some I/O possible.
        my ($readable, $writeable, $errs) = $sel;
        if ($errs) {
            # probably shouldn't happen, bail
            wrap_debug("select: I/O error reported on one or more handles");
            last;
        }
        if ($readable) {
        }
        next;
    } elsif ($!) {
        # select() error, probably child-process exit. We don't want to produce
        # output that might confuse the consumer of psql's own streams, so
        # unless debugging is enabled just bail here.
        wrap_debug("select: $!");
        last;
    } else {
        # Polling timeout, we'll just continue. Timeouts are only used so we can
        # check pumpable() in case IPC::Run notices something wrong.
        wrap_debug("select: timeout");
        next;
    }
} while ($h->pumpable)

$h->finish;

