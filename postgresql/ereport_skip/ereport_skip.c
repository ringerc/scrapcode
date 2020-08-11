#include "postgres.h"

#include "commands/seclabel.h"
#include "fmgr.h"
#include "miscadmin.h"
#include "utils/guc.h"

PG_MODULE_MAGIC;

static const char *
expensive_function(void)
{
	pg_usleep(2000000L);
	return "expensive";
}

PG_FUNCTION_INFO_V1(ereport_skip_test);

Datum
ereport_skip_test(PG_FUNCTION_ARGS)
{
	/* Constant expression */
	ereport(DEBUG1,
		(errmsg("%s", expensive_function())));
	PG_RETURN_VOID();
}

PG_FUNCTION_INFO_V1(ereport_skip_test2);

Datum
ereport_skip_test2(PG_FUNCTION_ARGS)
{
	/* Non-constant expression */
	ereport(client_min_messages,
		(errmsg("%s", expensive_function())));
	PG_RETURN_VOID();
}
