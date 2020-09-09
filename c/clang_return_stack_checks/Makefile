default: check

ifeq ($(origin CC),default)
CC = scan-build -o '$(SCAN_OUT)' clang
endif


# It doesn't seem to make much difference whether -O0 or -Og is used
OPTIMIZE_FLAG ?= -O0
#OPTIMIZE_FLAG ?= -Og

ifeq ($(origin CFLAGS),undefined)
CFLAGS = -Wall -Wextra $(OPTIMIZE_FLAG) -ggdb3
endif

all: return_stack_escape sigjmp_escape

ifndef SCAN_OUT
tmpdir:
	$(eval SCAN_OUT := $(shell mktemp -d scan-build-XXXXXXXX))
else
tmpdir: ;
endif

CFLAGS_LINK ?= $(CFLAGS) -fuse-ld=lld

guard.so: guard.c guard.h | tmpdir
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@

return_stack_escape: return_stack_escape.c guard.so | tmpdir
	$(CC) $(CFLAGS) $< ./guard.so -o $@

sigjmp_escape: sigjmp_escape.c guard.so | tmpdir
	$(CC) $(CFLAGS) -DUSE_INNER_GUARDS $< ./guard.so -o $@
	# These variants aren't run, but they exercise various combos for the static checker
	$(CC) $(CFLAGS) -DUSE_INNER_GUARDS -DFAKE_SIGSETJMP_RETURN=0 $< ./guard.so -o $@_fake0
	$(CC) $(CFLAGS) -DUSE_INNER_GUARDS -DFAKE_SIGSETJMP_RETURN=1 $< ./guard.so -o $@_fake1
	$(CC) $(CFLAGS) $< ./guard.so -o $@_noinner
	$(CC) $(CFLAGS) -DFAKE_SIGSETJMP_RETURN=0 $< ./guard.so -o $@_noinner_fake0
	$(CC) $(CFLAGS) -DFAKE_SIGSETJMP_RETURN=1 $< ./guard.so -o $@_noinner_fake1
	$(CC) $(CFLAGS) -DUSE_INNER_GUARDS -DPOP_ONLY_INNER_GUARDS $< ./guard.so -o $@_inner_pop



sigjmp_escape_hdr: sigjmp_escape_hdr.c sigjmp_escape_hdr_try.c | tmpdir
	$(CC) $(CFLAGS) -c sigjmp_escape_hdr_try.c -o sigjmp_escape_hdr_try.o
	$(CC) $(CFLAGS) -DUSE_FINALLY -c sigjmp_escape_hdr.c -o sigjmp_escape_hdr.o
	$(CC) $(CFLAGS_LINK) sigjmp_escape_hdr.o sigjmp_escape_hdr_try.o -o $@

mask_local_escape: mask_local_escape.c | tmpdir
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o guard.so return_stack_escape sigjmp_escape sigjmp_escape_fake* sigjmp_escape_guard sigjmp_escape_guard_fake* sigjmp_escape_guard_inner_pop sigjmp_escape_hdr mask_local_escape
	rm -rf scan-build-*

run_return_stack_escape: return_stack_escape
	./return_stack_escape 0
	if ./return_stack_escape 1; then exit 1; fi

run_sigjmp_escape: sigjmp_escape
	./sigjmp_escape 0 0
	./sigjmp_escape 1 0
	if ./sigjmp_escape 0 1; then exit 1; fi
	if ./sigjmp_escape 1 1; then exit 1; fi

run_sigjmp_escape_hdr: sigjmp_escape_hdr
	./sigjmp_escape_hdr 0 0
	if ./sigjmp_escape_hdr 1 0; then exit 1; fi
	if ./sigjmp_escape_hdr 0 1; then exit 1; fi
	./sigjmp_escape_hdr 1 1

run_mask_local_escape: mask_local_escape
	./mask_local_escape

run: run_return_stack_escape run_sigjmp_escape run_sigjmp_escape_hdr run_mask_local_escape

check: run
