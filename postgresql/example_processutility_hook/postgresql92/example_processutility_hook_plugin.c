#include "postgres.h"
#include "fmgr.h"
#include "tcop/utility.h"
#include "commands/dbcommands.h"
#include "miscadmin.h"
#include "utils/guc.h"

/*
 * example_processutility_hook.c: The Pg plugin interface glue to hook ProcessUtility hook, delegate
 * to the next hook if there is one, etc.
 *
 * This function is for PostgreSQL 9.2. 9.3 has a somewhat different processutility signature.
 */

PG_MODULE_MAGIC;

static ProcessUtility_hook_type next_ProcessUtility_hook = NULL;

static void
wal_log_ddltext(Node *parsetree,
                const char *queryString,
                ParamListInfo params,
                bool isTopLevel,
                DestReceiver *dest,
                char *completionTag)
{

	ereport(DEBUG4, (errmsg_internal("example_processutility_hook ProcessUtility_hook invoked")));


	switch (nodeTag(parsetree)) {
		case T_DropStmt:
		case T_CreateStmt:
			ereport(INFO, (errmsg_internal("Drop or create on database '%s' with search_path '%s', user '%s': %s", get_database_name(MyDatabaseId), GetConfigOption("search_path",false,false), GetUserNameFromId(GetUserId()), queryString)));
			break;
		default:
			break;
	}

	if (next_ProcessUtility_hook)
	{
		ereport(DEBUG4, (errmsg_internal("example_processutility_hook ProcessUtility_hook handing off to next hook ")));
		(*next_ProcessUtility_hook) (parsetree, queryString, params,
					isTopLevel, dest, completionTag);
	}
	else
	{
		ereport(DEBUG4, (errmsg_internal("example_processutility_hook ProcessUtility_hook invoking standard_ProcessUtility")));
		standard_ProcessUtility(parsetree, queryString, params,
					isTopLevel, dest, completionTag);
	}
}

void _PG_init(void);

/* Module load */
void
_PG_init(void)
{
	ereport(DEBUG4, (errcode(ERRCODE_SUCCESSFUL_COMPLETION), errmsg_internal("example_processutility_hook ProcessUtility_hook installed")));
        next_ProcessUtility_hook = ProcessUtility_hook;
        ProcessUtility_hook = wal_log_ddltext;
}
