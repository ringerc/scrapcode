.DEFAULT_GOAL := disassembly

# Set to 0 to recompile with tracepoints completely omitted, as if
# stubbed out by macros.
USE_SDT ?= 1

# Set to 0 to compile with tracepoints unguarded by semaphores
USE_SDT_SEMAPHORES ?= 1

CFLAGS += -Wall -Wextra -O2 -finline-functions

ifeq ($(USE_SDT),1)
  ifeq ($(USE_SDT_SEMAPHORES),1)
    variant=semaphores
    CPPFLAGS += -DUSE_SDT_SEMAPHORES
  else
    variant=enabled
  endif
  sdt_noop.c: sdt_noop_probes_enabled.h
  variant=enabled
  extra_objs+=sdt_noop_probes_enabled.o
  CFLAGS+=-DUSE_SDT
else
  sdt_noop.c: sdt_noop_probes_disabled.h
  variant=disabled
endif

sdt_noop_probes_enabled.h sdt_noop_probes_semaphores.: sdt_noop_probes.d
	dtrace -h -C -o $@ -s $<

sdt_noop_probes_enabled.o sdt_noop_probes_semaphores.o: sdt_noop_probes.d
	dtrace -G -o $@ -s $<

sdt_noop_probes_disabled.h: sdt_noop_probes.d
	sed -E \
	    -e '/[\t ]*probe/!d' \
	    -e 's/([\(,][^\),]+)/\1 __attribute__((unused))/g' \
	    -e 's/\<probe\>[\t ]*([^ \(]*)/__attribute__((always_inline)) static inline void SDT_NOOP_\U\1/' \
	    -e 's/;/ {}/' $< > $@
	rm -f sdt_noop_probes.o

sdt_noop_$(variant).o: sdt_noop.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

sdt_noop_$(variant): sdt_noop_$(variant).o $(extra_objs)

define newline


endef

symbols=main no_args with_many_args with_global_arg with_volatile_arg with_computed_arg
disassembly: sdt_noop_$(variant)
	$(foreach symbol,$(symbols),\
	    objdump -j .text --disassemble="$(symbol)" --no-addresses --no-show-raw-insn sdt_noop_$(variant); $(newline) \
	    objdump -j .text --disassemble="$(symbol).part.0" --no-addresses --no-show-raw-insn sdt_noop_$(variant); $(newline) \
	)

clean:
	rm -f sdt_noop*.o sdt_noop_probes_*.h
	$(foreach v,enabled disabled semaphores, rm -f sdt_noop_$(v) )
