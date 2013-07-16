#include "postgres.h"
#include "fmgr.h"
#include "utils/date.h"
#include "utils/datetime.h"
#include <stdlib.h>

Datum makedate(PG_FUNCTION_ARGS);

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(makedate);


Datum
makedate(PG_FUNCTION_ARGS)
{
    int yy, mm, dd;
    yy = PG_GETARG_INT32(0);
    mm = PG_GETARG_INT32(1);
    dd = PG_GETARG_INT32(2);
    PG_RETURN_DATEADT(date2j(yy, mm, dd) - POSTGRES_EPOCH_JDATE);
}
