#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define GUARD_NAME_MAX_LENGTH 20

#ifndef NO_TRACE_GUARDS
#define TRACE_GUARDS
#endif

struct guard
{
	int8_t guard_set;
	const char gname[GUARD_NAME_MAX_LENGTH];
	struct guard * previous;
};

extern struct guard * guard_ptr;

static inline void
set_guard(struct guard * const g)
{
	assert(!g->guard_set);
	g->guard_set = 1;
	g->previous = guard_ptr;
	guard_ptr = g;
#ifdef TRACE_GUARDS
	fprintf(stderr, "    set_guard(%p=%s) pushed previous %p=%s\n",
			g, g->gname, g->previous,
			g->previous ? g->previous->gname : "(nil)");
#endif
}

static inline void
clear_guard(struct guard * const g)
{
#ifdef TRACE_GUARDS
	fprintf(stderr, "    clear_guard(%p=%s) restoring previous %p=%s\n",
			g, g->gname, g->previous,
			g->previous ? g->previous->gname : "(nil)");
#endif
	assert(g->guard_set);
	assert(guard_ptr);
	assert(guard_ptr == g);
	g->guard_set = 0;
	guard_ptr = guard_ptr->previous;
}

extern int check_guard(void);

#ifndef __has_attribute
#define __has_attribute(attno) 0
#endif

#if __has_attribute(unused)
#define attr_unused() __attribute__((unused))
#else
#define attr_unused()
#endif
