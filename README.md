pg_amqp
=============

The pg_amqp package provides the ability for postgres statements to directly
publish messages to an [AMQP](http://www.amqp.org/) broker.

All bug reports, feature requests and general questions can be directed to the Issues section on Github. - https://github.com/eduard-gr/pg_amqp

Due to the fact that the library of the same name [pg_amqp](https://github.com/omniti-labs/pg_amqp) ceased to exist and be supported, I had to take action and make my own with minimal requirements

Building
--------

To build pg_amqp, just do this:

    make
    make install

Loading
-------

Once amqp is installed, you can add it to a database. Add this line to your
postgresql config

    shared_preload_libraries = 'pg_amqp.so'
    amqp_host = 'localhost'
    amqp_port = 5672
    amqp_user = 'guest'
    amqp_password = 'guest'

This extension requires PostgreSQL 9.1.0 or greater, so loading amqp is as simple
as connecting to a database as a super user and running 

    CREATE EXTENSION amqp;

If you've upgraded your cluster to PostgreSQL 9.1 and already had amqp
installed, you can upgrade it to a properly packaged extension with:


Basic Usage
-----------

A process starts and connects to PostgreSQL and runs:

    SELECT amqp.publish('amqp.direct', 'foo', 'message');
