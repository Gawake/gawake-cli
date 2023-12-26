#!/bin/bash

err_counter=0

echo "Installing gawake-cli..."

# [1] Install the main binary to /bin
echo "Installing the main binary to /bin"
install -v ./build/gawake-cli /usr/bin/                                                       # src destination
err_counter=$(($? + $err_counter))

# [2] Install the helper binaries to /opt/gawake/bin/cli
echo "Installing the helper binaries to /opt/gawake/bin/cli"
install -v -o root -g root -m 754 -D -t /opt/gawake/bin/cli/ ./build/{cron*,db_checker}       # destination src
err_counter=$(($? + $err_counter))

# [3] Install the uninstall script
install -v -o root -g root -m 555 uninstall.sh /opt/gawake/                                   # src destination
err_counter=$(($? + $err_counter))

# [4] Install the cron service to /etc/cron.d
echo "Installing the cron service to /etc/cron.d"
install -v -o root -g root -m 644 gawake /etc/cron.d/                                         # src destination
err_counter=$(($? + $err_counter))

# [5] Adding Gawake user
useradd -Mr gawake

# [6] Creating logs directory
mkdir -p /var/gawake/logs/

if [ $err_counter -eq 0 ]; then
  echo "Gawake installed successfully!"
else
  echo "Gawake installed with errors"
fi
