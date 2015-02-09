CREATE TYPE stablo;

CREATE OR REPLACE FUNCTION stablo_in(cstring) RETURNS stablo AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_out(stablo) RETURNS cstring AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE TYPE stablo (internallength = 4096, input = stablo_in, output = stablo_out, alignment = int);
CREATE OR REPLACE FUNCTION stablo_broj_cvorova(stablo) RETURNS INT AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_broj_cvorova_podstabla(stablo, cstring) RETURNS INT AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_postoji_cvor(stablo, cstring) RETURNS BOOL AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_vrijednost(stablo, cstring) RETURNS INT AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_promijeni_vrijednost(stablo, cstring, int) RETURNS stablo AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_dodaj_cvor(stablo, cstring, int) RETURNS stablo AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_orezi(stablo, cstring) RETURNS stablo AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_podstablo(stablo, cstring) RETURNS stablo AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_dodaj_podstablo(stablo, cstring, cstring) RETURNS stablo AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_preorder(stablo) RETURNS cstring AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_inorder(stablo) RETURNS cstring AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_postorder(stablo) RETURNS cstring AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_pretrazi(stablo, int) RETURNS cstring AS 'stablo' LANGUAGE C IMMUTABLE STRICT;

-- definiranje operatora jednakosti
CREATE OR REPLACE FUNCTION stablo_eq(stablo, stablo) RETURNS BOOL AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION stablo_neq(stablo, stablo) RETURNS BOOL AS 'stablo' LANGUAGE C IMMUTABLE STRICT;
CREATE OPERATOR = (leftarg = stablo, rightarg = stablo, procedure = stablo_eq, commutator = '=', negator = '<>');
CREATE OPERATOR <> (leftarg = stablo, rightarg = stablo, procedure = stablo_neq, commutator = '<>', negator = '=');