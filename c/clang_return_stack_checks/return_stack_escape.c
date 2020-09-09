#include <stdlib.h>
#include <stdio.h>
#include <error.h>

#include "guard.h"

int
foo(int do_fail)
{
	struct guard g = {0, "g", 0};
	set_guard(&g);

	/*
	 * This should emit a warning from clang's scan-build like
	 *
	 * return_stack_escape.c:14:3: warning: Address of stack memory
	 * associated with local variable 'g' is still referred to by the
	 * global variable 'guard_ptr' upon returning to the caller.  This will
	 * be a dangling reference
	 */
	if (do_fail)
		return do_fail;

	clear_guard(&g);

	return do_fail;
}

int
main(int argc, char * argv[])
{
	int do_fail;
	int ret;
	char * endpos;

	if (argc != 2)
		error(2, 0, "usage: %s 0|1", argv[0]);

	do_fail = strtol(argv[1], &endpos, 10);
	if (*endpos != '\0')
		error(2, 0, "couldn't parse \"%s\" as an integer", argv[1]);

	(void) foo(do_fail);

	ret = !check_guard();
	fputc('\n', stderr);
	return ret;

}
