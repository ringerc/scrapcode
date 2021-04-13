#include <stdint.h>

#ifdef USE_SDT
#include "sdt_noop_probes_enabled.h"
#else
#include "sdt_noop_probes_disabled.h"
#endif

void no_args(void);
int with_args(void);
int with_global_arg(void);
int with_volatile_arg(void);
void with_many_args(void);
void with_computed_arg(void);
void with_pointer_chasing(int**** arg);

__attribute__((noinline))
void
no_args(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_NO_ARGS_ENABLED())
#endif
		SDT_NOOP_NO_ARGS();
}

__attribute__((noinline))
int
with_args(void)
{
	int arg1 = 0;
	int arg2 = 1;
	int arg3 = 2;
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_ARGS_ENABLED())
#endif
		SDT_NOOP_WITH_ARGS(arg1, arg2, arg3);

	return arg1 + arg2 + arg3;
}

int some_global;

__attribute__((noinline))
int
with_global_arg(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_GLOBAL_ARG_ENABLED())
#endif
		SDT_NOOP_WITH_GLOBAL_ARG(some_global);

	return some_global;
}

__attribute__((noinline))
int
with_volatile_arg(void)
{
	volatile int arg1;
	arg1 = 42;
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_VOLATILE_ARG_ENABLED())
#endif
		SDT_NOOP_WITH_VOLATILE_ARG(arg1);

	return arg1;
}

__attribute__((noinline))
void
with_many_args(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_MANY_ARGS_ENABLED())
#endif
		SDT_NOOP_WITH_MANY_ARGS(1,2,3,4,5,6,7,8);
}

__attribute__((noinline))
static int
compute_probe_argument(void)
{
	return 100;
}

__attribute__((noinline))
void
with_computed_arg(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_COMPUTED_ARG_ENABLED())
#endif
		SDT_NOOP_WITH_COMPUTED_ARG(compute_probe_argument());
}

__attribute__((noinline))
void
with_pointer_chasing(int**** arg)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_POINTER_CHASING_ENABLED())
#endif
		SDT_NOOP_WITH_POINTER_CHASING(****arg);
}

int
main(int argc, char * argv[] __attribute__((unused)) )
{
	no_args();

	with_args();

	with_many_args();

	some_global = argc;
	with_global_arg();

	with_volatile_arg();

	with_many_args();

	with_computed_arg();

	int *some_value = malloc(sizeof(int));
	*some_value = 0x7f;
	int **some_value_p = &some_value;
	int ***some_value_pp = &some_value_p;
	with_pointer_chasing(&some_value_pp);

	free(some_value);
}
