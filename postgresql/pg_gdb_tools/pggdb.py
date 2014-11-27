# You can set breakpoints here, or interactively
# in gdb.
#
# Watchpoints and catchpoints work too.
#
# Remember to use
#
#    continue -a &
#
# to continue.
#

gdb.execute("set breakpoint pending on")
#gdb.execute("break PostgresMain")
#gdb.execute("break PostmasterMain")
gdb.execute("set breakpoint pending auto")

# You can also set conditional breakpoints,
# watchpoints, and breakpoint/watchpoint commands,
# just like normal gdb.

#---------

gdb.execute("set python print-stack full")
gdb.execute("set detach-on-fork off")
gdb.execute("set schedule-multiple on")
gdb.execute("set follow-fork-mode parent")
gdb.execute("set non-stop on")
# There's no sensible version tuple, so we have to use string hacks
if ' 7.7.' in gdb.VERSION:
    # Only needed on 7.7.x; see http://tromey.com/blog/?p=734
    gdb.execute("set target-async on")
if ' 7.8.' in gdb.VERSION:
    gdb.execute("set print symbol-loading off")

# PostgreSQL likes to use SIGINT to signal the checkpointer, but gdb
# wants to use it to interrupt processes.
#
# There aren't proper signal handlers in gdb's Python support yet
# but we can work around it by trapping the stop event the signal
# produces, delivering the signal, and continuing.

def do_continue():
    """Async callback to issue continue from the gdb event queue"""
    gdb.execute("continue")

def stop_handler(event):
    if isinstance(event, gdb.SignalEvent):
        if event.stop_signal == "SIGINT":
            # The checkpointer gets sigints normally, so skip them if we're in
            # the postgres checkpointer process.
            #
            # This would be easier if gdb let us see the process image name for
            # the active inferior from Python.
            #
            try:
                v = gdb.parse_and_eval("IsPostmasterEnvironment && IsUnderPostmaster && CheckpointerShmem != 0")
                if v == True:
                    # We want to ignore SIGINT to the checkpointer, but gdb
                    # doesn't offer Python signal handlers, so the best we can
                    # do is deliver the signal and continue.
                    gdb.execute("signal SIGINT")
                    gdb.post_event(do_continue)
            except gdb.error:
                # Likely not the context we are interested in - wrong process, etc.
                pass

gdb.events.stop.connect(stop_handler)
