A small Python script for GDB's embedded interpreter that sets up multi-process
debugging for gdb, so you can debug the postmaster (or some parent of the
postmaster) and set breakpoints on any PostgreSQL function in any backend, or
trap fatal signals in any child.

**If you use this on a PostgreSQL instance running on data you care about, you're insane**.

Requires GDB 7.7, prefers GDB 7.8

Just

    gdb --args make check
    (gdb) source pggdb.py
    (gdb) break myfunction
    (gdb) run

More info at http://blog.2ndquadrant.com/processes-breakpoints-watchpoints-postgresql/

For more info on GDB's multi-process support, see:

* https://sourceware.org/gdb/onlinedocs/gdb/Inferiors-and-Programs.html
* https://sourceware.org/gdb/onlinedocs/gdb/Forks.html#Forks
* http://tromey.com/blog/?p=734
