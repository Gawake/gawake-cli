#!/bin/bash

source config.sh

gawake_cli=/usr/bin/gawake-cli
err_counter=0

systemctl stop gawaked.service
systemctl stop io.github.kelvinnovais.GawakeServer.service

# Removing the main binary from /bin
if [ -f "$gawake_cli" ]; then
  if rm $gawake_cli; then
    echo "Main binary removed"
  else
    echo "COULDN'T REMOVE MAIN BINARY"
  fi
  err_counter=$(($? + $err_counter))
else
  echo "gawake-cli already removed"
fi

# Removing binaries from /opt/gawake/bin/cli
if [ -d "$GAWAKE_DEST_DIR" ]; then
  if rm -rf $GAWAKE_DEST_DIR; then
    echo "Binaries removed"
  else
    echo "COULDN'T REMOVE BINARIES"
  fi
  err_counter=$(($? + $err_counter))
else
  echo "Binaries already removed"
fi

# Removing services files
rm $GAWAKED_SERVICE
rm $GAWAKE_DBUS_SERVICE
rm $GAWAKE_DBUS_CONF
systemctl daemon-reload

# Removing Gawake user
/usr/sbin/groupdel gawake
/usr/sbin/userdel gawake

# Ask for confirmation before deleting the database
if [ -f "$GAWAKE_DB_PATH" ]; then
  read -p "Do you want to remove the database? [y/N] " db
  if [ "$db" = "y" ] || [ "$db" = "Y" ]; then
    if rm $GAWAKE_DB_PATH; then
      echo "Database removed"
    else
      echo "COULDN'T REMOVE DATABASE"
    fi
    err_counter=$(($? + $err_counter))
  fi
fi

if [ $err_counter -eq 0 ]; then
  echo "Gawake removed successfully!"
else
  echo "Gawake removed with errors"
fi
