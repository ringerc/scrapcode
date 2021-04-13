.DEFAULT_GOAL := disassembly

USE_SDT_SEMAPHORES ?= 1

ifeq ($(USE_SDT_SEMAPHORES),1)
CPPFLAGS += -DUSE_SDT_SEMAPHORES
endif

sdt_noop_probes.h: sdt_noop_probes.d
	dtrace -h -C -o $@ -s $<

sdt_noop_probes.o: sdt_noop_probes.d
	dtrace -G -o $@ -s $<

sdt_noop.o: sdt_noop_probes.h sdt_noop.c

sdt_noop: sdt_noop.o sdt_noop_probes.o

define newline


endef

symbols=main no_args with_many_args with_global_arg with_volatile_arg with_computed_arg
disassembly: sdt_noop
	$(foreach symbol,$(symbols),\
	    objdump -j .text --disassemble="$(symbol)" --no-addresses --no-show-raw-insn sdt_noop; $(newline) \
	)

clean:
	rm -f sdt_noop_probes.o sdt_noop_probes.h sdt_noop sdt_noop.o
