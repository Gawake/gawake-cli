#!/bin/bash

BUILD_DIR=./_build/src
GAWAKE_DEST_DIR=/opt/gawake/bin/cli
GAWAKE_DB_DIR=/var/lib/gawake
DB_NAME=gawake.db

echo "Installing gawake-cli..."

# [1] Adding Gawake user
echo "Adding Gawake user"
/sbin/useradd -Mrl gawake

# [2] Creating directories
echo "Creating directories"
mkdir -p $GAWAKE_DEST_DIR
# Gawake database directory
mkdir -p $GAWAKE_DB_DIR
chown gawake:gawake $GAWAKE_DB_DIR
chmod 770 $GAWAKE_DB_DIR

# [3] Creating database
echo "Creating database"
cat ./src/database.sql | sqlite3 $GAWAKE_DB_DIR/$DB_NAME

# [4] Installing the services binaries to /opt/gawake/bin/cli
echo "Installing the binaries to ${GAWAKE_DEST_DIR}"
# destination src
install -o root -g root -m 550 $BUILD_DIR/gawaked.out $GAWAKE_DEST_DIR/gawaked
install -o gawake -g gawake -m 550 $BUILD_DIR/gawake-dbus-server.out $GAWAKE_DEST_DIR/gawake-dbus-server

# [5] Install services files
echo "Installing services files"
install ./services/system/gawaked/gawaked.service /usr/lib/systemd/system/gawaked.service
install ./services/system/gawake-dbus-server/io.github.kelvinnovais.GawakeServer.service \
	/usr/lib/systemd/system/io.github.kelvinnovais.GawakeServer.service

install ./services/system/gawake-dbus-server/io.github.kelvinnovais.GawakeServer.conf \
	/usr/share/dbus-1/system.d/io.github.kelvinnovais.GawakeServer.conf

# [6] Installing the main binary to /bin
echo "Installing the main binary to /bin"
install $BUILD_DIR/gawake-cli.out /usr/bin/gawake-cli
