# Gawake

<div align="center">
  <img src="https://img.shields.io/static/v1?label=Platform&message=Linux&color=yellow&style=for-the-badge&logo=Linux" /> <img src="https://img.shields.io/static/v1?label=Language&message=Python3&color=blue&style=for-the-badge&logo=Python" /> <img src="https://img.shields.io/static/v1?label=Coded%20with&message=Neovim&color=stronggreen&style=for-the-badge&logo=Neovim" />
</div>

## What does Gawake do?

You can schedule the time and the day - such as on your smartphone alarm - that your PC will turn on. It's possible to create many personalized rules, depending on your needs.
There are 3 main modes to suspend yor PC:
+ **off** – Turn the computer off completely (isn’t officially supported by the ACPI specification, but this works with many computers anyway).
+ **disk** - Suspend to disk. This state offers the greatest power savings. When the computer turns on, its state will be restored.
+ **mem** - Suspend to RAM. Offers significant power savings as everything in the system is put into a low-power state, except for memory.

***If you want power saving, use "off" or "disk" modes.***

Gawake makes the "rtcwake" linux command easier and more pratical. See the rtcwake documentation here: https://man7.org/linux/man-pages/man8/rtcwake.8.html

## Gawake CLI

Here how it seems:
<div align="center"> <img src="https://user-images.githubusercontent.com/83086622/147366356-67cdf6a7-a9fd-4c38-922d-d5cb051afa44.png" /> </div>
<div align="center"> <img src="https://user-images.githubusercontent.com/83086622/147366359-3f756736-2aa9-4c5b-b87f-7a59f485d2c9.png" /> </div>

### Install
To install, follow these steps:
```bash
  # Clone this repository
  git clone https://github.com/KelvinNovais/Gawake.git

  # Move gawake script to /bin
  sudo mv Gawake/CLI/gawake-cli.py /bin/gawake-cli

  # Edit the crontab
  sudo nano /etc/crontab

  # Add these lines to the end of the crontab:
  # GAWAKE
  0 *   * * *	root	/bin/gawake-cli -c
  15 *	* * *	root	/bin/gawake-cli -c
  30 *	* * *	root	/bin/gawake-cli -c
  45 *	* * *	root	/bin/gawake-cli -c
```
Important! You must have Pyhton in version 3 or later.

### Usage

Terminal commands:
```bash
  # To access all functions, run:
  $ gawake-cli
  
  # To directly schedule wake up, run:
  $ gawake-cli -s
  
  
  # Documentation only: #
  $ gawake-cli -c
  # The previous command is used on crontab to check if there is a valid turn off rule at that time. If you want to schedule a wake up, use 'gawake-cli -s' instead.
```

### Comming features
+ Run a command after your PC turning on (like opening the browser - for GUI, only);
+ Graphical user interface;
