#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(replc);
Datum replc(PG_FUNCTION_ARGS);

PGDLLEXPORT Datum
replc(PG_FUNCTION_ARGS)
{
	/* Set `buf` to a palloc'd copy of the input string, deTOASTed if needed */
	char * const buf = text_to_cstring(PG_GETARG_TEXT_PP(0));
	char * ch = buf;
	int depth = 0;


	while (*ch != '\0')
	{
		switch (*ch)
		{
			case '[':
				depth++;
				if (depth > 2)
					*ch = '{';
				break;
			case ']':
				if (depth > 2)
					*ch = '}';
				depth--;
				break;
		}
		ch++;
	}
	if (depth != 0)
		ereport(WARNING,
				(errmsg("Opening and closing []s did not match, got %d extra [s", depth)));

	PG_RETURN_DATUM(CStringGetTextDatum(buf));
}
