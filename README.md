

<p align="center"><img src="./Images/logo.png" alt="FiberPunk's logo" width=400/></p>

<h1 align="center">Beam-ESP32</h1>
<p align="center">
  <img src="https://img.shields.io/badge/arduino-opensource-brightgreen" alt="opensource"/>
   <img src="https://img.shields.io/badge/hardware-lowcost-blue" alt="lowcost"/>

</p>

## 1. Project catalog description

- Dependencies: Espressif System 1.0.6

## 2. Beam Project Introduction

<img src="./Images/hardware.jpg" width=400 />

### 2.1 What is Beam?

Beam is an easy and affordable way to control and monitor 3D printers via WiFi, USB-enabled serial, and SD I/O. Beam is very robust and power-efficient as it runs on an MCU (ESP32) while still offering a wide range of functionalities.

Key features
- East to install - Beam can be installed by plugging into a printer’s USB-enabled serial connection and data cable to SD card slow. This means no opening up the printer and complicated wiring. Anyone can do it.
- Easy to set up - Beam can be set up to connect user network by simply running the desktop application, entering network credentials, and exporting the setting to sd card. 
- Beam has good compatibility with printers due to Beam only relying on serial connection for control and monitor and utilizing the high bandwidth SD I/O for transfer.
Faster and more reliable printing via Beam’s unique SD Card I/O bridge to printer
- Beam works with or without an external internet connection, and functionalities can be realized via BeamManager
- Easy to manage multiple printers with a single software control point
- Affordable hardware cost. No need to acquire a Pi for every printer
- Email Notification via BeamManger 
- Future Expandability with Beam Camera and Machine Vision for Failure Detection via BeamManager
- Provide simple RestfulAPI to allow other software control (such as postman debugging tools)
- Beam includes extended interfaces (IIC, UART, SLR control circuit) to work with ESP32-Camera. This allows for time-lapse photography production, taking photos according to the progress and email reminders, PC local printing failure detection, and smoke flame detection, all of which do not rely on registered accounts and networks.


### 2.Why Beam?

When we first look at adding WiFi control to our 3D printers, there are already excellent choices, such as the wonderful Octoprint project run by our good friend Gina with great plugins and communities. Octoprint controls and transfers via serial connection only, which is limited by the speed of the connection and firmware compatibility issues. 

We want to challenge ourselves to see if there is a leaner, MCU based solution that can achieve these goals:

- Allows for faster print speed than serial connection, which is needed for newer printers, especially CoreXY Printer
- More compatibility with different printers
- Easy to install, especially for users who are new to 3D printing
- Simple to set up and manage a fleet of printers.
- Robust operation such that the device can be powered off immediately and does not rely on an Internet connection to function


To meet the above goals, we needed a tightly integrated solution that requires hardware that does not exist on the market. This led us to develop Beam’s ESP32 based board and the open-source firmware.  We also designed the unique SD Bridge that allows Beam to transfer GCode via high-speed SD I/O while controlling/monitoring via the serial connection.

## 3. Instructions for use

### 3.1 The network requirements of the host computer

Please make sure you have 2.4G network, ESP32 can only connect 2.4G wifi at present. In addition, 360 or some antivirus software, the computer set static IP, VPN, etc., will have an impact on the device search. If you want to use an automated search device, make sure that your computer's IP network segment is consistent with the router's network segment.

### 3.2 Quick start

**1.Network Configuration**

After inserting the SD card, configure the SD card

<img src="./Images/gif/1.config.gif" width=500 />

**2.LAN device scanning**

<img src="./Images/gif/2.find-devices.gif" width=500 />

**3.Control printing**

Control panel.

<img src="./Images/gif/3.control-pannel.gif" width=500 />

**4.Print**

<img src="./Images/gif/5.print.gif" width=500 />

**5.Mail configuration**

Email can configure the percentage of each printing completed, take a picture, and send a reminder mail to the designated mailbox. Printing failure reminder is when the software detects the probability of printing failure, it will send an email to notify the user (cooperate with Camera shooting).

<img src="./Images/email.png" width=300 />

## 4.Beam-API

Beam provides core APIs to enable more platforms to access him for control. Listed below are the APIs already included in Beam and their specific usage.[API docs](./FP-BeamAPI.md)

## 5.How to update Beam-ESP32 firmware

1. Download [flash tool](https://github.com/fiberpunk1/Beam-ESP32/releases/download/Beta-v0.1.0/BeamFlash-Installer.exe)
2. On the Beam-ESP32 release page, download the first .bin file
3. Burn the firmware in the order shown in the following figure:

<img src="./Images/update-bin.png />



