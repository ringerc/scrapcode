#include "sigjmp_escape_hdr_try.h"

#include <stdlib.h>
#include <stdio.h>
#include <error.h>

static void
test_jmp(int should_jump, int should_return_early)
{
	TEST_TRY();
	{
		fprintf(stderr, "    took branch 0\n");

		if (should_jump)
		{
			do_a_jump(1);
		}

		if (should_return_early)
		{
			/*
			 * This should result in an escape of &b and a warning like
			 *
			 *       warning: Address of stack memory associated with local variable '_local_sigjmp_buf' is still referred to by the global variable 'TEST_exception_stack' upon returning to the caller.  This will be a dangling reference
			 *
			 */
			fprintf(stderr, "    escaping branch 0 early\n");
			return;
		}
	}
#if USE_FINALLY
	TEST_FINALLY();
#else
	TEST_CATCH();
#endif
	{
		/* entered with should_jump == 1 and took the do_a_jump() branch */
		fprintf(stderr, "    took branch 1\n");

		if (should_return_early)
		{
			/*
			 * Because we're coded like PG_TRY() this will not
			 * raise a warning if we use TEST_CATCH(). The jmpbuf
			 * in TEST_exception_stack is cleared, and it's safe
			 * albeit ugly to return here.
			 *
			 * Returning from TEST_FINALLY() would be incorrect,
			 * but will not leak the stack pointer, so it won't
			 * raise a warning here either. We'd need a separate
			 * guard in the TEST_TRY() / TEST_END_TRY() to detect
			 * that.
			 */
			fprintf(stderr, "     escaping branch 1 early\n");
			return;
		}
	}
	TEST_END_TRY();
}

int
main(int argc, char * argv[])
{
	int should_jump;
	int should_return_early;
	if (argc != 3)
		error(2, 0, "usage: %s {{should_jump 0|1}} {{should_return_early 0|1}}", argv[0]);
	should_jump = atoi(argv[1]);
	should_return_early = atoi(argv[2]);
	fprintf(stderr, "sigjmp_escape_hdr(%d,%d):\n",
			should_jump, should_return_early);
	test_jmp(should_jump, should_return_early);
	if (TEST_exception_stack != 0)
		error(1, 0, "    escaped with invalid sigjmp_buf in TEST_exception_stack");
	else
		fprintf(stderr, "    ok\n");

	return 0;
}
