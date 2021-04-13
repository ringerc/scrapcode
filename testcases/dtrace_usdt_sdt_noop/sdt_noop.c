#include <stdint.h>

#ifdef USE_SDT
#include "sdt_noop_probes_enabled.h"
#else
#include "sdt_noop_probes_disabled.h"
#endif

void no_args(void);
void with_args(void);
void with_global_arg(void);
void with_volatile_arg(void);
void with_many_args(void);
void with_computed_arg(void);

void
no_args(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_NO_ARGS_ENABLED())
#endif
		SDT_NOOP_NO_ARGS();
}

void
with_args(void)
{
	int arg1 = 0;
	int arg2 = 1;
	int arg3 = 2;
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_ARGS_ENABLED())
#endif
	SDT_NOOP_WITH_ARGS(arg1, arg2, arg3);
}

int some_global;

void
with_global_arg(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_GLOBAL_ARG_ENABLED())
#endif
		SDT_NOOP_WITH_GLOBAL_ARG(some_global);
}

void
with_volatile_arg(void)
{
	volatile int arg1;
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_VOLATILE_ARG_ENABLED())
#endif
		SDT_NOOP_WITH_VOLATILE_ARG(arg1);
}

void
with_many_args(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_MANY_ARGS_ENABLED())
#endif
		SDT_NOOP_WITH_MANY_ARGS(1,2,3,4,5,6,7,8);
}

 __attribute__((noinline)) __attribute__((optimize("-O0")))
static int
compute_probe_argument(void)
{
	return 100;
}

void
with_computed_arg(void)
{
#ifdef USE_SDT_SEMAPHORES
	if (SDT_NOOP_WITH_COMPUTED_ARG_ENABLED())
#endif
		SDT_NOOP_WITH_COMPUTED_ARG(compute_probe_argument());
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
}
