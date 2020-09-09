#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <error.h>

#include "guard.h"

/*
 * Support faking out sigsetjmp() to see if clang scan-build
 * behaves differently based on the call provided here.
 */
#ifdef FAKE_SIGSETJMP_RETURN
#ifdef sigsetjmp
#undef sigsetjmp
#endif
#define sigsetjmp(buf,clearsig) (FAKE_SIGSETJMP_RETURN)
#endif /* FAKE_SIGSETJMP_RETURN */

static sigjmp_buf * jbuf = 0;

static void
do_a_jump(void)
{
	/* Jump back to the alternate branch */
	fprintf(stderr, "    jumping to branch 1\n");
	siglongjmp(*jbuf, 1);
}

static void
test_jmp(int should_jump, int should_return_early)
{
	struct guard g_outer = {0, "g_outer", 0};
	set_guard(&g_outer);

	sigjmp_buf b;
	if (sigsetjmp(b, 0) == 0)
	{
#ifdef USE_INNER_GUARDS
		struct guard g_branch_0 = {0, "g_branch_0", 0};
		set_guard(&g_branch_0);
		assert(guard_ptr->previous == &g_outer);
#endif

		jbuf = &b;

		fprintf(stderr, "    took branch 0\n");

		if (should_jump)
		{
#ifdef USE_INNER_GUARDS
			clear_guard(&g_branch_0);
#endif
			do_a_jump();
		}

		if (should_return_early)
		{

			/*
			 * Interestingly scan-build appears not to understand that when we
			 * -DUSE_INNER_GUARDS and run with should_jump=0, the pointer to
			 *  g_outer stored in g_branch_0->previous by
			 *  set_guard(&g_branch_0) has still escaped. It either didn't
			 *  follow the indirection, or only reports the escape of
			 * g_branch_0. We can check which by optionally popping only one
			 * level of guards:
			 */
#if defined(USE_INNER_GUARDS) && defined(POP_ONLY_INNER_GUARDS)
			fprintf(stderr, "    clearing g_branch_0 before early return\n");
			clear_guard(&g_branch_0);
			assert(guard_ptr == &g_outer);
#endif

			/*
			 * Deliberately fail to clear guards guard_outer or g_branch_0 here.
			 *
			 * Should raise clang scan-build warning
			 *     warning: Address of stack memory associated with local variable 'b' is still referred to by the global variable 'jbuf' upon returning to the caller.  This will be a dangling reference
			 *
			 * and if USE_INNER_GUARDS then
			 *     warning: Address of stack memory associated with local variable 'g_branch_0' is still referred to by the global variable 'guard_ptr' upon returning to the caller.  This will be a dangling reference
			 *
			 * otherwise the same complaint about g_outer.
			 */
			fprintf(stderr, "    escaping branch 0 early\n");
			return;
		}

#ifdef USE_INNER_GUARDS
		clear_guard(&g_branch_0);
#endif
	}
	else
	{
#ifdef USE_INNER_GUARDS
		struct guard g_branch_1 = {0, "g_branch_1", 0};
		set_guard(&g_branch_1);
#endif

		/* entered with should_jump == 1 and took the do_a_jump() branch */
		fprintf(stderr, "    took branch 1\n");

		if (should_return_early)
		{
#if defined(USE_INNER_GUARDS) && defined(POP_ONLY_INNER_GUARDS)
			/*
			 * Like the branch0 case, we'll also check what happens if we pop
			 * only one level of guards.
			 */
			fprintf(stderr, "    clearing g_branch_1 before early return\n");
			clear_guard(&g_branch_1);
			assert(guard_ptr == &g_outer);
#endif

			/*
			 * Deliberately fail to clear guards guard_outer or g_branch_1 here.
			 *
			 * If -DUSE_INNER_GUARDS, should raise clang scan-build warning:
			 *
			 *     warning: Address of stack memory associated with local variable 'g_branch_1' is still referred to by the global variable 'guard_ptr' upon returning to the caller.  This will be a dangling reference
			 *
			 * Otherwise it should complain about g_outer.
			 */

			fprintf(stderr, "     escaping branch 1 early\n");
			return;
		}

#ifdef USE_INNER_GUARDS
		clear_guard(&g_branch_1);
#endif
	}

	/* Properly clear stored jump buffer */
	jbuf = 0;

	/*
	 * scan-build fails to notice that no path reaches this code when
	 * called with should_return_early=1 (for either value of should_jump),
	 * so it does not detect the escape of g_outer.
	 *
	 * clang should be warning us when we can escape with a leaked guard
	 * pointer, but it seems like it gets confused by sigsetjmp.
	 */
	clear_guard(&g_outer);
}

int
main(int argc, char * argv[])
{
	int should_jump;
	int should_return_early;
	int guard_ok;
	if (argc != 3)
		error(2, 0, "usage: %s {{should_jump 0|1}} {{should_return_early 0|1}}", argv[0]);
	should_jump = atoi(argv[1]);
	should_return_early = atoi(argv[2]);
	fprintf(stderr, "sigjmp_escape(%d,%d):\n",
			should_jump, should_return_early);
	test_jmp(should_jump, should_return_early);
	guard_ok = check_guard();
	fputs("\n", stderr);
	if (jbuf != 0)
		error(1, 0, "    escaped with invalid sigjmp_buf");
	else if (!guard_ok)
		error(1, 0, "    escaped with bad guard state");
	else
		fprintf(stderr, "    ok\n");

	return 0;
}
