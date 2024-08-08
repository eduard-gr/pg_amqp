MODULES = pg_amqp
EXTENSION = pg_amqp
DATA = pg_amqp--1.0.sql
PG_CONFIG = pg_config

PG_CFLAGS = -I/usr/include -I/usr/include/postgresql
PG_LIBS = -lrabbitmq -lpq

include $(PG_CONFIG) --pgxs

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
