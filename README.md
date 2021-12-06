

<p align="center"><img src="./Images/logo.png" alt="FiberPunk's logo" /></p>

<h1 align="center">Beam-ESP32</h1>
<p align="center">
  <img src="https://img.shields.io/badge/arduino-opensource-brightgreen" alt="opensource"/>
   <img src="https://img.shields.io/badge/hardware-lowcost-blue" alt="lowcost"/>

</p>

## 1. Project catalog description

- Dependencies: Espressif System 1.0.6

## 2. Beam Project Introduction

![ Img ](./Images/hardware.jpg)

### 2.1 What is Beam?

Beam is a module for FDM printers with wifi file transfer and wifi control printing functions. Unlike Raspberry Pi, Beam uses a simpler microprocessor to handle these tasks. It includes such features as:

- Really easy to config (Plug & Play)
- Do not rely on the Internet, do not rely on any cloud functions
- Faster and more stable data transfer with the printer (SD card printing)
- Good device compatibility, no need to do any configuration, can be compatible with most models (thanks to UsbHost SD technology)
- It is very easy to expand multiple devices, one PC manages many different type of 3D printers from different manufacturers
- Lower hardware cost investment
- With BeamManager, you can realize the progress of email reminders
- Provide a simple Restful API to let other software control (such as postman debugging tools)
- Includes expansion interface (IIC, UART, SLR control circuit)
- With ESP32-Camera, it can realize time-lapse photography production, take photos according to schedule and email reminders, PC local printing failure detection and smoke and flame detection, all of which do not rely on registered accounts and networks

In order to better realize its own functions, Beam has developed its own electronic hardware and established an open source community. Customized hardware can enable users to get a better experience, rather than pieced together functions. Beam is a complete solution. The combination of software and hardware is more harmonious. Only when both are taken into account at the same time can the out-of-the-box experience be achieved.

### 2.Why Beam?

Why do we do the Beam project? In summary, there are several reasons:

- The requirements for installing and using Octoprint on Raspberry Pi are still very high for most people.
- We don’t want to disassemble the machine and then wire it so that the 3D printer has some additional functions
- We don't want to use the cloud server to monitor and upload our print files to other people's servers
- Raspberry Pi is not cheap. When there are multiple printers, the workload is multiple times.
- When we have multiple 3D printers of different models and manufacturers, it is very troublesome to manage them uniformly
- Cloud as a Service is expensive, especially for AI inference servers. We hope to use the monitoring function, using the local PC CPU resources to achieve AI monitoring technology

Using Beam, you can perfectly avoid the problems listed above and implement Plug & Play.

## 3. Quick start

### 3.1 The network requirements of the host computer

Please make sure you have 2.4G network, ESP32 can only connect 2.4G wifi at present. In addition, 360 or some antivirus software, the computer set static IP, VPN, etc., will have an impact on the device search. If you want to use an automated search device, make sure that your computer's IP network segment is consistent with the router's network segment.

### 3.2 Quick start

1. Distribution network

After inserting the SD card, configure the SD card

![ Img ](./Images/gif/1.config.gif)

2. LAN device scanning

![ Img ](./Images/gif/2.find-devices.gif)

3. Control printing

Control panel and upload files:

![ Img ](./Images/gif/3.control-pannel.gif)

4. Print

![ Img ](./Images/gif/5.print.gif)

5. Mail configuration

The mail can be configured to print out the percentage, take a picture, and send a reminder email to the specified mailbox. Printing failure reminder is when the software detects the probability of printing failure, send an email to notify the user (with Camera shooting).

![ Img ](./Images/email.png)

## 4.Beam-API

Beam provides the core API to allow more platforms to access his control. Below is a list of the APIs that Beam already includes and how to use them.[API docs](./FP-BeamAPI.md)

## 5.How to update Beam-ESP32 firmware

1. Download [flash tool](https://github.com/fiberpunk1/Beam-ESP32/releases/download/Beta-v0.1.0/BeamFlash-Installer.exe)
2. On the Beam-ESP32 release page, download the first .bin file
3. Burn the firmware in the order shown in the figure below:

![ Img ](./Images/update-bin.png)



