#include "postgres.h"

#include "fmgr.h"
#include "miscadmin.h"
#include "access/parallel.h"
#include "access/xact.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(parallel_unsafe);

Datum
parallel_unsafe(PG_FUNCTION_ARGS)
{
	int arg = PG_GETARG_INT32(0);
	TransactionId my_xid;

	/* You can't assign an xid in a parallel func, it's not allowed */
	my_xid = GetTopTransactionId();

	ereport(NOTICE,
			(errmsg("pid %u, xid %u, parallel worker=%u", MyProcPid, my_xid, IsParallelWorker())));

	PG_RETURN_INT32(arg);
}
