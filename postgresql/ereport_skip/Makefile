# src/test/modules/ereport_skip/Makefile

MODULES = ereport_skip
PGFILEDESC = "ereport_skip"

EXTENSION = ereport_skip
DATA = ereport_skip--1.0.sql

REGRESS = ereport_skip_constexpr \
	  ereport_skip_nonconst

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = src/test/modules/ereport_skip
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
