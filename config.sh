#!/bin/bash

# Build directory
BUILD_DIR=./_build/src

# Services binaries
GAWAKE_DEST_DIR=/opt/gawake/bin/cli

# Services paths
GAWAKED_SERVICE=/usr/lib/systemd/system/gawaked.service
GAWAKE_DBUS_SERVICE=/usr/lib/systemd/system/io.github.kelvinnovais.GawakeServer.service
GAWAKE_DBUS_CONF=/usr/share/dbus-1/system.d/io.github.kelvinnovais.GawakeServer.conf

# Database
GAWAKE_DB_DIR=/var/lib/gawake
DB_NAME=gawake.db
GAWAKE_DB_PATH=$GAWAKE_DB_DIR/$DB_NAME