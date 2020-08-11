-- Test with non-constexpr elevel to make sure we still elide calls.

SET client_min_messages = 'info';

SELECT clock_timestamp() AS before_ts \gset
SELECT ereport_skip_test2();
SELECT clock_timestamp() AS after_ts \gset

SELECT (TIMESTAMP :'after_ts' - TIMESTAMP :'before_ts') > INTERVAL '1' SECOND AS did_delay_when_no_output;

SET client_min_messages = 'debug1';

SELECT clock_timestamp() AS before_ts \gset
SELECT ereport_skip_test2();
SELECT clock_timestamp() AS after_ts \gset

SELECT (TIMESTAMP :'after_ts' - TIMESTAMP :'before_ts') > INTERVAL '1' SECOND AS did_delay_when_output;
