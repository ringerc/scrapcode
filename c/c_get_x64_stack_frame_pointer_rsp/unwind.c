/*
 * This demo program seeks to demonstrate how libunwind can be used
 * to save the stack before a longjmp() then used to unwind the original
 * stack after the longjmp(), at least up to the point it got overwritten.
 *
 * Want to show how to do so in gdb though.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

static sigjmp_buf b;
static unw_context_t last_jumped_from;
static void * last_jumped_from_gcc;
static void * last_jumped_from_unwind;


static void
printstack(unw_context_t * ctx)
{
	unw_cursor_t curs;
	unw_word_t ip, sp;

	fprintf(stderr, "--begin trace--\n");
	unw_init_local(&curs, ctx);
	while (unw_step(&curs) > 0) {
		unw_get_reg(&curs, UNW_REG_IP, &ip);
		unw_get_reg(&curs, UNW_REG_SP, &sp);
		fprintf (stderr, "    ip = %lx, sp = %lx\n", (long) ip, (long) sp);
	}
	fprintf(stderr, "--end trace--\n");
}

static void
crash(void)
{
	char padding[20] = "foo bar baz";
	fprintf(stdout, "I'm going to crash: %s\n", padding);
	abort();
}

/*
 * longjmp() to caller() from getframe()
 */
static void
callee(int dojump)
{
	if (dojump)
		longjmp(b, 1);
	else
		abort();
}

/*
 * Save current unwind state to a global and print before jump
 * stack.
 *
 * Doesn't jump, it's trying to get the frame for the caller.
 */
static void
getframe(int arg, const char * const msg, int dojump)
{
	void * current;
	int some_local = rand();
	unw_context_t ctx;
	unw_cursor_t curs;
	unw_word_t ip, sp;

	current = __builtin_frame_address(0);

	unw_getcontext(&ctx);
	//printstack(&ctx);

	memcpy(&last_jumped_from, &ctx, sizeof(unw_context_t));
	//printstack(&last_jumped_from);

	unw_init_local(&curs, &ctx);
	unw_get_reg(&curs, UNW_REG_IP, &ip);
	unw_get_reg(&curs, UNW_REG_SP, &sp);
	last_jumped_from_unwind = (void*) sp;

	fprintf(stderr, "rsp   = 0x%016lx\n", sp);
	fprintf(stderr, "rip   = 0x%016lx\n", ip);
	fprintf(stdout, "arg   = %d\n", arg);
	fprintf(stdout, "rand  = %d\n", some_local);
	fprintf(stdout, "msg   = %s\n", msg);

	{
		fprintf(stderr, "gccsp = 0x%016lx\n", (long int)current);
		last_jumped_from_gcc = current;
	}


	callee(dojump);
}

/*
 * Stack padding to simulate a callstack before the getframe().
 *
 * In postgres errfinish() will be called with a much deeper stack
 * than the handler function that consumes the result so the upper
 * parts of the popped stack are likely to be intact. Simulate this.
 * Don't allow the compiler to elide the stack padding var.
 *
 * The bitwise negation of arg is so that the only function that
 * has the un-negated arg in their sig is getframe. So it can be
 * used with the searchstack script.
 */
static void
call_getframe(int arg, const char * const msg, int dojump)
{
	volatile char stackpadding[1000];
	memset(stackpadding, '\x7f', sizeof(stackpadding));
	getframe(~ arg, msg, dojump);
}

static void
caller(int arg, const char * const msg, int dojump)
{
	if (sigsetjmp(b, 0) != 0) {
		/* jumped */
		/* Print the stack. Warning, this will clobber
		 * residual state on the old stack. */
		//printstack(&last_jumped_from);
		crash();
	} else {
		call_getframe(arg, msg, dojump);
	}
}

int main(int argc, const char * const argv[])
{
	if (argc != 4) {
		fprintf(stderr, "usage: %s <<int>> <<msg>> <<dojump>>\n", argv[0]);
		exit(1);
	}
	fprintf(stderr, "sizeof(unw_context_t) = %d\n", sizeof(unw_context_t));
	unw_set_caching_policy(unw_local_addr_space, UNW_CACHE_NONE);
	caller(~ atoi(argv[2]), argv[1], atoi(argv[3]));
}
