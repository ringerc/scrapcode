# `ereport_skip`

Demo extension showing that evaluation of ereport() arguments is bypassed
when elevel is low enough that they are not required.

Put it in src/test/modules and "make check", or run with PGXS using
installcheck.
