/*
 * Hide sigsetjmp() in a macro in a header to see if clang scan-build
 * is missing it because of that somehow.
 */

#include <setjmp.h>

#ifndef __have_attribute
#define __have_attribute(attname) 0
#endif

#if __have_attribute(noreturn)
#define test_attribute_noreturn() __attribute__((noreturn))
#else
#define test_attribute_noreturn()
#endif

extern test_attribute_noreturn() void test_re_throw(void);

#define TEST_RE_THROW() test_re_throw()

extern sigjmp_buf * TEST_exception_stack;

#define TEST_TRY()  \
	do { \
		sigjmp_buf *_save_exception_stack = TEST_exception_stack; \
		sigjmp_buf _local_sigjmp_buf; \
		int _do_rethrow = 0; \
		if (sigsetjmp(_local_sigjmp_buf, 0) == 0) \
		{ \
			TEST_exception_stack = &_local_sigjmp_buf

#define TEST_CATCH()	\
		} \
		else \
		{ \
			TEST_exception_stack = _save_exception_stack; \

#define TEST_FINALLY()	\
		} \
		else \
			_do_rethrow = 1; \
		{ \
			TEST_exception_stack = _save_exception_stack

#define TEST_END_TRY()  \
		} \
		if (_do_rethrow) \
				TEST_RE_THROW(); \
		TEST_exception_stack = _save_exception_stack; \
	} while (0)

extern test_attribute_noreturn() void do_a_jump(int jumpval);
