#!/bin/bash

# moving files to /bin
sudo cp gawake-cli.py /bin/gawake-cli
sudo cp gawake-schedule.py /bin/gawake-schedule

# making them executable
sudo chmod +x /bin/gawake-cli /bin/gawake-schedule

# creating folder to store the database
mkdir /home/${USER}/.gawake

# if more than one try happens, remove the old uninstall file
rm /home/${USER}/.gawake/uninstall.sh

# creating the uninstall file
echo "#!/bin/bash" >> /home/${USER}/.gawake/uninstall.sh
echo "sudo rm /bin/gawake-cli" >> /home/${USER}/.gawake/uninstall.sh
echo "sudo rm /bin/gawake-schedule" >> /home/${USER}/.gawake/uninstall.sh

# making uninstall executable
chmod +x /home/${USER}/.gawake/uninstall.sh
