#include "postgres.h"
#include "fmgr.h"
#include "utils/bytea.h"
#include "utils/uuid.h"
#include "utils/builtins.h"

#include <stdlib.h>

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(bytea_to_uuid);

Datum
bytea_to_uuid(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(uuid_to_bytea);

Datum
uuid_to_bytea(PG_FUNCTION_ARGS);

/*
 * Grabbed from varlena.c. This is a private struct, and we shouldn't really be
 * using it. See comments in bytea_to_uuid for why it's necessary.
 */
struct pg_uuid_t
{
	    unsigned char data[UUID_LEN];
};

Datum
bytea_to_uuid(PG_FUNCTION_ARGS)
{
	bytea	   *inbytea;
	pg_uuid_t  *uuid;

	inbytea = PG_DETOAST_DATUM(PG_GETARG_BYTEA_P(0));
	if (VARSIZE_ANY_EXHDR(inbytea) != UUID_LEN)
	{
		ereport(ERROR,
				(errcode(ERRCODE_STRING_DATA_LENGTH_MISMATCH),
				 errmsg("Input of %d bytes was not exactly 16 bytes (128 bits) in size", VARSIZE_ANY_EXHDR(inbytea) )));
	}

	/*
	 * This is a bit dirty. Really, uuid.h should expose uuid to/from binary
	 * functions. As it is, we happen to know that the definition is simply
	 * a 16-byte array, so we're going to brute-force it.
	 *
	 * To be strictly correct we'd convert to a uuid string, then use uuid_in,
	 * but that's slow and I'm lazy today.
	 *
	 * If you use this in production, I'll hurt you.
	 */
	uuid = (pg_uuid_t*)palloc(UUID_LEN);
	memcpy(uuid, VARDATA_ANY(inbytea), UUID_LEN);
	
	PG_RETURN_UUID_P(uuid);
}

/**
 * Assumes that uuid is a simple 16-byte struct.
 *
 * If you use this in production you deserve whatever happens to you.
 */
Datum
uuid_to_bytea(PG_FUNCTION_ARGS)
{
	bytea	   *outbytea;
	pg_uuid_t  *uuid;

	/* uuid isn't TOASTAble */
	uuid = PG_GETARG_UUID_P(0);

	outbytea = (bytea*)palloc(VARHDRSZ + UUID_LEN);
    SET_VARSIZE(outbytea, VARHDRSZ + UUID_LEN);
	memcpy(VARDATA(outbytea), (void*)uuid, UUID_LEN);

    PG_RETURN_BYTEA_P(outbytea);
}
