#include <stdint.h>
#include <assert.h>

struct guard
{
	int8_t guard_set;
};

extern struct guard * guard_ptr;

static void
set_guard(struct guard * const g)
{
	assert(!g->guard_set);
	assert(!guard_ptr);
	g->guard_set = 1;
	guard_ptr = g;
}

static inline void
clear_guard(struct guard * const g)
{
	assert(g->guard_set);
	assert(guard_ptr);
	g->guard_set = 0;
	guard_ptr = 0;
}
