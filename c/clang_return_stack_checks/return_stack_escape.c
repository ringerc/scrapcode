#include <stdlib.h>
#include <stdio.h>
#include <error.h>

#include "guard.h"

int
foo(int do_fail)
{
	struct guard g = {0};
	set_guard(&g);

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

	ret = foo(do_fail);

	if (guard_ptr)
		printf("guard value: %hhd\n", guard_ptr->guard_set);
	else
		printf("guard value: no guard pointer\n");

	return ret;
}
