A small Python script for GDB's embedded interpreter that sets up multi-process
debugging for gdb, so you can debug the postmaster (or some parent of the
postmaster) and set breakpoints on any PostgreSQL function in any backend, or
trap fatal signals in any child.

**If you use this on a PostgreSQL instance running on data you care about, you're insane**.

Requires GDB 7.7, prefers GDB 7.8.

You'll need to set a command on stop actions, breakpoints, etc, that does an
"interrupt -a" if you want to have everything stop. Otherwise execution will continue
normally for other processes.

e.g.

    gdb --args make check
    (gdb) source pggdb.py
    (gdb) break myfunction
    Make breakpoint pending on future shared library load? (y or [n]) y
    Breakpoint 1 (myfunction) pending
    (gdb) commands 1
    Type commands for breakpoint(s) 1, one per line.
    End with a line saying just "end".
    > interrupt -a
    > end
    (gdb) run

While running, the gdb command prompt remains active, albeit nearly unusable due to log spam.


More info at http://blog.2ndquadrant.com/processes-breakpoints-watchpoints-postgresql/

For more info on GDB's multi-process support, see:

* https://sourceware.org/gdb/onlinedocs/gdb/Inferiors-and-Programs.html
* https://sourceware.org/gdb/onlinedocs/gdb/Forks.html#Forks
* http://tromey.com/blog/?p=734
