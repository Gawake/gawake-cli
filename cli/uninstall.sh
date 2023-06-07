#!/bin/bash

# Variables
err_counter=0
gawake_cli=/usr/bin/gawake-cli
helper_bins=/opt/gawake/bin/cli/
cron=/etc/cron.d/gawake
logs=/var/gawake/logs/
db_path=/var/gawake/gawake-cli.db

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

# Removing helper binaries from /opt/gawake/bin/cli
if [ -d "$helper_bins" ]; then
  if rm -rf $helper_bins; then
    echo "Helper binaries removed"
  else
    echo "COULDN'T REMOVE HELPER BINARIES"
  fi
  err_counter=$(($? + $err_counter))
else
  echo "Helper binaries already removed"
fi

# Removing the cron service to /etc/cron.d
if [ -f "$cron" ]; then
  if rm $cron; then
    echo "Cron rule removed"
  else
    echo "COULDN'T REMOVE CRON RULE"
  fi
  err_counter=$(($? + $err_counter))
else
  echo "Cron rule already removed"
fi

# Removing Gawake user
userdel gawake

# Ask for confirmation before deleting logs and the database
if [ -d "$logs" ]; then
  read -p "Do you want to remove the logs? [y/N] " logs
  if [ "$logs" = "y" ] || [ "$logs" = "Y" ]; then
    if rm -rf $logs; then
      echo "Logs removed"
    else
      echo "COULDN'T REMOVE LOGS"
    fi
    err_counter=$(($? + $err_counter))
  fi
fi

if [ -f "$db_path" ]; then
  read -p "Do you want to remove the database? [y/N] " db
  if [ "$db" = "y" ] || [ "$db" = "Y" ]; then
    if rm $db_path; then
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
