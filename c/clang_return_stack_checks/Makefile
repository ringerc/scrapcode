ifeq ($(origin CC),default)
CC = scan-build clang
endif

ifeq ($(origin CFLAGS),undefined)
CFLAGS = -Wall -Wextra -Og -ggdb3
endif

all: return_stack_escape

guard.so: guard.c guard.h
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@

return_stack_escape: return_stack_escape.c guard.so
	$(CC) $(CFLAGS) $< ./guard.so -o $@

clean:
	rm -f guard.so return_stack_escape

run:
	./return_stack_escape 0
	if ./return_stack_escape; then exit 1; fi
