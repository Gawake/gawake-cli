#!/bin/bash

source config.sh

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
cat ./src/database.sql | sqlite3 $GAWAKE_DB_PATH
chown gawake:gawake $GAWAKE_DB_PATH
chmod 660 $GAWAKE_DB_PATH

# [4] Installing the services binaries to /opt/gawake/bin/cli
echo "Installing the binaries to ${GAWAKE_DEST_DIR}"
# destination src
install -C -o root -g root -m 550 $BUILD_DIR/gawaked.out $GAWAKE_DEST_DIR/gawaked
install -C -o gawake -g gawake -m 550 $BUILD_DIR/gawake-dbus-server.out $GAWAKE_DEST_DIR/gawake-dbus-server

# [5] Install services files
echo "Installing services files"
install -C ./services/system/gawaked/gawaked.service $GAWAKED_SERVICE
install -C ./services/system/gawake-dbus-server/io.github.kelvinnovais.GawakeServer.service $GAWAKE_DBUS_SERVICE

install -C ./services/system/gawake-dbus-server/io.github.kelvinnovais.GawakeServer.conf $GAWAKE_DBUS_CONF

# [6] Installing the main binary to /bin
echo "Installing the main binary to /bin"
install -C $BUILD_DIR/gawake-cli.out /usr/bin/gawake-cli