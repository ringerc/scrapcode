# src/test/modules/parallel_unsafe/Makefile

MODULES = parallel_unsafe
PGFILEDESC = "parallel_unsafe - regression testing of deliberately wrong parallel safe function"

EXTENSION = parallel_unsafe
DATA = parallel_unsafe--1.0.sql

REGRESS = parallel_unsafe

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = src/test/modules/parallel_unsafe
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
