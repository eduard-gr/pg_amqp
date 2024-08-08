#include "postgres.h"
#include "fmgr.h"
#include "libpq-fe.h"
#include "utils/guc.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

static amqp_connection_state_t connection;
static amqp_socket_t *socket = NULL;

static char *amqp_host = NULL;
static int amqp_port = 5672;
static char *amqp_user = NULL;
static char *amqp_password = NULL;

void _PG_init(void);
void _PG_fini(void);

Datum pg_amqp_publish(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(pg_amqp_publish);
Datum pg_amqp_publish(PG_FUNCTION_ARGS)
{
    if (connection == NULL) {
        elog(ERROR, "RabbitMQ connection is not initialized.");
        PG_RETURN_VOID();
    }

    text *exchange = PG_GETARG_TEXT_P(0);
    text *routing = PG_GETARG_TEXT_P(1);
    text *message = PG_GETARG_TEXT_P(2);

    amqp_basic_publish(
        connection,
        /*channel*/ 1,
        /*exchange*/ amqp_cstring_bytes(text_to_cstring(exchange)),
        /*routing*/ amqp_cstring_bytes(text_to_cstring(routing)),
        0,
        0,
        NULL,
        amqp_cstring_bytes(text_to_cstring(message)));

    PG_RETURN_VOID();
}


void _PG_init(void)
{
    connection = amqp_new_connection();
    socket = amqp_tcp_socket_new(connection);
    if (!socket) {
        elog(ERROR, "Failed to create TCP socket for RabbitMQ.");
        return;
    }

    // Read RabbitMQ configuration from postgresql.conf
    amqp_host = GetConfigOption("amqp_host", true, true);
    amqp_port = atoi(GetConfigOption("amqp_port", true, true));
    amqp_user = GetConfigOption("amqp_user", true, true);
    amqp_password = GetConfigOption("amqp_password", true, true);

    if (amqp_socket_open(socket, amqp_host, amqp_port)) {
        elog(ERROR, "Failed to open TCP socket to RabbitMQ.");
        return;
    }

    amqp_rpc_reply_t reply = amqp_login(
        connection,
        "/",
        0,
        131072,
        0,
        AMQP_SASL_METHOD_PLAIN,
        amqp_user,
        amqp_password);

    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        elog(ERROR, "Failed to login to RabbitMQ.");
        return;
    }

    amqp_channel_open(connection, 1);
    reply = amqp_get_rpc_reply(connection);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        elog(ERROR, "Failed to open channel with RabbitMQ.");
        return;
    }
}

void _PG_fini(void)
{
    if (connection == NULL) {
        return;
    }

    amqp_channel_close(connection, 1, AMQP_REPLY_SUCCESS);
    amqp_connection_close(connection, AMQP_REPLY_SUCCESS);
    amqp_destroy_connection(connection);
}
