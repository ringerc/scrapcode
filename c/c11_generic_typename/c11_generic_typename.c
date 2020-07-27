/*
 * A fun experiment in using C11 _Generic to convert typenames to strings,
 * then use that to auto-generate arrays of type-names to pass to a function
 * along with a variadic argument list.
 *
 * That way the function can be called with many different argument lists
 * and type lists, e.g. for tracing/debug/logging purposes.
 *
 * I'm thinking of adapting the approach for use with systemtap SDTs, to
 * auto-generate data-type markers for the SDT tracepoints and insert them into
 * a suitable .stapsdt ELF section as const-data. Combined with stringifying
 * the argument names, that'd allow SDTs to be largely self-describing.
 *
 * It's not perfect since the _Generic can only understand a concrete list of
 * types. You can't pass any struct of your choice to it. It's also only going
 * to supply typenames, it doesn't appear possible to easily obtain any sort of
 * DWARF Die entry pointer or anything.
 *
 * An alternative might be to post-process the executable after compilation.
 * Generate the executable with -g3 so macro debuginfo is included. Read the
 * stap probes ELF section. Find the defining macros. Enumerate their
 * arguments, argument names (where simple variables) and argument types if
 * possible. Create a new section with the info and insert it into the
 * executable.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#if __STDC_VERSION__ >= 201112L
  #define HAVE_TYPENAME
#else
  #ifndef NO_WARN_MISSING_GENERIC
    #warning no c11 generic support available with STDC_VERSION = __STDC_VERSION__
  #endif
#endif

struct typeformat {
	const char * const name;
	const char * const fmt;
};

static const struct typeformat typeformats[] = {
	{"int", "%d"},
	{"unsigned int", "%u"},
	{"long", "%ld"},
	{"unsigned long", "%lu"},
	{"char", "%c"},
	{"signed char", "%c"},
	{"unsigned char", "%c"},
	{"char *", "%s"},
	{"const signed char", "%c"},
	{"const unsigned char", "%c"},
	{"const char *", "%s"},
	{"signed const char *", "%s"},
	{"unsigned const char *", "%s"},
	{"const void *", "%p"}
};

#ifdef HAVE_TYPENAME
/*
 * Note on _Generic:
 *
 *   C11 only
 *
 *   A unique type may only appear once.
 *
 *   const qualifiers are ignored for the variable type itself, but not for the
 *   target type of pointer types. So these are the same:
 *
 * 	int
 *   	const int
 *
 *   and these are the same:
 *
 *   	char *
 *   	char * const
 *
 *   but these are distinct:
 *
 *   	char *
 *   	const char *
 *
 *   Qualifier order is not significant, so these are the same:
 *
 *   	signed const char *
 *   	const signed char *
 *
 */

/* Map an expression to a type name */
#define TYPENAME(x) _Generic((x), \
	int: "int", \
	unsigned int: "unsigned int", \
	long: "long", \
	unsigned long: "unsigned long", \
	char: "char", \
	signed char: "signed char", \
	unsigned char: "unsigned char", \
	char *: "const char *", \
	signed char *: "const signed char *", \
	unsigned char *: "const unsigned char *", \
	const char *: "const char *", \
	const signed char *: "const signed char *", \
	const unsigned char *: "const unsigned char *", \
	void *: "void *", \
	const void *: "void *", \
	default: ("TYPENAME(" #x "): ERROR unhandled type") \
	)
#else
#define TYPENAME(x) ""
#endif

#define Expand1(x) x
#define Expand(x) Expand1(x)

/* Create the __auto_type declaration list */
#define _UNTYPED_ARG(x,n)   const __auto_type arg ## n __attribute__((unused)) = x;
#define UNTYPED_ARG1(x)      _UNTYPED_ARG((x), 1)
#define UNTYPED_ARG2(x,y)    UNTYPED_ARG1(y); _UNTYPED_ARG((x),2)
#define UNTYPED_ARG3(x,...)  UNTYPED_ARG2(__VA_ARGS__); _UNTYPED_ARG((x),3)
#define UNTYPED_ARG4(x,...)  UNTYPED_ARG3(__VA_ARGS__); _UNTYPED_ARG((x),4)
#define UNTYPED_ARG5(x,...)  UNTYPED_ARG4(__VA_ARGS__); _UNTYPED_ARG((x),5)
#define UNTYPED_ARG6(x,...)  UNTYPED_ARG5(__VA_ARGS__); _UNTYPED_ARG((x),6)
#define UNTYPED_ARG7(x,...)  UNTYPED_ARG6(__VA_ARGS__); _UNTYPED_ARG((x),7)
#define UNTYPED_ARG8(x,...)  UNTYPED_ARG7(__VA_ARGS__); _UNTYPED_ARG((x),8)
#define UNTYPED_ARG9(x,...)  UNTYPED_ARG8(__VA_ARGS__); _UNTYPED_ARG((x),9)
#define UNTYPED_ARG10(x,...) UNTYPED_ARG9(__VA_ARGS__); _UNTYPED_ARG((x),10)

#define ARGTYPEVAR(n) argtype ## n

/*
 * Create __auto_type declaration list with argtypeN constant definitions
 * containing string data type for each argN variable.
 */
#define _TYPED_ARG(x,n) \
		_UNTYPED_ARG(x,n); \
		const char * const Expand1(ARGTYPEVAR(n)) __attribute__((unused)) = TYPENAME(arg ## n)
#define TYPED_ARG1(x)      _TYPED_ARG((x), 1)
#define TYPED_ARG2(x,y)    TYPED_ARG1(y); _TYPED_ARG((x),2)
#define TYPED_ARG3(x,...)  TYPED_ARG2(__VA_ARGS__); _TYPED_ARG((x),3)
#define TYPED_ARG4(x,...)  TYPED_ARG3(__VA_ARGS__); _TYPED_ARG((x),4)
#define TYPED_ARG5(x,...)  TYPED_ARG4(__VA_ARGS__); _TYPED_ARG((x),5)
#define TYPED_ARG6(x,...)  TYPED_ARG5(__VA_ARGS__); _TYPED_ARG((x),6)
#define TYPED_ARG7(x,...)  TYPED_ARG6(__VA_ARGS__); _TYPED_ARG((x),7)
#define TYPED_ARG8(x,...)  TYPED_ARG7(__VA_ARGS__); _TYPED_ARG((x),8)
#define TYPED_ARG9(x,...)  TYPED_ARG8(__VA_ARGS__); _TYPED_ARG((x),9)
#define TYPED_ARG10(x,...) TYPED_ARG9(__VA_ARGS__); _TYPED_ARG((x),10)

/*
 * List of generated argument names from TYPED_ARGn.
 *
 * These are reversed on purpose.
 */
#define ARGNAMES1 arg1
#define ARGNAMES2 arg2, ARGNAMES1
#define ARGNAMES3 arg3, ARGNAMES2
#define ARGNAMES4 arg4, ARGNAMES3
#define ARGNAMES5 arg5, ARGNAMES4
#define ARGNAMES6 arg6, ARGNAMES5
#define ARGNAMES7 arg7, ARGNAMES6
#define ARGNAMES8 arg8, ARGNAMES7
#define ARGNAMES9 arg9, ARGNAMES8
#define ARGNAMES10 arg10, ARGNAMES9

/*
 * List of generated argument type-variable names from TYPED_ARGn.
 *
 * These are reversed on purpose.
 */
#define ARGTYPEVARS1 ARGTYPEVAR(1)
#define ARGTYPEVARS2 ARGTYPEVAR(2), ARGTYPEVARS1
#define ARGTYPEVARS3 ARGTYPEVAR(3), ARGTYPEVARS2
#define ARGTYPEVARS4 ARGTYPEVAR(4), ARGTYPEVARS3
#define ARGTYPEVARS5 ARGTYPEVAR(5), ARGTYPEVARS4
#define ARGTYPEVARS6 ARGTYPEVAR(6), ARGTYPEVARS5
#define ARGTYPEVARS7 ARGTYPEVAR(7), ARGTYPEVARS6
#define ARGTYPEVARS8 ARGTYPEVAR(8), ARGTYPEVARS7
#define ARGTYPEVARS9 ARGTYPEVAR(9), ARGTYPEVARS8
#define ARGTYPEVARS10 ARGTYPEVAR(10), ARGTYPEVARS9

/* Create the __auto_type declaration list, and a static array of type names called _types */
#define _WRAPCALL_TYPES(n)  const char * const _types[n+1] __attribute__((unused)) = { Expand1(ARGTYPEVARS ## n), 0 }
#define WRAPCALL_TYPES1(...)  TYPED_ARG1(__VA_ARGS__);  _WRAPCALL_TYPES(1)
#define WRAPCALL_TYPES2(...)  TYPED_ARG2(__VA_ARGS__);  _WRAPCALL_TYPES(2)
#define WRAPCALL_TYPES3(...)  TYPED_ARG3(__VA_ARGS__);  _WRAPCALL_TYPES(3)
#define WRAPCALL_TYPES4(...)  TYPED_ARG4(__VA_ARGS__);  _WRAPCALL_TYPES(4)
#define WRAPCALL_TYPES5(...)  TYPED_ARG5(__VA_ARGS__);  _WRAPCALL_TYPES(5)
#define WRAPCALL_TYPES6(...)  TYPED_ARG6(__VA_ARGS__);  _WRAPCALL_TYPES(6)
#define WRAPCALL_TYPES7(...)  TYPED_ARG7(__VA_ARGS__);  _WRAPCALL_TYPES(7)
#define WRAPCALL_TYPES8(...)  TYPED_ARG8(__VA_ARGS__);  _WRAPCALL_TYPES(8)
#define WRAPCALL_TYPES9(...)  TYPED_ARG9(__VA_ARGS__);  _WRAPCALL_TYPES(9)
#define WRAPCALL_TYPES10(...) TYPED_ARG10(__VA_ARGS__); _WRAPCALL_TYPES(10)

#define WRAPCALL4(callfunc,...) do { WRAPCALL_TYPES4(__VA_ARGS__); callfunc(_types, 4, ARGNAMES4); } while (0)
#define WRAPCALL4_TYPES(callfunc,_types,...) do { UNTYPED_ARG4(__VA_ARGS__); callfunc(_types, 4, ARGNAMES4); } while (0)

#ifdef HAVE_TYPENAME
#define CHECK_ARGTYPES4(expected_argtypes,...) do { WRAPCALL_TYPES4(__VA_ARGS__); compare_argtypes(expected_argtypes, _types, 4); } while (0)
#else
#define CHECK_ARGTYPES4(expected_argtypes,...)
#endif


/*
 * Call with
 *
 * 	int foo;
 * 	char * bar;
 * 	const char * const argtypes = { "int", "char *" };
 * 	CHECK_ARGTYPES2(argtypes, foo, bar);
 *
 * The argtypes array is not counted.
 */ 	
void
compare_argtypes(const char * const * const explicit_types, const char * const * const detected_types, int nargs)
{
	int i = 0;
	const char * et;
	const char * dt;
	int error = 0;

	fputs("detected typenames:\n", stderr);
	while ((et = explicit_types[i]) && (dt = detected_types[i]))
	{
		if (i >= nargs)
			fprintf(stderr, "ERROR: wrong arg count %d\n", nargs);

		if (!dt != !et)
		{
			fprintf(stderr, "    ERROR: detected_types[%d] and explicit_types[%d] end-marker mismatch: %p vs %p\n",
					i, i, dt, et);
			error++;
		}
		else if (strcmp(dt, et) != 0)
		{
			fprintf(stderr, "    ERROR: detected_types[%d] and explicit_types[%d] differ: \"%s\" vs \"%s\"\n",
					i, i, dt, et);
			error++;
		}
		else
		{
			fprintf(stderr, "    %s\n", dt);
		}

		i++;
	} while (et && dt);

	if (error)
		exit(1);
}

void
format_args(const char * const * const types, int nargs, ...)
{
	va_list vl;
	int argno;
	const char *typename;
	char formatstr[200];
	char *format_next = &formatstr[0];

	memset(&formatstr[0], '\0', 200);
	for (argno = 0, typename = types[argno]; typename != 0; typename = types[++argno])
	{
		if (argno > 0) {
			strcpy(format_next, ", ");
			format_next += 2;
		}
		if (typename[0] == '\0') {
			/* No type provided */
			strcpy(format_next, "?");
			format_next += 1;
		}
		else if (strcmp(typename, "int") == 0) {
			strcpy(format_next, "%d");
			format_next += 2;
		} else if (strcmp(typename, "char *") == 0 || strcmp(typename, "const char *") == 0) {
			strcpy(format_next, "%s");
			format_next += 2;
		} else if (strcmp(typename, "void *") == 0) {
			strcpy(format_next, "%p");
			format_next += 2;
		} else {
			fprintf(stderr, "ERROR: format_char(): unhandled argtype \"%s\" at types[%d]\n", typename, argno);
			exit(1);
		}
		assert(argno <= nargs);
	}
	assert(argno == nargs);
	strcpy(format_next, "\n");

	fprintf(stderr, "format string is %s", formatstr);

	va_start(vl, types);
	vprintf(&formatstr[0], vl);
	va_end(vl);
}

int main(void) {
	int a = 10;
	const char *b = "foo";
	const int c = 9;
	void *d = (void*)0xDEADBEEFL;
	/* expected_argtypes should ignore const qualifiers */
	const char * expected_argtypes[5] = {"int", "const char *", "int", "void *", 0};
#ifdef HAVE_TYPENAME
	static const int using_typeof = 1;
#else
	static const int using_typeof = 0;
#endif

	fprintf(stderr, "------- C standard: %ld, using typeof: %d ---------\n",
			(long int)(__STDC_VERSION__), using_typeof);

	/* Compare autodetected types to manually listed types */
	CHECK_ARGTYPES4(expected_argtypes, a, b, c, d);
	/* Use autodetected types if supported */
	WRAPCALL4(format_args, a, b, c, d);
	/* Pass with manually supplied type strings */
	WRAPCALL4_TYPES(format_args, expected_argtypes, a, b, c, d);

	return 0;
}
