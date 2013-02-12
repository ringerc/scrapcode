#include "postgres.h"
#include "fmgr.h"
#include <stdlib.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(random_bytea);

Datum
random_bytea(PG_FUNCTION_ARGS)
{
    int32     numbytes;
    bytea *   randbytea;
    char *    cur_data_ptr;
    long int  lastrand;
    long int  remaining;
    int n;

    numbytes = PG_GETARG_INT32(0);
    randbytea = palloc(VARHDRSZ + numbytes);
    SET_VARSIZE(randbytea, VARHDRSZ + numbytes);
    cur_data_ptr = VARDATA(randbytea);

    /*
     * You can't just memcpy() the random
     * value as it only varies between 0 and RAND_MAX
     * not over the full value of the type. Instead
     * we copy the random data byte-by-byte until
     * there's less than a full random byte in the
     * value.
     */
    remaining = 0;
    while ( (numbytes--) > 0) {
         remaining = remaining >> 8;
         lastrand = lastrand >> 8;
         if (remaining < 256) {
            lastrand = random();
            remaining = RAND_MAX;
         }
         *(cur_data_ptr++) = (unsigned char) lastrand;
    }

    PG_RETURN_BYTEA_P(randbytea);
}
