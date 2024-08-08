CREATE FUNCTION @extschema@.publish(
    exchange varchar,
    routing_key varchar,
    message varchar
)
RETURNS boolean AS 'pg_amqp.so', 'pg_amqp_publish'
LANGUAGE C IMMUTABLE;

COMMENT ON FUNCTION @extschema@.publish(varchar, varchar, varchar) IS
'Publishes a message (exchange, routing_key, message).
The message will only be published if the containing PostgreSQL transaction successfully commits.';

