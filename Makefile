EXTENSION = pg_amqp
EXTVERSION = 1.0

PG_CONFIG  ?= pg_config

MODULES = src/pg_amqp

DATA = sql/pg_amqp--1.0.sql

INCLUDES = -I/usr/local/include
LIBS = -lrabbitmq

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

CFLAGS += $(INCLUDES)
LDFLAGS += $(LIBS)