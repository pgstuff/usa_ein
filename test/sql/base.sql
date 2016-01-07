\set ECHO none
\i sql/usa_ein.sql
\set ECHO all

CREATE TABLE usa_eins(id serial primary key, usa_ein usa_ein unique);
INSERT INTO usa_eins(usa_ein) VALUES('55-5555555');
INSERT INTO usa_eins(usa_ein) VALUES('01-2345678');
-- check for alternative input format
INSERT INTO usa_eins(usa_ein) VALUES('999999999');
-- check for invalid SSNs
INSERT INTO usa_eins(usa_ein) VALUES('07-1111111');
INSERT INTO usa_eins(usa_ein) VALUES('012-345678');
INSERT INTO usa_eins(usa_ein) VALUES('01-2345678 ');


SELECT * FROM usa_eins ORDER BY usa_ein;

SELECT id,usa_ein::text FROM usa_eins WHERE usa_ein = '55-5555555';

-- index scan
TRUNCATE usa_eins;
INSERT INTO usa_eins(usa_ein) SELECT '01-'||id FROM generate_series(1234500, 1234599) id;
--INSERT INTO usa_eins(usa_ein) SELECT '01-'||id FROM generate_series(1000000, 9999999) id;

SET enable_seqscan = false;
SELECT id,usa_ein::text FROM usa_eins WHERE usa_ein = '01-1234567';
SELECT id,usa_ein FROM usa_eins WHERE usa_ein >= '01-1234567' LIMIT 5;
SELECT count(id) FROM usa_eins;
SELECT count(id) FROM usa_eins WHERE usa_ein <> ('01-1234567'::text)::usa_ein;
RESET enable_seqscan;

-- operators and conversions
SELECT '01-1234567'::usa_ein < '01-1234567'::usa_ein;
SELECT '01-1234567'::usa_ein > '01-1234567'::usa_ein;
SELECT '01-1234567'::usa_ein < '01-1234568'::usa_ein;
SELECT '01-1234567'::usa_ein > '01-1234568'::usa_ein;
SELECT '01-1234567'::usa_ein <= '01-1234567'::usa_ein;
SELECT '01-1234567'::usa_ein >= '01-1234567'::usa_ein;
SELECT '01-1234567'::usa_ein <= '01-1234568'::usa_ein;
SELECT '01-1234567'::usa_ein >= '01-1234568'::usa_ein;
SELECT '01-1234567'::usa_ein <> '01-1234567'::usa_ein;
SELECT '01-1234567'::usa_ein <> '01-1234568'::usa_ein;
SELECT '01-1234567'::usa_ein = '01-1234567'::usa_ein;
SELECT '01-1234567'::usa_ein = '01-1234568'::usa_ein;

-- test limits
SELECT '01-0000000'::usa_ein;
SELECT '99-0000000'::usa_ein;
SELECT '01-9999999'::usa_ein;
SELECT '99-9999999'::usa_ein;

-- COPY FROM/TO
TRUNCATE usa_eins;
COPY usa_eins(usa_ein) FROM STDIN;
55-5555555
999999999
\.
COPY usa_eins TO STDOUT;

-- clean up --
DROP TABLE usa_eins;
