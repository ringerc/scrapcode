--
-- Test for facilities of security label
--
CREATE EXTENSION parallel_unsafe;

SELECT x, y
FROM generate_series(1,100) x, parallel_unsafe(x) y;

SET force_parallel_mode = on;
SET max_parallel_workers = 10;

SELECT x, y
FROM generate_series(1,100) x, parallel_unsafe(x) y;
