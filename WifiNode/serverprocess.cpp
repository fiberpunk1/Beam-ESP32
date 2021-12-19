#include "serverprocess.h"
#include "printerprocess.h"
#include "nodeconfig.h"


const char* host = "EdgeeWifi";
void espGetSDCard();
void espReleaseSD();
void reset559();
void sendCmdByPackage(String cmd);
void sendCmdByPackageNow(String cmd);
void cancleOrFinishPrint();

void returnOK() 
{
  server.send(200, "text/plain", "ok");
}

void returnFail(String msg) 
{
  server.send(500, "text/plain", msg + "\r\n");
}

bool loadFromSdCard(String path) 
{
  return true;
}


void handleFileUpload() 
{
  if (server.uri() != "/edit") 
  {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) 
  {
    if (SD_MMC.exists((char *)upload.filename.c_str())) 
    {
      SD_MMC.remove((char *)upload.filename.c_str());
    }
    uploadFile = SD_MMC.open(upload.filename.c_str(), FILE_WRITE);
    // DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(upload.filename);
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    if (uploadFile) 
    {
      uploadFile.write(upload.buf, upload.currentSize);
    }
    // DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: "); DBG_OUTPUT_PORT.println(upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) 
  {
    if (uploadFile) 
    {
      uploadFile.close();
    }
    // DBG_OUTPUT_PORT.print("Upload: END, Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void deleteRecursive(String path) 
{
  File file = SD_MMC.open((char *)path.c_str());
  if (!file.isDirectory()) 
  {
    file.close();
    SD_MMC.remove((char *)path.c_str());
    return;
  }

  file.rewindDirectory();
  while (true) 
  {
    File entry = file.openNextFile();
    if (!entry) 
    {
      break;
    }
    String entryPath = path + "/" + entry.name();
    if (entry.isDirectory()) 
    {
      entry.close();
      deleteRecursive(entryPath);
    } else 
    {
      entry.close();
      SD_MMC.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SD_MMC.rmdir((char *)path.c_str());
  file.close();
}

void handleDelete() 
{
  if (server.args() == 0) 
  {
    return returnFail("BAD ARGS");
  }
  // String path = server.arg(0);
  String path = server.arg("path");
  // DBG_OUTPUT_PORT.print(path);
  if (path == "/" || !SD_MMC.exists((char *)path.c_str())) 
  {
    returnFail("No SD Card");
    return;
  }
  deleteRecursive(path);
  returnOK();
}

void handleCreate() {
  if (server.args() == 0) 
  {
    return returnFail("BAD ARGS");
  }
  String path = server.arg(0);
  if (path == "/" || SD_MMC.exists((char *)path.c_str())) 
  {
    returnFail("No SD Card");
    return;
  }

  if (path.indexOf('.') > 0) {
    File file = SD_MMC.open((char *)path.c_str(), FILE_WRITE);
    if (file) 
    {
      file.write(0);
      file.close();
    }
  } 
  else 
  {
    SD_MMC.mkdir((char *)path.c_str());
  }
  returnOK();
}

void printDirectory() 
{
  if (!server.hasArg("dir")) 
  {
    return returnFail("BAD ARGS");
  }
  String path = server.arg("dir");
  if (path != "/" && !SD_MMC.exists((char *)path.c_str())) 
  {
    return returnFail("No SD Card!");
  }
  File dir = SD_MMC.open((char *)path.c_str());
  path = String();
  if (!dir.isDirectory()) 
  {
    dir.close();
    return returnFail("NOT DIR");
  }
  dir.rewindDirectory();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  WiFiClient client = server.client();

  server.sendContent("[");
  for(int cnt = 0; true; ++cnt)
   {
    File entry = dir.openNextFile();
    if(!entry) 
    {
      break;
    }

    String output;
    if(cnt > 0)
    {
      output = ',';
    }

    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += entry.name();
    output += "\"";
    output += "}";
    server.sendContent(output);
    entry.close();
  }
  server.sendContent("]");
  dir.close();
}
void sendCmdByPackageNow(String cmd)
{
    cmd_length = cmd.length();
    uint8_t package[cmd_length+4];
    
    char str_buf[cmd_length];
    
    memset(package, ' ', cmd_length+4);  //fill the package with space char

    package[0] = 0xff;         //package head
    package[1] = cmd_length;
    cmd.toCharArray(str_buf, cmd_length);
    memcpy(&package[2], str_buf, cmd_length);
    package[cmd_length+1] = 0x0a;
    package[cmd_length+2] = gcrc.get_crc8((uint8_t const*)(package+2), cmd_length);  //input the crc data
    package[cmd_length+3] = 0xfd; //package tail
    
    //String sendgc((char*)package);
    PRINTER_PORT.write(package,cmd_length+4);
    //PRINTER_PORT.print(sendgc);
    PRINTER_PORT.flush();
}
// send out the cmd package, not just cmd
void sendCmdByPackage(String cmd)
{
  // writeLog("Send:");
  // writeLog(cmd);
  if(current_usb_status)
  {
      cmd_length = cmd.length();
      uint8_t package[cmd_length+4];
      
      char str_buf[cmd_length];
      
      memset(package, ' ', cmd_length+4);  //fill the package with space char

      package[0] = 0xff;         //package head
      package[1] = cmd_length;
      cmd.toCharArray(str_buf, cmd_length);
      memcpy(&package[2], str_buf, cmd_length);
      package[cmd_length+1] = 0x0a;
      package[cmd_length+2] = gcrc.get_crc8((uint8_t const*)(package+2), cmd_length);  //input the crc data
      package[cmd_length+3] = 0xfd; //package tail
      
      //String sendgc((char*)package);
      PRINTER_PORT.write(package,cmd_length+4);
      //PRINTER_PORT.print(sendgc);
      PRINTER_PORT.flush();
  }
  else
  {
    returnFail("NO PRINTER");
  }

}

void handleNotFound() 
{
  if (hasSD && loadFromSdCard(server.uri())) 
  {
    return;
  }
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) 
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  // DBG_OUTPUT_PORT.print(message);
}
void reportDevice()
{
  String ip = "Beam-"+cf_node_name + ":" + WiFi.localIP().toString();
   if(g_status==PRINTING)
  {
    ip = ip + ":" + "PRINTING";
  }
  server.send(200, "text/plain",ip);
}
void reportVersion()
{
  String version = "version:"+String(VERSION);
  server.send(200, "text/plain",version);
}
void printerStatus()
{
  sendCmdByPackage("M105\n");
  delay(200);
  if(g_status==PRINTING)
  {
    sendCmdByPackage("M27\n");
    delay(200);
  }
  String result = current_temp+","+current_bed_temp+","+current_layers; 
  server.send(200, "text/plain",result);
}

void reset559()
{
  digitalWrite(5, LOW);
  delay(50);
  digitalWrite(5, HIGH);
  delay(500);
  digitalWrite(5, LOW);
}


void resetUSBHost()
{
  reset559();
  returnOK();
}

void cancleOrFinishPrint()
{
    String finish_cmd = "Beam-"+cf_node_name+"-Finish";
    g_status = CANCLE;
    g_status = P_IDEL;
    recv_ok = false;
    recvl_ok = false;
    sendCmdByPackage("M524\n");
    current_bed_temp = "";
    current_layers = "";
    current_temp = "";
    espGetSDCard();
    sendHttpMsg(finish_cmd);
    // if(socket_client.connected())
    //   socket_client.stop();
}

void printerControl()
{
  if (!server.hasArg("op")) 
  {
    return returnFail("No Command");
  }
  String op = server.arg("op");

  if(current_usb_status)
  {
      if(op=="PAUSE")
      {
        g_status = PAUSE;
        sendCmdByPackage("M0\n");

      }
      else if(op=="CANCLE")
      {
        cancleOrFinishPrint();
      }
      else if(op=="RECOVER")
      {
        if(g_status == PAUSE)
        {
          g_status = PRINTING;
          // PRINTER_PORT.print("G4 S1.0\n");
          sendCmdByPackage("M108\n");
        }

      }
      else if(op=="GETSD")
      {
        espGetSDCard();
      }
      else if(op=="RELEASESD")
      {
        espReleaseSD();
      }
      returnOK();
  }
  else
  {
    returnFail("NO PRINTER");
  }
  


}

void getPCAddress()
{
    if (!server.hasArg("ip")) 
    {
        return returnFail("BAD ARGS");
    }
    pc_ipaddress = server.arg("ip");
    if (!socket_client.connect(pc_ipaddress.c_str(), 1688)) 
    {
        return returnFail("Connect PC failed!");
    }

    returnOK();
}


void sendGcode()
{
  if(current_usb_status)
  {
    if (!server.hasArg("gc")) 
    {
        return returnFail("No ARGS");
    }
    String op = server.arg("gc")+"\n";
    if(op.startsWith("G0"))
    {
      sendCmdByPackage("G91\n");
      delay(100);
      sendCmdByPackage(op); 
      delay(100);
      sendCmdByPackage("G90\n");
    }
    else
    {
      sendCmdByPackage(op);  
    }
    
    returnOK();
  }
  else
  {
    returnFail("NO PRINTER");
  }
  
}

void printStart()
{
  if(current_usb_status)
  {
    if (!server.hasArg("filename")) 
    {
      return returnFail("No file");
    }
    reset559();
    delay(500);
    espReleaseSD();
    delay(500);
    String path = server.arg("filename");
    if(g_status==P_IDEL)
    {
      if (!socket_client.connect(pc_ipaddress.c_str(), 1688)) 
      {
          return returnFail("Connect PC failed!");
      }
      String print_cmd = "M23 "+path+"\n";
      sendCmdByPackage(print_cmd);
      delay(200);
      sendCmdByPackage("M24\n");
      g_status = PRINTING;

    }
    returnOK();
  }
  else
  {
    returnFail("NO PRINTER");
  }
  
}

ServerProcess::ServerProcess()
{

}

void ServerProcess::serverInit()
{
    String pre_line="";
        //log file


    File g_printfile;
    OP_STATUS g_status=P_IDEL;
    ERROR_CODE g_error=NORMAL;

    if (MDNS.begin(host)) 
    {
        MDNS.addService("fiberpunk", "tcp", 23);
        // DBG_OUTPUT_PORT.println("MDNS responder started");
        // DBG_OUTPUT_PORT.print("You can now connect to http://");
        // DBG_OUTPUT_PORT.print(host);
        // DBG_OUTPUT_PORT.println(".local");
    }

    server.on("/find", HTTP_GET, reportDevice);
    server.on("/version", HTTP_GET, reportVersion);
    server.on("/gcode", HTTP_GET, sendGcode);
    server.on("/pcsocket", HTTP_GET, getPCAddress);
    server.on("/resetusb",HTTP_GET,resetUSBHost);

    // server.on("/capture", HTTP_GET, testCaptureImage);

    //  server.on("/rename", HTTP_GET, deviceRename);
    server.on("/operate", HTTP_GET, printerControl);
    server.on("/status", HTTP_GET, printerStatus);

    server.on("/print", HTTP_GET, printStart);
    server.on("/list", HTTP_GET, printDirectory);
    server.on("/remove", HTTP_DELETE, handleDelete);
    server.on("/edit", HTTP_PUT, handleCreate);
    server.on("/edit", HTTP_POST, []() {
        returnOK();
    }, handleFileUpload);
    server.onNotFound(handleNotFound);

  

  server.begin();

  // DBG_OUTPUT_PORT.println("HTTP server started");

}


void espReleaseSD()
{
    //1.release SD card
    pinMode(2, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(12, INPUT_PULLUP);
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
    pinMode(15, INPUT_PULLUP);

    digitalWrite(18, HIGH);  
    delay(500);
  
    //2.send gcode to marlin, init and reload the sd card
    sendCmdByPackage("M21\n");
    delay(500);  
    SD_MMC.end();  
}

void espGetSDCard()
{
    //0. release sd from marlin
    sendCmdByPackage("M22\n");
    delay(500);
    digitalWrite(18, LOW); 

    //1.初始化SD
    while(!SD_MMC.begin())
    {
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
      
        delay(500);
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);  
        delay(500);
        //break;      
    } 
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE)
    {
        //Serial.println("No SD_MMC card attached");
        //return;
    }  
}

void ServerProcess::serverLoop()
{
    server.handleClient();
    if (rst_usb == true)
    {
        reset559();
        rst_usb = false;
    }
    if (g_status==PRINTING)
    {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, LOW);
    }
    else
    {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);
    }
    if(paused_for_user)
    {
      paused_for_user = false;
      sendCmdByPackage("M108\n");
    }
}
