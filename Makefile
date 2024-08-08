EXTENSION = pg_amqp

MODULES = pg_amqp
OBJS = src/pg_amqp.o

DATA = sql/pg_amqp--1.0.sql

PG_CONFIG  ?= pg_config

PG_CFLAGS := -I/usr/include -I/usr/include/postgresql
PG_LIBS := -lrabbitmq -lpq

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
