#include "sigjmp_escape_hdr_try.h"
#include <stdio.h>
#include <error.h>

sigjmp_buf * TEST_exception_stack = 0;

test_attribute_noreturn() void
test_re_throw(void)
{
	if (TEST_exception_stack)
		siglongjmp(*TEST_exception_stack, 1);
	else
		error(1, 0, "     no exception stack in test_re_throw()");
}

test_attribute_noreturn() void
do_a_jump(int jumpval)
{
	fprintf(stderr, "     jumping to %d; ", jumpval);
	siglongjmp(*TEST_exception_stack, jumpval);
}
