

<p align="center"><img src="./Images/logo.png" alt="FiberPunk's logo" width=400/></p>

<h1 align="center">Beam-ESP32</h1>
<p align="center">
  <img src="https://img.shields.io/badge/arduino-opensource-brightgreen" alt="opensource"/>
   <img src="https://img.shields.io/badge/hardware-lowcost-blue" alt="lowcost"/>

</p>

## 1. Project catalog description

- Dependencies: Espressif System 1.0.6

## 2. Beam Node Project Introduction

**PC Client:**
<div align=center>
<img src="./Images/BeamNexus.jpg" width=800 />
</div>

### 2.1 What is Beam Node?

Node is an easy and affordable way to control and monitor 3D printers via WiFi, USB-enabled serial, and SD I/O. Node is very robust and power-efficient as it runs on an MCU (ESP32) while still offering a wide range of functionalities.

**Key features:**
- East to install - Beam can be installed by plugging into a printer’s USB-enabled serial connection and data cable to SD card slow. This means no opening up the printer and complicated wiring.
- Easy to set up - Beam can be set up to connect user network by simply running the desktop application or use web page, entering network credentials, and exporting the setting to sd card. 
- Node has good compatibility with printers due to Beam only relying on serial connection for control and monitor and utilizing the high bandwidth SD I/O for transfer.
Faster and more reliable printing via Beam’s unique SD Card I/O bridge to printer
- Node works with or without an external internet connection, and functionalities can be realized via BeamNexus
- Easy to manage multiple printers with a single software control point
- Affordable hardware cost. No need to acquire a Pi for every printer
- Email Notification via BeamNexus 
- Future Expandability with Beam Camera and Machine Vision for Failure Detection via BeamNexus
- Provide simple RestfulAPI to allow other software control (such as postman debugging tools)
- Node includes extended interfaces (IIC, UART, SLR control circuit) to work with ESP32-Camera. This allows for time-lapse photography production, taking photos according to the progress and email reminders, PC local printing failure detection, and smoke flame detection, all of which do not rely on registered accounts and networks.

**Nexus's local printing failure detection:**

<div align=center>
<img src="./Images/1-3-1.jpg" width=800 />
</div>

**Node's lite web UI:**

<div align=center>
<img src="./Images/NodeWebUI.jpg" width=800 />
</div>

**Node installed in Ender3-v2:**

<div align=center>
<img src="./Images/ender3case.jpg" width=800 />
</div>

### 2.Why Beam Node?

When we first look at adding WiFi control to our 3D printers, there are already excellent choices, such as the wonderful [Octoprint](https://github.com/OctoPrint/OctoPrint) project run by our good friend Gina with great plugins and [communities](https://octoprint.org/). Octoprint controls and transfers via serial connection only, which is limited by the speed of the connection and firmware compatibility issues. 

We want to challenge ourselves to see if there is a leaner, MCU based solution that can achieve these goals:

- Allows for faster print speed than serial connection, which is needed for newer printers, especially CoreXY Printer
- More compatibility with different printers
- Easy to install, especially for users who are new to 3D printing
- Simple to set up and manage a fleet of printers.
- Robust operation such that the device can be powered off immediately and does not rely on an Internet connection to function

<div align=center>
<img src="./Images/board-2.jpg" width=600 />
</div>

To meet the above goals, we needed a tightly integrated solution that requires hardware that does not exist on the market. This led us to develop Node’s ESP32 based board and the open-source firmware.  We also designed the unique SD Bridge that allows Beam to transfer GCode via high-speed SD I/O while controlling/monitoring via the serial connection.


## 3.BeamNode-API

BeamNode provides core APIs to enable more platforms(Such as [Octoprint](https://github.com/OctoPrint/OctoPrint),BeamNexus, or postman tools) to access him for control. Listed below are the APIs already included in Node and their specific usage.[API docs](./FP-BeamAPI.md)

## 4.How to update  firmware

Please refer to this [guide](https://docs.google.com/presentation/d/1EhdrxVowPHj--iUrkvdkC4d59uzsc6_Up4gLxd3CpAw/edit?usp=sharing) to complete the firmware burning and updating. 

## 5. Community support

- [Facebook](https://www.facebook.com/Fiberpunk)
- [Discord](https://discord.gg/VNNFrfhsbN)




