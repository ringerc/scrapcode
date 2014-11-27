# You can set breakpoints here, or interactively
# in gdb.
#
# Watchpoints and catchpoints work too.
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


# Stop the postmaster killing backends when it sees a
# segfault or similar.
gdb.execute("handle SIGQUIT nostop noprint nopass")

def do_continue():
    gdb.execute("continue")

def exit_handler(event):
    global my_stop_request
    if event.exit_code == 0:
        # TODO: Should also accept nonzero exits that aren't a result of fatal
        # signals, except by 'postgres' backends under the postmaster. gdb doesn't
        # give us enough information for this at the moment.
        has_threads = [ inferior.num for inferior in gdb.inferiors() if inferior.threads() ]
        if has_threads:
            has_threads.sort()
            gdb.execute("inferior %d" % has_threads[0])
            my_stop_request = True

gdb.events.exited.connect(exit_handler)

def stop_handler(event):
    global my_stop_request
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
                # Likely not the context we are interested in
                pass
        elif event.stop_signal in ("SIGSEGV", "SIGABRT"):
            # Event of interest happened, disable exit handler
            gdb.events.exited.disconnect(exit_handler)
    elif isinstance(event, gdb.BreakpointEvent):
        pass
    elif my_stop_request:
        my_stop_request = False
        gdb.post_event(do_continue)

gdb.events.stop.connect(stop_handler)
