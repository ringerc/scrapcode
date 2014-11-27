A small Python script for GDB's embedded interpreter that sets up multi-process
debugging for gdb, so you can debug the postmaster (or some parent of the
postmaster) and set breakpoints on any PostgreSQL function in any backend, or
trap fatal signals in any child.

Just

    gdb --args make check
    (gdb) source pggdb.py
    (gdb) break myfunction
    (gdb) run
