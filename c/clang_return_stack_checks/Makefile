CC ?= clang
CFLAGS ?= -Wall -Wextra

all: return_stack_escape

guard.so: guard.c guard.h
	$(CC) -shared -fPIC $< -o $@

return_stack_escape: return_stack_escape.c guard.so
	$(CC) $< ./guard.so -o $@

clean:
	rm -f guard.so return_stack_escape
