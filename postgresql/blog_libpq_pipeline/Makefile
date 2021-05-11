PG_CONFIG?=pg_config
ifeq ($(shell $(PG_CONFIG) --version),)
$(error could not find pg_config as $(PG_CONFIG), try explicit PG_CONFIG var)
endif

CPATH+=:$(shell $(PG_CONFIG) --includedir):$(shell $(PG_CONFIG) --includedir-server)
CFLAGS+=$(shell $(PG_CONFIG) --cflags)
CPPFLAGS+=$(shell $(PG_CONFIG) --cppflags)

export CPATH CFLAGS CPPFLAGS

async_sample: async_sample.c
