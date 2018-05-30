OpenBeacon Mini Quick Start
===========================

Requirements
------------

* Internet connected PC (Windows, Linux, or macOS)
* Python 3 programming language [installed](https://www.python.org/)
* NTP time synchronization (Use of ntpd on Linux and macOS, or on Windows a program such as [Meinberg NTP](https://www.meinbergglobal.com/english/sw/)

Software Prerequisite Installation
----------------------------------
The Python 3 programming language and the Python serial port library must be installed on your PC before the time synchronization script can be run.

#### Windows
* Download the latest Python 3 installer package from the [official Python download page](https://www.python.org/downloads/). Run the installer with all of the default options.
* Open up a command prompt and execute the following command to install Python serial port support:
`pip install pyserial`.

#### Linux (Debian-based distributions)
* Open a terminal and install the Python 3 package from your distribution package manager with the following command: `sudo apt install python3 python3-pip`
* Install Python serial port support with this command: `sudo pip install pyserial`.

Connections
-----------
Plug OpenBeacon Mini into your PC via the Micro USB connector on the Empyrean daughterboard. This provides both power and a data connection to OpenBeacon Mini. Connect an appropriate 50&Omega; antenna to the Antenna port BNC connector.

Band Modules
------------

Time Synchronization
--------------------
Open a terminal/command window in the directory where the timesync.py script resides. In order to run the time synchronization program, we need to know which serial port that timesync.py can find OpenBeacon Mini. You can use the command `python3 timesync.py -l` to list all of the available serial ports on your PC.

The `-p` argument of timesync.py is used to specify the port that your OpenBeacon Mini is connected to.  So for example, if your OpenBeacon Mini is on COM25, you would use the following command on Windows to start the program:

`python timesync.py -p COM25`

The command on Linux and macOS machines would be:

`python3 timesync.py -p /dev/ttyACM0`

Once the program connects to OpenBeacon Mini and sets the time on it, the green LED on OpenBeacon Mini will light to indicate a good time synchronization and timesync.py will print out a timestamp of the synchronization.

Menu System
-----------

Settings
--------

Transmitting
------------
