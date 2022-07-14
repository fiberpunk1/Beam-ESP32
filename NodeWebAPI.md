## 1. API interface description

This document is used to describe the API design of the Node module.

---

### 1.Find Device API

**Brief description**

- Provide an http API for the client to search Node device, and if Node receives this network request, it will return its own device name
- If it has been connected before, it will return the status of the current device

**Request URL**
- ` http://192.168.1.133:88/find `

**Request Method**
- GET 

**Parameters**

- None

**Return Example**

```
 Beam-Node-Name:192.168.1.133:PRINTING
```
>Note: If it is in the print, only after the PRINTING will return, the first connection, will not return to change the string

---

### 2. Get file directory information

**Brief description**

- Get all the file directory information in the SD card

**Request URL**
- ` http://192.168.1.133:88/list?dir=/ `
  
**Request Method**
- GET

**Parameters**
- None

**Return Example**


```
[
    {"type":"dir","name":"/System Volume Information"},
    {"type":"file","name":"/castle.zip"},
    {"type":"file","name":"/config.txt"}
]
```

**Return error code**

"NOT DIR" No directory is generally caused by the Node not being able to obtain the SD card. This error is returned when the printer is printing on an occupied SD card. If the Node fails to mount the SD card, it will also return a change error.

**Return parameter description**

|Parameter Name|Type|Description|
|:-----:  |:-----:|-----                           |
|type |string   |The current type of this name, file or directory|
|name |string   |File or directory name|

**Remarks**

- The Node file system currently in use does not support folder access, so all print files should be placed under the root directory of the sd card

---

### 3. Upload files

**Brief description**

- User uploads files

**Request URL**
- ` http://192.168.1.133:88/edit `
  
**Request Method**
- POST

**Parameters**

- Uploading files as a form

**Return Example**

- Upload completed, return ok

**Return parameter description**

- None

**Remarks**

- None

---

### 4. Delete files

**Brief description**

- User deletes files from the sd card

**Request URL**
- ` http://192.168.1.133:88/remove?path=/xxx.gcode `

**Request Method**
- DELETE

**Parameters**

- Specify path=/xxx.gcode in the url, which is the file to be deleted

**Return Example**
- Delete completed, return ok

**Return parameter description**
- None

**Return error code**"BAD PATH" means no file exists to change, this error will also appear when Node cannot get the SD card.

**Remarks**
- None

---

### 5. Send PC address to Node, create socket and connect to Nexcus

**Brief description**

- Send the client's IP address to Node, Node will create a socket to connect to the client's socket after receiving it, so that a long connection can be established and Node can actively send information to the client to notify him to do something (need to implement a socket server on the client side, listening to port 1688)

- All serial returns from the printer received by Node are sent out through this socket.

**Request URL**
- ` http://192.168.1.133:88/pcsocket?ip=192.168.1.1 `

**Request Method**
- GET

**Parameters**
- Specify the client's own IP address in the url

**Return Example**
- Create completed, return ok
- socket connection creation failed, return Connect PC failed

**Return parameter description**
- None

**Remarks**
- None

---

### 6. Sending Gcode control commands

**Brief description**

- Send a Gcode command to Node using the url, Node will send it to the printer as soon as it receives it.

**Request URL**
- ` http://192.168.1.133:88/gcode?gc=G28 X Y `

**Request Method**
- GET

**Parameters**
- Specify the Gcode command to be sent in the url

**Return Example**
- Create completed, return ok

**Return parameter description**
- None

**Remarks**
- None

---

### 7. Start printing**Brief description**

- Specify in the url a gcode file (DOS 8.3 format) contained in the sd card to notify Node to print

**Request URL**
- ` http://192.168.1.133:88/print?filename=/xxx.gcode `

**Request Method**
- GET

**Parameters**
- Specify the Gcode command to be sent in the url

**Return Example**
- Execution complete, return ok

**Return parameter description**
- None

**Remarks**
- The files printed are selected from the list of sd card files obtained in the previous
- The file name needs to be in DOS 8.3 format

---

### 8. Control printing pause, resume, cancel, etc.

**Brief description**

- Control printer print,pause,cancel,resume,Node mount SD card,Node unmount SD card by url

**Request URL**
- ` http://192.168.1.133:88/operate?op=PAUSE `

**Request Method**
- GET

**Parameters**
- PAUSE 
- RECOVER
- CANCEL
- GETSD
- RELEASESD

**Return Example**
- Execution complete, return ok

**Return parameter description**
- None

**Return error code**"NO PRINTER" means that the USB connection between the Node's USB and the printer has failed and the USB connection needs to be re-established

**Remarks**
- None

---

### 9. Get the status of the printer

**Brief description**

- Get some status strings of the printer, including: Printhead temperature, hot bed temperature, total number of layers, current number of layers printed
- This can be replaced by sending a query to gcode and then getting the socket data back in real time to know the status of the printer

**Request URL**
- ` http://192.168.1.133:88/status`

**Request Method**
- GET

**Parameters**
- None

**Return Example**
- Execution complete, return ok

**Return parameter description**
- The return is a number of columns of strings, the front-end to get this data, you need to use the regular to get the information they want, a typical return string is as follows:


```
;LAYER_COUNT:90,;LAYER:4,
B: 23.4 /40  T: 100.3/210

```

**Remarks**
- The content returned here is directly returned by the printer, which belongs to direct forwarding, and this url can be sent every few seconds to update the display status of the front-end

### 10. Re-establish USB connection

**Brief description**

- If the 3D printer is plugged into the Node's USB-A and the Node does not automatically detect the USB access, you can use this request to recheck the USB device.

**Request URL**
- ` http://192.168.1.133:88/resetusb`

**Request Method**
- GET

**Parameters**
- None

**Return Example**
- Execution complete, return ok

### 11. Setting up the Node's SD card communication protocol

**Brief description**

- Different 3D printer motherboards use different protocols for SD cards, mainly SDIO and SPI. This API allows user to set which SD card protocol the Node uses. After setting to this, the selection will be recorded and saved by power off.

**Request URL**
- ` http://192.168.1.133:88/setsdtype?type=SPI`

**Request Method**
- GET

**Parameters**
- type
    - "SPI"
    - "SDIO"

**Return Example**
- Execution complete, return ok


### 12. Get Node's firmware version information

**Brief description**

- This API user gets the version information of the Node firmware.

**Request URL**
- ` http://192.168.1.133:88/version`

**Request Method**
- GET

**Parameters**
- None

**Return Example**
- Execution complete, return version information


### 13. Get Node's wifi signal strength

**Brief description**

- This API user gets the rssi of the Node.

**Request URL**
- ` http://192.168.1.133:88/getrssi`

**Request Method**
- GET

**Parameters**
- None

**Return Example**
- RSSI value