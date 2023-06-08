# Gawake

[1. What does Gawake do?](#what-does-gawake-do)

[2. Gawake CLI](#gawake-cli)

**[3. Installation](#install)**

**[4. Usage](#usage)**

## What does Gawake do?

You can schedule the time and the day - such as on your smartphone alarm - that your PC will turn on. It's possible to create many personalized rules, depending on your needs.
There are 3 main modes to suspend yor PC:
+ **off** – Turn the computer off completely (isn’t officially supported by the ACPI specification, but this works with many computers anyway).
+ **disk** - Suspend to disk. This state offers the greatest power savings. When the computer turns on, its state will be restored.
+ **mem** - Suspend to RAM. Offers significant power savings as everything in the system is put into a low-power state, except for memory.

***If you want power saving, use "off" or "disk" modes.***

Gawake makes the "rtcwake" linux command easier and more pratical. See the rtcwake documentation here: https://man7.org/linux/man-pages/man8/rtcwake.8.html

## Gawake CLI

### Install

#### From the pre-compiled binaries

```bash
  # Download the latest release

  # Run the install script
  chmod +x install.sh
  sudo ./install.sh
```

#### Compile it yourself

You will need:

```
git
gcc
make

# SQLite3 developer package:
sqlite-devel        # For Fedora
libsqlite3-dev      # For Debian, Ubuntu, and derivatives
```
```bash
  # Clone this repository
  git clone https://github.com/KelvinNovais/Gawake.git
  cd Gawake/cli/
  
  # Compile
  make all
  
  # Install
  sudo make install
```

### Usage

Terminal commands:
```bash
  # To access all functions, run:
  $ gawake-cli
  
  # To directly schedule wake up, run:
  $ gawake-cli -s
  
  # To run a custom schedule:
  # Scheduling for 2023-12-28 09:30:00, using the default mode
  $ gawake-cli -c 20231228093000
  
  # To run a custom schedule, with another mode:
  # Scheduling for 2023-07-25 16:45:00
  $ gawake-cli -c 20230725164500 -m disk
  
  # To uninstall Gawake
  $ gawake-cli -U
```
