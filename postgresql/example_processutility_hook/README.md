This is an example `ProcessUtility_hook` for PostgreSQL 9.2 or 9.3, showing how
to use `ProcessUtility_hook`. It logs DROP DATABASE statements.

The signature of `ProcessUtility_hook` changed in 9.3 so you need a different module
for that.

For an example of doing something more useful with a `ProcessUtility_hook` you
can check out [my command filter for the alpha version of PostgreSQL
bi-directional logical
replication](https://github.com/ringerc/postgres/blob/bdr-reject-unsafe-commands/contrib/bdr/bdr_commandfilter.c),
which selectively rejects certain commands.
