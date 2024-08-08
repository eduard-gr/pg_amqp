#include "postgres.h"
#include "miscadmin.h"
#include "fmgr.h"

#include "utils/guc.h"
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <limits.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

#define SET_BYTES_FROM_PG_TEXT_ARG(var,col) do{ \
    if(!PG_ARGISNULL(col)) { \
        text *txt = PG_GETARG_TEXT_PP(col); \
        var.bytes = VARDATA_ANY(txt); \
        var.len = VARSIZE_ANY_EXHDR(txt); \
    } \
}while(0)

static amqp_connection_state_t connection;
static amqp_socket_t *socket = NULL;

static char *amqp_host = "localhost";
static int amqp_port = 5672;
static char *amqp_user = "guest";
static char *amqp_password = "guest";

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

    amqp_bytes_t exchange;
    amqp_bytes_t routing;
    amqp_bytes_t message;

    SET_BYTES_FROM_PG_TEXT_ARG(exchange, 0);
    SET_BYTES_FROM_PG_TEXT_ARG(routing, 1);
    SET_BYTES_FROM_PG_TEXT_ARG(message, 2);

    amqp_basic_properties_t properties;
    properties._flags = AMQP_BASIC_DELIVERY_MODE_FLAG;
    properties.delivery_mode = 2;

    int result;
    result = amqp_basic_publish(
        connection,
        /*channel*/ 1,
        /*exchange*/ exchange,
        /*routing*/ routing,
        0,
        0,
        &properties,
        message);

    if(result != AMQP_STATUS_OK){
        elog(ERROR, "RabbitMQ failed to send message.");
    }

    PG_RETURN_VOID();
}


void _PG_init(void)
{


    if (!process_shared_preload_libraries_in_progress)
    {
        ereport(ERROR, (errmsg("pg_amqp can only be loaded via shared_preload_libraries"),
                        errhint("Add pg_cron to the shared_preload_libraries "
                                "configuration variable in postgresql.conf.")));
    }

    // Read RabbitMQ configuration from postgresql.conf

    DefineCustomStringVariable(
        "amqp.host",
        gettext_noop("amqp server host"),
        NULL,
        &amqp_host,
        "localhost",
        PGC_POSTMASTER,
        GUC_SUPERUSER_ONLY,
        NULL, NULL, NULL);

    DefineCustomIntVariable(
        "amqp.port",
        gettext_noop("amqp server port"),
        NULL,
        &amqp_port,
        5672,
        0,
        INT_MAX,
        PGC_POSTMASTER,
        GUC_SUPERUSER_ONLY,
        NULL, NULL, NULL);

    DefineCustomStringVariable(
        "amqp.user",
        gettext_noop("amqp server user"),
        NULL,
        &amqp_user,
        "guest",
        PGC_POSTMASTER,
        GUC_SUPERUSER_ONLY,
        NULL, NULL, NULL);

    DefineCustomStringVariable(
        "amqp.password",
        gettext_noop("amqp server password"),
        NULL,
        &amqp_password,
        "guest",
        PGC_POSTMASTER,
        GUC_SUPERUSER_ONLY,
        NULL, NULL, NULL);

    connection = amqp_new_connection();
    socket = amqp_tcp_socket_new(connection);
    if (!socket) {
        elog(ERROR, "Failed to create TCP socket for RabbitMQ.");
        return;
    }


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

    elog(LOG, "pg amqp started");
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
