#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include <stdlib.h>

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(array_idx);

Datum array_idx(PG_FUNCTION_ARGS);

/*
 * Find 'findelem' within array 'v'. Returns -1 if not found in an array
 * without any NULL elements, NULL if not found but one or more NULLs appeared
 * in the array.
 *
 * Declared STRICT, must not be called with NULL input(s).
 */
Datum
array_idx(PG_FUNCTION_ARGS)
{
    Datum        findelem = PG_GETARG_DATUM(0);
    ArrayType    *v = PG_GETARG_ARRAYTYPE_P(1);

    Oid          arg0_typeid = get_fn_expr_argtype(fcinfo->flinfo, 0);
    Oid          arg1_typeid = get_fn_expr_argtype(fcinfo->flinfo, 1);
    Oid          arg0_elemid;
    Oid          arg1_elemid;
    Oid          element_type;

    int16        typlen;
    bool         typbyval;
    char         typalign;

    Datum        *elems;
    bool         *nulls;
    int          nelems;

    Datum        cur_elem;
    bool         has_nulls;

    int i;
    

    if (ARR_NDIM(v) == 0)
        PG_RETURN_NULL();
    else if (ARR_NDIM(v) != 1)
        ereport(ERROR,
            (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
             errmsg("idx(...) only makes sense for one-dimensional arrays, not %i", ARR_NDIM(v))));

    element_type = ARR_ELEMTYPE(v);
    /* For optimal performance we'd get this from a syscache or cache it in the function
     * call context; see array_push in src/backend/utils/adt/array_userfuncs.c */
    get_typlenbyvalalign(element_type,
                         &typlen,
                         &typbyval,
                         &typalign);
    
    deconstruct_array(v, element_type, typlen, typbyval, typalign, &elems, &nulls, &nelems);

    has_nulls = false;
    for (i = 0; i < nelems; i++)
    {
        has_nulls |= nulls[i];
        cur_elem = elems[i];
        /* Compare find_elem for equality against cur_elem */
        PG_RETURN_INT(i+1);
    }
   
    if (has_nulls)
        PG_RETURN_NULL();
    else
        PG_RETURN_INT(-1);
}
