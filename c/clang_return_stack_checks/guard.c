#include "guard.h"
#include <stdio.h>

struct guard * guard_ptr = 0;

int
check_guard(void)
{
	if (guard_ptr)
	{
		/* can't safely print guard name etc due to stack safety */
		fprintf(stderr, "guard not ok: guard pointer %p", guard_ptr);
	}
	else
	{
		fprintf(stderr, "guard ok: guard pointer empty");
	}
	return guard_ptr == 0;
}
