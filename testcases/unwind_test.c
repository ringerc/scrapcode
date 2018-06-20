#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FUNCNAME_LENGTH 100

static jmp_buf buf;

void
save_context_jump(unw_context_t *ctx)
{
	if (unw_getcontext(ctx) != 0)
	{
		printf("unw_getcontext() failed\n");
		exit(1);
	}

	longjmp(buf, 1);
}

void
chain2(unw_context_t *ctx)
{
	char dummy_buf[42];
	strncpy(&dummy_buf[0], "life", 42);
	save_context_jump(ctx);
	printf("%s", &dummy_buf[0]);
}

inline static void
chain(unw_context_t *ctx)
{
	volatile int thingy = 4;
	chain2(ctx);
}

void
print_context(unw_context_t *ctx)
{
	unw_cursor_t cursor;
	int frameno = 0;

	if (unw_init_local(&cursor, ctx) != 0)
	{
		printf("unw_init_local failed\n");
		exit(1);
	}

	while (unw_step(&cursor) > 0)
	{
		unw_word_t offp, ip, sp;
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
		char frame_funcname[MAX_FUNCNAME_LENGTH];
		int ret;
		ret = unw_get_proc_name(&cursor, &frame_funcname[0], MAX_FUNCNAME_LENGTH, &offp);
		if (ret == 0)
			printf("\n\tFRAME %4d: %30s +%-4ld ip=0x%lx sp=0x%lx",
				frameno, frame_funcname, (long)offp, (long)ip, (long)sp);
		else
			printf("\n\tFRAME %4d: < ?? > ip=0x%lx sp=0x%lx",
				frameno, (long)ip, (long)sp);
		frameno ++;
	}
	printf("\n");
}

int
main(int argc, char ** argv)
{
	unw_context_t ctx;

	if (setjmp(buf))
	{
		print_context(&ctx);
		exit(0);
	}

	chain(&ctx);
}
