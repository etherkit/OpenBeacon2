# OpenBeacon 2 Quick Start

## Requirements

* Internet connected PC (Windows or Linux)
* NTP time synchronization (Use of ntpd on Linux and macOS, or on Windows a program such as [Meinberg NTP](https://www.meinbergglobal.com/english/sw/))

## Setup

### Download Client

In order to use OpenBeacon 2, you will need to connect it to a PC with its clock set by NTP. OpenBeacon 2 currently supports Windows (64-bit), Linux x86 (64-bit), and Linux ARM (Raspberry Pi) PCs as synchronization clients.

Download the appropriate client program below:

* [Windows](https://github.com/etherkit/OpenBeacon2/raw/master/client/win/ob2sync.exe)
* [Linux x86-64](https://github.com/etherkit/OpenBeacon2/raw/master/client/linux-x86/ob2sync)
* [Linux ARM](https://github.com/etherkit/OpenBeacon2/raw/master/client/linux-arm/ob2sync)

### Initial Startup

The OpenBeacon 2 client program is currently command-line only. In order to start OpenBeacon 2 and synchonize the OpenBeacon 2 on-board clock with PC time, first connect OpenBeacon 2 with a USB Micro-B to USB A cable to the PC and then execute the client program.

The client program takes a variety of parameters, but the most important one to specify is the virtual serial port that OpenBeacon 2 is using on your PC. If you are uncertain of which port is being used, the client can list all available serial ports for you.

For Linux:

    ./ob2sync -l

For Windows:

    ob2sync -l

Once you know the OpenBeacon 2 serial port, start the client program.

For Linux, assuming the serial port is /dev/ttyACM1:

    ./ob2sync -p /dev/ttyACM1

For Windows, assuming the serial port is COM5:

    ob2sync -p COM5

If the client program successfully connected, you will see the display on OpenBeacon 2 change from "OpenBeacon 2 Sync with PC to begin" to the main operating display with current frequency, time, and mode. The client program will show

    OpenBeacon 2
    Time sync at X

where X is the current date and time and the green LED on OpenBeacon 2 will light to indicate a good time synchronization.

### Minimal Configuration

Now that OpenBeacon 2 is connected to the PC, we can set the configuration so that it is ready for basic transmissions. All of the OpenBeacon 2 configuration parameters can be set via either the client program or directly from the OpenBeacon 2 hardware, but since it is generally easier to use the client program, we'll demonstrate using only the client program.

A list of the [configuration parameters can be found here](SerialProtocol.md#OpenBeacon-2-Parameters). These keywords are used in conjunction with the ```set``` keyword in order to set the appropriate parameter.

So, in order to set your callsign (using the fictonal callsign N0CALL), you'll issue the following command using the client program:

    set callsign N0CALL

Likewise, set your Maidenhead grid square:

    set grid AA00

Your approximate output power in dBm:

    set power 23

OpenBeacon 2 stores transmit messages (for the freeform modes like CW, QRSS, etc) in four buffers. Set the message in buffer one with the following command:

    set msg_buffer_1 "N0CALL HNY"

If your message will include any space characters, be sure to enclose your message in quotes.

It is wise to ensure that OpenBeacon 2 has the correct configuration by asking the client program to read back the current configuration with the ```get``` command. So, for example

    get callsign

should return

    callsign: N0CALL

### Initiate A Transmission

Now that OpenBeacon 2 is configured, let's do a test transmission to ensure it is working correctly. Connect a 50 ohm RF dummy load that can dissipate at least 500 mW safely to the OpenBeacon 2 antenna terminal.

It's best to use a mode where you can properly monitor the output of OpenBeacon 2 in order to verify that it is transmitting correctly. That can mean by listening to a CW transmission by ear on a receiver or by using WSJT-X to decode a WSPR transmission, to give a few examples. Find the currently supported modes by issuing the following command in the client program:

    list modes

Once you've found a mode that you want to try, find the output frequency on the OpenBeacon 2 display or by issuing the client command:

    get base_freq

Of course, you can also set your transmission frequency with

    set base_freq [FREQUENCY]

where ```FREQUENCY``` is the desired operating frequency (within the current band) in Hertz.

Tune your receiver to that frequency, and then schedule a transmission by issuing the command:

    tx enable

On the OpenBeacon 2 display, you'll see the scheduled time of the transmission at the bottom-right (if the screensaver is active, press any key to return to the operating display).

When the transmission starts, the blue LED on OpenBeacon 2 will light up. Once the transmission is complete, the blue LED will extinguish and you'll get a notification in the client of the type of transmission completed, along with the transmission duration.

OpenBeacon 2 will continue to transmit at a specified interval until you disable transmissions. To do this, issue the command:

    tx disable

You can also use Button 1 (at the far left under the display) to do the same.

The transmission interval can be set by issuing the command

    set tx_intv [INTERVAL]

where ```INTERVAL``` is the amount of time in mintues you want between transmissions.

Since OpenBeacon 2 is a multi-band transmitter, you also have the option of getting a list of bands available to operate based on the currently plugged-in band modules:

    list bands

You can set then set the current operating band to any of the ones listed with

    set band [BAND]

where ```BAND``` is one of the strings listed in the ```list bands``` command.

## Script Execution

On its own, OpenBeacon 2 can do a repeating transmission of a single message on a single band and single mode. However, by using the scripting built into the client program, a virtually limitless amount of transmit automation can be used to control OpenBeacon 2. You can see a handful of example scripts in [this directory](/client/examples).

Delving into the details of writing a script is beyond the scope of this quick tutorial, but it is easy to execute scripts in the client by using the ```run_pyscript``` command. If you wanted to execute the ```multi_wspr_tx.py``` example script, it's as easy as downloading the script to your PC and using the command

    run_pyscript multi_wspr_tx.py

assuming that the script file was downloaded to the same directory as the client program. Cancel execution of any script by pressing ```Ctrl-C``` while it is running.

## Firmware Updates

Due to the built-in bootloader in the Empyrean daughterboard used to power OpenBeacon 2, you don't need to have any special hardware or software installed in order to update the firmware. All you'll need is to grab the firmware in the UF2 file format (latest version [here](https://github.com/etherkit/OpenBeacon2/raw/master/firmware/OpenBeacon2.ino.empyrean_alpha.uf2)), connect OpenBeacon 2 to your PC with its USB cable, put the device into firmware update mode by double-tapping the reset button, and then waiting for OpenBeacon 2/Empyrean to show up on your PC as an external drive. After that, simply drag the firmware update UF2 file from your PC to the Empyrean drive. OpenBeacon 2/Empyrean will automatically update the firmware and then reset the device. That's it!