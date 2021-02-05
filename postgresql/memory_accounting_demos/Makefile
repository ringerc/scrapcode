SCAN_BUILD?=

all: overcommit fork_cow_demo

fork_cow_demo: fork_cow_demo.c
	$(SCAN_BUILD) gcc -ggdb3 -Wall -Wextra $< -o $@ -lrt
overcommit: overcommit.c
	$(SCAN_BUILD) gcc -ggdb3 -Wall -Wextra $< -o $@

clean:
	rm -f overcommit fork_cow_demo
