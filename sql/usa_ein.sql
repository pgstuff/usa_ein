/*
 * Author: The maintainer's name
 * Created at: Wed Oct 14 23:12:59 -0400 2015
 *
 */

SET client_min_messages = warning;

-- SQL definitions for USAEIN type
CREATE TYPE usa_ein;

-- basic i/o functions
CREATE OR REPLACE FUNCTION usa_ein_in(cstring) RETURNS usa_ein AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_out(usa_ein) RETURNS cstring AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_send(usa_ein) RETURNS bytea AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_recv(internal) RETURNS usa_ein AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE usa_ein (
	input = usa_ein_in,
	output = usa_ein_out,
	send = usa_ein_send,
	receive = usa_ein_recv,
	internallength = 4,
	passedbyvalue
);

-- functions to support btree opclass
CREATE OR REPLACE FUNCTION usa_ein_lt(usa_ein, usa_ein) RETURNS bool AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_le(usa_ein, usa_ein) RETURNS bool AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_eq(usa_ein, usa_ein) RETURNS bool AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_ne(usa_ein, usa_ein) RETURNS bool AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_ge(usa_ein, usa_ein) RETURNS bool AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_gt(usa_ein, usa_ein) RETURNS bool AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_cmp(usa_ein, usa_ein) RETURNS int4 AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;

-- to/from text conversion
CREATE OR REPLACE FUNCTION usa_ein_to_text(usa_ein) RETURNS text AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION usa_ein_from_text(text) RETURNS usa_ein AS '$libdir/usa_ein'
LANGUAGE C IMMUTABLE STRICT;

-- operators
CREATE OPERATOR < (
	leftarg = usa_ein, rightarg = usa_ein, procedure = usa_ein_lt,
	commutator = >, negator = >=,
	restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR <= (
	leftarg = usa_ein, rightarg = usa_ein, procedure = usa_ein_le,
	commutator = >=, negator = >,
	restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR = (
	leftarg = usa_ein, rightarg = usa_ein, procedure = usa_ein_eq,
	commutator = =, negator = <>,
	restrict = eqsel, join = eqjoinsel,
	merges
);
CREATE OPERATOR <> (
	leftarg = usa_ein, rightarg = usa_ein, procedure = usa_ein_ne,
	commutator = <>, negator = =,
	restrict = neqsel, join = neqjoinsel
);
CREATE OPERATOR > (
	leftarg = usa_ein, rightarg = usa_ein, procedure = usa_ein_gt,
	commutator = <, negator = <=,
	restrict = scalargtsel, join = scalargtjoinsel
);
CREATE OPERATOR >= (
	leftarg = usa_ein, rightarg = usa_ein, procedure = usa_ein_ge,
	commutator = <=, negator = <,
	restrict = scalargtsel, join = scalargtjoinsel
);

-- aggregates
CREATE OR REPLACE FUNCTION usa_ein_smaller(usa_ein, usa_ein)
RETURNS usa_ein
AS '$libdir/usa_ein'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION usa_ein_larger(usa_ein, usa_ein)
RETURNS usa_ein
AS '$libdir/usa_ein'
    LANGUAGE C IMMUTABLE STRICT;

CREATE AGGREGATE min(usa_ein)  (
    SFUNC = usa_ein_smaller,
    STYPE = usa_ein,
    SORTOP = <
);

CREATE AGGREGATE max(usa_ein)  (
    SFUNC = usa_ein_larger,
    STYPE = usa_ein,
    SORTOP = >
);

-- btree operator class
CREATE OPERATOR CLASS usa_ein_ops DEFAULT FOR TYPE usa_ein USING btree AS
	OPERATOR 1 <,
	OPERATOR 2 <=,
	OPERATOR 3 =,
	OPERATOR 4 >=,
	OPERATOR 5 >,
	FUNCTION 1 usa_ein_cmp(usa_ein, usa_ein);
-- cast from/to text
CREATE CAST (usa_ein AS text) WITH FUNCTION usa_ein_to_text(usa_ein) AS ASSIGNMENT;
CREATE CAST (text AS usa_ein) WITH FUNCTION usa_ein_from_text(text) AS ASSIGNMENT;

/* Does this survive a pg_dump?
CREATE CAST (int       AS usa_ein)   WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (usa_ein   AS int)       WITHOUT FUNCTION;
*/
