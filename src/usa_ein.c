#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"
#include "utils/builtins.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

typedef uint32 usa_ein_t;

#define USAEINGetDatum(x)	 Int32GetDatum(x)
#define DatumGetUSAEIN(x)	 DatumGetInt32(x)
#define PG_GETARG_USAEIN(n) DatumGetUSAEIN(PG_GETARG_DATUM(n))
#define PG_RETURN_USAEIN(x) return USAEINGetDatum(x)

//#define DIGITTOINT(n)	(((n) >= '0' && (n) <= '9') ? (int32) ((n) - '0') : 0)

Datum usa_ein_in(PG_FUNCTION_ARGS);
Datum usa_ein_out(PG_FUNCTION_ARGS);
Datum usa_ein_to_text(PG_FUNCTION_ARGS);
Datum usa_ein_from_text(PG_FUNCTION_ARGS);
Datum usa_ein_send(PG_FUNCTION_ARGS);
Datum usa_ein_recv(PG_FUNCTION_ARGS);
Datum usa_ein_lt(PG_FUNCTION_ARGS);
Datum usa_ein_le(PG_FUNCTION_ARGS);
Datum usa_ein_eq(PG_FUNCTION_ARGS);
Datum usa_ein_ne(PG_FUNCTION_ARGS);
Datum usa_ein_ge(PG_FUNCTION_ARGS);
Datum usa_ein_gt(PG_FUNCTION_ARGS);
Datum usa_ein_cmp(PG_FUNCTION_ARGS);

static usa_ein_t cstring_to_usa_ein(char *usa_ein_string);
static char *usa_ein_to_cstring(usa_ein_t usa_ein);
static bool usa_ein_is_valid(int32 prefix, int32 serial);



/* generic input/output functions */
PG_FUNCTION_INFO_V1(usa_ein_in);
Datum
usa_ein_in(PG_FUNCTION_ARGS)
{
	usa_ein_t	result;

	char   *usa_ein_str = PG_GETARG_CSTRING(0);
	result = cstring_to_usa_ein(usa_ein_str);

	PG_RETURN_USAEIN(result);
}

PG_FUNCTION_INFO_V1(usa_ein_out);
Datum
usa_ein_out(PG_FUNCTION_ARGS)
{
	usa_ein_t	packed_usa_ein;
	char   *usa_ein_string;

	packed_usa_ein = PG_GETARG_USAEIN(0);
	usa_ein_string = usa_ein_to_cstring(packed_usa_ein);

	PG_RETURN_CSTRING(usa_ein_string);
}

/* type to/from text conversion routines */
PG_FUNCTION_INFO_V1(usa_ein_to_text);
Datum
usa_ein_to_text(PG_FUNCTION_ARGS)
{
	char	*usa_ein_string;
	text	*usa_ein_text;

	usa_ein_t	packed_usa_ein =  PG_GETARG_USAEIN(0);

	usa_ein_string = usa_ein_to_cstring(packed_usa_ein);
	usa_ein_text = DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(usa_ein_string)));

	PG_RETURN_TEXT_P(usa_ein_text);
}

PG_FUNCTION_INFO_V1(usa_ein_from_text);
Datum
usa_ein_from_text(PG_FUNCTION_ARGS)
{
	text  *usa_ein_text = PG_GETARG_TEXT_P(0);
	char  *usa_ein_str = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(usa_ein_text)));
	usa_ein_t usa_ein = cstring_to_usa_ein(usa_ein_str);

	PG_RETURN_USAEIN(usa_ein);
}

/* Functions to convert to/from bytea */
PG_FUNCTION_INFO_V1(usa_ein_send);
Datum
usa_ein_send(PG_FUNCTION_ARGS)
{
	StringInfoData buffer;
	usa_ein_t usa_ein = PG_GETARG_USAEIN(0);

	pq_begintypsend(&buffer);
	pq_sendint(&buffer, (int32)usa_ein, 4);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buffer));
}

PG_FUNCTION_INFO_V1(usa_ein_recv);
Datum usa_ein_recv(PG_FUNCTION_ARGS)
{
	usa_ein_t	usa_ein;
	StringInfo	buffer = (StringInfo) PG_GETARG_POINTER(0);

	usa_ein = pq_getmsgint(buffer, 4);
	PG_RETURN_USAEIN(usa_ein);
}

/* functions to support btree opclass */
PG_FUNCTION_INFO_V1(usa_ein_lt);
Datum
usa_ein_lt(PG_FUNCTION_ARGS)
{
	usa_ein_t a = PG_GETARG_USAEIN(0);
	usa_ein_t b = PG_GETARG_USAEIN(1);
	PG_RETURN_BOOL(a < b);
}

PG_FUNCTION_INFO_V1(usa_ein_le);
Datum
usa_ein_le(PG_FUNCTION_ARGS)
{
	usa_ein_t a = PG_GETARG_USAEIN(0);
	usa_ein_t b = PG_GETARG_USAEIN(1);
	PG_RETURN_BOOL (a <= b);
}

PG_FUNCTION_INFO_V1(usa_ein_eq);
Datum
usa_ein_eq(PG_FUNCTION_ARGS)
{
	usa_ein_t a = PG_GETARG_USAEIN(0);
	usa_ein_t b = PG_GETARG_USAEIN(1);
	PG_RETURN_BOOL(a == b);
}

PG_FUNCTION_INFO_V1(usa_ein_ne);
Datum
usa_ein_ne(PG_FUNCTION_ARGS)
{
	usa_ein_t a = PG_GETARG_USAEIN(0);
	usa_ein_t b = PG_GETARG_USAEIN(1);
	PG_RETURN_BOOL(a != b);
}

PG_FUNCTION_INFO_V1(usa_ein_ge);
Datum
usa_ein_ge(PG_FUNCTION_ARGS)
{
	usa_ein_t a = PG_GETARG_USAEIN(0);
	usa_ein_t b = PG_GETARG_USAEIN(1);
	PG_RETURN_BOOL(a >= b);
}

PG_FUNCTION_INFO_V1(usa_ein_gt);
Datum
usa_ein_gt(PG_FUNCTION_ARGS)
{
	usa_ein_t a = PG_GETARG_USAEIN(0);
	usa_ein_t b = PG_GETARG_USAEIN(1);
	PG_RETURN_BOOL(a > b);
}

PG_FUNCTION_INFO_V1(usa_ein_cmp);
Datum
usa_ein_cmp(PG_FUNCTION_ARGS)
{
	usa_ein_t a = PG_GETARG_USAEIN(0);
	usa_ein_t b = PG_GETARG_USAEIN(1);

	PG_RETURN_INT32(a - b);
}

static int32
usa_ein_cmp_internal(usa_ein_t a, usa_ein_t b)
{
    return a - b;

    /*if (a < b)
        return -1;
    else if (a > b)
        return 1;

    return 0;*/
}

/*****************************************************************************
 * Aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(usa_ein_smaller);

Datum
usa_ein_smaller(PG_FUNCTION_ARGS)
{
   usa_ein_t left  = PG_GETARG_USAEIN(0);
   usa_ein_t right = PG_GETARG_USAEIN(1);
   usa_ein_t result;

   result = usa_ein_cmp_internal(left, right) < 0 ? left : right;
   PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(usa_ein_larger);

Datum
usa_ein_larger(PG_FUNCTION_ARGS)
{
   usa_ein_t left  = PG_GETARG_USAEIN(0);
   usa_ein_t right = PG_GETARG_USAEIN(1);
   usa_ein_t result;

   result = usa_ein_cmp_internal(left, right) > 0 ? left : right;
   PG_RETURN_TEXT_P(result);
}


/*
 * Convert a cstring to an USA EIN, validating the input.
 * Input in forms AA-BBBBBBB or AABBBBBBB is accepted.
 */
static usa_ein_t
cstring_to_usa_ein(char *usa_ein_str)
{
    char   *c;
    usa_ein_t   result;
    char    dashes[] = {2};
    int     dashes_no = 0;
    int     ndigits = 0;
    int32   prefix,
            serial;

    prefix = serial = result = 0;

    for (c = usa_ein_str; *c != 0; c++)
    {
        if (*c >= '0' && *c <= '9')
        {
            if (ndigits < 2) prefix = prefix * 10 + *c - '0';
            else if (ndigits < 9) serial = serial * 10 + *c - '0';
            result = result * 10 + *c - '0';
            ndigits++;
        }
        else if (*c == '-')
        {
            int pos = c - usa_ein_str;
            if (dashes_no < 1 && pos == dashes[dashes_no])
                dashes_no++;
            else
                ereport(ERROR,
                        (errmsg("Invalid format of input data \"%s\".", usa_ein_str),
                         errhint("Valid formats are: AA-BBBBBBB or AABBBBBBB")));
        }
        else
            ereport(ERROR,
                    (errmsg("Unexpected character '%c' in input data \"%s\".", *c, usa_ein_str),
                     errhint("Valid USA EIN consists of digits and optional dashes")));

    }
    if (ndigits != 9)
        ereport(ERROR,
                (errmsg("Invalid number of digits (%d) in input data \"%s\".", ndigits, usa_ein_str),
                 errhint("Valid USA EIN consists of 9 digits.")));

    if (!usa_ein_is_valid(prefix, serial))
        ereport(ERROR,
                (errmsg("USA EIN number \"%s\" is invalid.", usa_ein_str)));

    PG_RETURN_USAEIN(result);
}
/*static usa_ein_t
cstring_to_usa_ein(char *usa_ein_str)
{
	char   *c;
	usa_ein_t 	result;
	char 	dashes[] = {2};
	int		dashes_no = 0;
	int		ndigits = 0;
	int32	prefix,
			serial;

	prefix = serial = 0;

	for (c = usa_ein_str; *c != 0; c++)
	{
		if (isdigit(*c))
		{
			if (ndigits < 2) prefix = prefix * 10 + DIGITTOINT(*c);
			else if (ndigits < 9) serial = serial * 10 + DIGITTOINT(*c);
			ndigits++;
		}
		else if (*c == '-')
		{
			int pos = c - usa_ein_str;
			if (dashes_no < 1 && pos == dashes[dashes_no])
				dashes_no++;
			else
				ereport(ERROR,
						(errmsg("invalid format of input data %s", usa_ein_str),
						 errhint("Valid formats are: AA-BBBBBBB or AABBBBBBB")));
		}
		else
			ereport(ERROR,
					(errmsg("unexpected character '%c' in input data %s", *c, usa_ein_str),
					 errhint("Valid USA EIN consists of digits and optional dashes")));

	}
	if (ndigits != 9)
		ereport(ERROR,
				(errmsg("invalid number of digits (%d) in input data %s", ndigits, usa_ein_str),
				 errhint("Valid USA EIN consists of 9 digits")));

	if (!usa_ein_is_valid(prefix, serial))
		ereport(ERROR,
				(errmsg("USA EIN number %s is invalid", usa_ein_str)));

	result = serial + (prefix << 24);
	PG_RETURN_USAEIN(result);

}*/

/* Convert the internal representation to the AA-BBBBBBB output string */
static char *
usa_ein_to_cstring(usa_ein_t usa_ein)
{
    int32   remainder = usa_ein;
    int32   digit_value;
    char   *usa_ein_str = palloc(11);

    if (usa_ein < 0)
        /* Error out */;

    if (usa_ein > 999999999 || usa_ein < 0)
        ereport(ERROR,
                (errmsg("Invalid data."),
                 errhint("The EIN data is out of range.")));

    digit_value = remainder * .00000001;
    usa_ein_str[0] = '0' + digit_value;
    remainder = remainder - digit_value * 100000000;
    digit_value = remainder * .0000001;
    usa_ein_str[1] = '0' + digit_value;
    remainder = remainder - digit_value * 10000000;
    digit_value = remainder * .000001;
    usa_ein_str[2] = '-';
    usa_ein_str[3] = '0' + digit_value;
    remainder = remainder - digit_value * 1000000;
    digit_value = remainder * .00001;
    usa_ein_str[4] = '0' + digit_value;
    remainder = remainder - digit_value * 100000;
    digit_value = remainder * .0001;
    usa_ein_str[5] = '0' + digit_value;
    remainder = remainder - digit_value * 10000;
    digit_value = remainder * .001;
    usa_ein_str[6] = '0' + digit_value;
    remainder = remainder - digit_value * 1000;
    digit_value = remainder * .01;
    usa_ein_str[7] = '0' + digit_value;
    remainder = remainder - digit_value * 100;
    digit_value = remainder * .1;
    usa_ein_str[8] = '0' + digit_value;
    remainder = remainder - digit_value * 10;
    usa_ein_str[9] = '0' + remainder;
    usa_ein_str[10] = '\0';

    return usa_ein_str;
}
/*static char *
usa_ein_to_cstring(usa_ein_t usa_ein)
{
    int32   prefix,
            serial;
    int32   ndigits;
    char   *usa_ein_str = palloc(11);

    if (usa_ein < 0)
        * Error out *;

    serial  = usa_ein & ((1 << 24) - 1);
    prefix = (usa_ein >> 24) & 127;

    if (!usa_ein_is_valid(prefix, serial))
        ereport(ERROR,
                (errmsg("USA EIN number %s is invalid", usa_ein_str)));

    if ((ndigits = snprintf(usa_ein_str, 11, "%02d-%07d", prefix, serial)) != 10)
        ereport(ERROR,
                (errmsg("invalid size (%d) of in input data %s", ndigits-1, usa_ein_str),
                 errhint("Valid USA EIN consists of 9 digits")));

    if (usa_ein_str[2] != '-')
        ereport(ERROR,
                (errmsg("invalid format of input data %s", usa_ein_str),
                 errhint("Valid formats are: AA-BBBBBBB or AABBBBBBB")));


    return usa_ein_str;
}*/

/*
 * Check whether the components of the USA EIN are valid.
 */
static bool
usa_ein_is_valid(int32 prefix, int32 serial)
{
	if (prefix == 0)
		return false;

	if (prefix == 7 || prefix == 8 || prefix == 9)
		return false;

	return true;
}

