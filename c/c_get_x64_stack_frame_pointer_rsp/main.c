/*
 * This demo program seeks to demonstrate how saving the stack
 * pointer before a longjmp() makes it possible to debug a program
 * after it returns from the longjmp(), assuming there
 * was no overwrite of the stack.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>

static sigjmp_buf b;
static void * last_jumped_from;

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
static long long int __attribute__((noinline))
getcaller(void)
{
	unsigned long long int current;
	asm volatile ( "movq %%rsp, %0\n" : "=rm" (current) );
	return current;
}
*/

/*
 * Save current stack pointer %rsp to a global, and also print it.
 * Doesn't jump, it's tryign to get the frame for the caller.
 */
static void
getframe(int arg, const char * const msg, int dojump)
{
	int some_local = rand();
	/*
	unsigned long long int current;
	asm volatile ( "movq %%rsp, %0\n" : "=rm" (current) );
	*/

	/*
	 * See https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
	 *
	 * You can get the frame in gdb with
	 *
	 *     frame view last_jumped_from+16 getframe
	 *
	 *     frame view last_jumped_from-16 getframe
	 *
	 * once this returns! Woah.
	 *
	 * However, it doesn't get a backtrace from there.
	 */
	void * current;
	current = __builtin_frame_address(0);

	last_jumped_from = current;
	fprintf(stderr, "frame = 0x%p\n", current);
	fprintf(stdout, "arg   = %d\n", arg);
	fprintf(stdout, "rand  = %d\n", some_local);
	fprintf(stdout, "msg   = %s\n", msg);
	callee(dojump);
}

/*
 * Stack padding to simulate a callstack before the getframe().
 *
 * In postgres errfinish() will be called with a much deeper stack
 * than the handler function that consumes the result so the upper
 * parts of the popped stack are likely to be intact. Simulate this.
 * Don't allow the compiler to elide the stack padding var.
 */
static void
call_getframe(int arg, const char * const msg, int dojump)
{
	volatile char stackpadding[500];
	memset((char *)&stackpadding[0], '\x7f', 500);
	getframe(arg, msg, dojump);
}

static void
caller(int arg, const char * const msg, int dojump)
{
	if (sigsetjmp(b, 0) != 0) {
		/* jumped */
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
	caller(atoi(argv[2]), argv[1], atoi(argv[3]));
}
