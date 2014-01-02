--
-- From http://stackoverflow.com/a/8798265/398670
--

CREATE FUNCTION array_idx(needle ANYELEMENT, haystack ANYARRAY)
RETURNS INT AS $$
    SELECT i
      FROM generate_subscripts($2, 1) AS i
     WHERE $2[i] = $1
  ORDER BY i
$$ LANGUAGE sql STABLE;

COMMENT ON array_idx(anyelement, anyarray) IS
'Find the index of an element within an array, returning NULL if not found';
