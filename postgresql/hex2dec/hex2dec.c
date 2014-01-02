#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "errno.h"
#include "limits.h"
#include <stdlib.h>

PG_MODULE_MAGIC;

Datum from_hex(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(hex2dec);

Datum
hex2dec(PG_FUNCTION_ARGS)
{ 
	char *endpos;
	const char *hexstr = text_to_cstring(PG_GETARG_TEXT_PP(0));
	long decval = strtol(hexstr, &endpos, 16);
	if (endpos[0] != '\0')
	{
		ereport(ERROR, (ERRCODE_INVALID_PARAMETER_VALUE, errmsg("Could not decode input string %s as hex", hexstr)));
	}
	if (decval == LONG_MAX && errno == ERANGE)
	{
		ereport(ERROR, (ERRCODE_NUMERIC_VALUE_OUT_OF_RANGE, errmsg("Input hex string %s overflows int64", hexstr)));
	}
	PG_RETURN_INT64(decval);
}
