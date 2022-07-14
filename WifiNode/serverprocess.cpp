#include "serverprocess.h"
#include "printerprocess.h"
#include "nodeconfig.h"

const char* host = "EdgeeWifi";
void hardwareReleaseSD();
void espGetSDCard();
void espReleaseSD();
void reset559();
void sendCmdByPackage(String cmd);
void sendCmdByPackageNow(String cmd);
void cancleOrFinishPrint();

//#define NEPTUNE 1


extern void Write_String(int a,int b,String str);
extern void writeLog(String);

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void returnOK(AsyncWebServerRequest *request){
  request->send(200, "text/plain", "ok");
}

//void returnFail(String msg) 
//{
//  server.send(500, "text/plain", msg + "\r\n");
//}
void eventHandle(AsyncEventSourceClient *client){
      if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello! I get you data", NULL, millis(), 10000);
}

bool loadFromSdCard(String path, AsyncWebServerRequest *request) {
  String dataType = "text/plain";
  if (path.endsWith("/")) {
    path += "index.htm";
  }

  if (path.endsWith(".src")) {
    path = path.substring(0, path.lastIndexOf("."));
  } else if (path.endsWith(".htm")) {
    dataType = "text/html";
  } else if (path.endsWith(".css")) {
    dataType = "text/css";
  } else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  } else if (path.endsWith(".png")) {
    dataType = "image/png";
  } else if (path.endsWith(".gif")) {
    dataType = "image/gif";
  } else if (path.endsWith(".jpg")) {
    dataType = "image/jpeg";
  } else if (path.endsWith(".ico")) {
    dataType = "image/x-icon";
  } else if (path.endsWith(".xml")) {
    dataType = "text/xml";
  } else if (path.endsWith(".pdf")) {
    dataType = "application/pdf";
  } else if (path.endsWith(".zip")) {
    dataType = "application/zip";
  }

  File dataFile = SPIFFS.open(path.c_str());
  if (dataFile.isDirectory()) {
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SPIFFS.open(path.c_str());
  }

  if (!dataFile) {
    return false;
  }
  request->send(dataFile, path, dataType); 
  return true;
}


void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *t_data, size_t len, bool final){
  if (request->url() != "/edit") {
    return;
  }
  //start
  if (!index) {
    if(printer_sd_type==0)
    {
      if (SD.exists((char *)filename.c_str())) 
      {
        SD.remove((char *)filename.c_str());
      }
      uploadFile = SD.open(filename.c_str(), FILE_WRITE);
    }
    else if(printer_sd_type==1)
    {
      if (SD_MMC.exists((char *)filename.c_str())) 
      {
        SD_MMC.remove((char *)filename.c_str());
      }
      uploadFile = SD_MMC.open(filename.c_str(), FILE_WRITE);
    }

    DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(filename);
  } 

  //write
  if (len) {
    if (uploadFile) {
      uploadFile.write(t_data, len);
    }
    DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: "); DBG_OUTPUT_PORT.println(len);
  }

  //finish
  if (final){
    if (uploadFile) {
      uploadFile.close();
    }
    String logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    DBG_OUTPUT_PORT.print(logmessage); 
  }
}

void deleteRecursive(String path) 
{
  if(printer_sd_type==0)
  {
    File file = SD.open((char *)path.c_str());
    if (!file.isDirectory()) 
    {
      file.close();
      SD.remove((char *)path.c_str());
    }
  }
  else if(printer_sd_type==1)
  {
    File file = SD_MMC.open((char *)path.c_str());
    if (!file.isDirectory()) 
    {
      file.close();
      SD_MMC.remove((char *)path.c_str());
    }
  }
}

void handlerRemove(AsyncWebServerRequest *request) {
    Serial.println("HANDLE DELETE");
  if (!request->hasArg("path")) {
    request->send(500, "text/plain", "BAD ARGS");
    Serial.println("no path arg");
  }
  else{
      AsyncWebParameter* p = request->getParam(0);
      String path = p->value();
      
      if(printer_sd_type==0)
      {
        if (path == "/" || !SD.exists((char *)path.c_str())) 
        {
          request->send(500, "text/plain", "BAD PATH");
          Serial.println("path not exists");
        }else
        {
          deleteRecursive(path);
          Serial.println("send ok"); 
          request->send(200, "text/plain", "ok");
        }
      }
      else if(printer_sd_type==1)
      {
        if (path == "/" || !SD_MMC.exists((char *)path.c_str())) 
        {
          request->send(500, "text/plain", "BAD PATH");
          Serial.println("path not exists");
        }else
        {
          deleteRecursive(path);
          Serial.println("send ok"); 
          request->send(200, "text/plain", "ok");
        }
      }


  }
}


void printDirectory(AsyncWebServerRequest * request) {

  int params = request->params();
  if (params == 0) {
    request->send(500, "text/plain","BAD ARGS");
    return;
  }
  AsyncWebParameter* p = request->getParam(0);
  String path = p->value();

  if(printer_sd_type==0)
  {
    if (path != "/" && !SD.exists((char *)path.c_str())) {
      request->send(500, "text/plain","BAD PATH");
      return;
    }

    writeLog(cf_node_name+"SD Type: SPI");
    File dir = SD.open((char *)path.c_str());
    path = String();
    if (!dir.isDirectory()) {
      dir.close();
      request->send(500, "text/plain", "NOT DIR");
      return;
    }
    dir.rewindDirectory();
    

    String output = "[";
    for (int cnt = 0; true; ++cnt) {
      File entry = dir.openNextFile();
      if (!entry) {
        break;
      }
      if (cnt > 0) {
        output += ',';
      }
      output += "{\"type\":\"";
      output += (entry.isDirectory()) ? "dir" : "file";
      output += "\",\"name\":\"";
      output += entry.name();
      output += "\"";
      output += "}";
      entry.close();
    }
    output += "]";
    request->send(200, "text/json", output);
    dir.close();
    return;
  }
  else if(printer_sd_type==1)
  {
    writeLog(cf_node_name+"SD Type: SDIO");
    if (path != "/" && !SD_MMC.exists((char *)path.c_str())) {
      request->send(500, "text/plain","BAD PATH");
      return;
    }
    File dir = SD_MMC.open((char *)path.c_str());
       path = String();
    if (!dir.isDirectory()) {
      dir.close();
      request->send(500, "text/plain", "NOT DIR");
      return;
    }
    dir.rewindDirectory();
    

    String output = "[";
    for (int cnt = 0; true; ++cnt) {
      File entry = dir.openNextFile();
      if (!entry) {
        break;
      }
      if (cnt > 0) {
        output += ',';
      }
      output += "{\"type\":\"";
      output += (entry.isDirectory()) ? "dir" : "file";
      output += "\",\"name\":\"";
      output += entry.name();
      output += "\"";
      output += "}";
      entry.close();
    }
    output += "]";
    request->send(200, "text/json", output);
    dir.close();
  }
  else
  {
    writeLog(cf_node_name+"SD Type:"+String(printer_sd_type));
    request->send(500, "text/plain", "NOT SD");
  }
  


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

}

void handleNotFound(AsyncWebServerRequest *request) {
  if (loadFromSdCard(request->url(), request)) {
      if(!last_power_status)
      {
        reset_sd_559 = 1;
      }
      else
      {
        rst_usb = true;
      }
    return;
  }
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += "\n";
  request->send(404, "text/plain", message);
  DBG_OUTPUT_PORT.print(message);
}

void reportDevice(AsyncWebServerRequest *request)
{
  String ip = "Beam:"+cf_node_name + ":" + WiFi.localIP().toString();
  if(g_status==PRINTING)
  {
    ip = ip + ":" + "PRINTING";
  }
//  if(!last_power_status)
//  {
//    reset_sd_559 = 1;
//  }
//  else
//  {
//    rst_usb = true;
//  }
  request->send(200, "text/plain",ip);
}
void reportVersion(AsyncWebServerRequest *request)
{
  String version = "version:"+String(VERSION);
  request->send(200, "text/plain",version);
}
void printerStatus(AsyncWebServerRequest *request)
{
  sendCmdByPackage("M105\n");
  delay(200);
  if(g_status==PRINTING)
  {
    sendCmdByPackage("M27\n");
    delay(200);
    sendCmdByPackage("M27 C\n");
  }
  String result = current_temp+","+current_bed_temp+","+current_layers; 
  request->send(200, "text/plain",result);
}

void reset559()
{
  digitalWrite(5, LOW);
  delay(50);
  digitalWrite(5, HIGH);
  delay(500);
  digitalWrite(5, LOW);
}



void resetUSBHost(AsyncWebServerRequest *request)
{
  reset559();
  request->send(200, "text/plain","ok");
}

void cancleOrFinishPrint()
{
    saveCurrentPrintStatus("NONE");
    last_power_status = 0;
    String finish_cmd = cf_node_name+":Finish";

    g_status = P_IDEL;
    recv_ok = false;
    recvl_ok = false;

    sendCmdByPackage("M524\n");
    delay(50);
    sendCmdByPackage("M118 @Stop\n");

#ifdef NEPTUNE
    delay(50);
    sendCmdByPackage("M25\n");
    delay(50);
    reset_sd_559 = 1;
    
#endif
    // espGetSDCard();
    sendHttpMsg(finish_cmd);
    current_bed_temp = "";
    current_layers = "";
    current_temp = "";
    
    // if(socket_client.connected())
    //   socket_client.stop();
}

void setSDType(AsyncWebServerRequest *request)
{
  if (!request->hasArg("type")) 
  {
    request->send(500, "text/plain", "NO Command");
  }
  else
  {
      AsyncWebParameter* p = request->getParam(0);
      String op = p->value();
      Write_String(17,150,op);
      delay(100);
      request->send(200, "text/plain","ok");
      ESP.restart(); 
  }
}

void printerControl(AsyncWebServerRequest *request)
{
  if (!request->hasArg("op")) 
  {
    request->send(500, "text/plain", "NO Command");
  }
  else
  {
      AsyncWebParameter* p = request->getParam(0);
      String op = p->value();

      // if(current_usb_status)
      {
          if(op=="PAUSE")
          {
            g_status = PAUSE;
            #ifdef NEPTUNE
              sendCmdByPackage("M25\n");
            #endif
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
            #ifdef NEPTUNE
              sendCmdByPackage("M24\n");
            #endif
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
          request->send(200, "text/plain","ok");
      }
      // else
      // {
      //   request->send(500, "text/plain","NO PRINTER");
      // }
  }
  
  
 
  


}

void getPCAddress(AsyncWebServerRequest *request)
{
    if (!request->hasArg("ip")) 
    {
        request->send(500, "text/plain", "BAD ARGS");
    }
    else
    {
      AsyncWebParameter* p = request->getParam(0);
      pc_ipaddress = p->value();
      if (!socket_client.connected()) 
      {
         if(socket_client.connect(pc_ipaddress.c_str(), 1688))
         {
          request->send(200, "text/plain", "ok");
         }
         else
        { 
          request->send(500, "text/plain", "Connect PC failed!");
        } 
         
      }
      else
      {
        request->send(200, "text/plain", "ok");
       }

    }
}

void getRSSI(AsyncWebServerRequest *request)
{
  int rssi =  0;
  rssi = WiFi.RSSI();
  String rssi_str = "RSSI:"+String(rssi);
  request->send(200, "text/plain", rssi_str);
}

void sendGcode(AsyncWebServerRequest *request)
{
  if(current_usb_status)
  {
    if (!request->hasArg("gc")) 
    {
        request->send(500, "text/plain", "NO ARGS");
    }
    else
    {
        AsyncWebParameter* p = request->getParam(0);
        String op = p->value()+"\n";
        
        if(op.startsWith("G0"))
        {
          sendCmdByPackage("G91\n");
          delay(100);
          sendCmdByPackage(op); 
          delay(100);
          sendCmdByPackage("G90\n");
        }
        else if(op.startsWith("M115"))
        {
          String node_version = "Node version:"+String(VERSION);
          events.send(node_version.c_str(), "gcode_cli");
          sendCmdByPackage(op);
        }
        else
        {
          sendCmdByPackage(op);  
        }
        request->send(200, "text/plain", "ok");
    }
  }
  else
  {
    request->send(500, "text/plain", "NO PRINTER");
  }
   
}

void printStart(AsyncWebServerRequest * request)
{
 if(current_usb_status)
 {
    if (!request->hasArg("filename")) 
    {
      request->send(500, "text/plain", "NO FILE");
    }
    else
    {
        AsyncWebParameter* p = request->getParam(0);
        String path = p->value();
        path.toLowerCase();
        current_file = path;
        if(g_status==P_IDEL)
        {
           print_start_flag = 1;
           request->send(200, "text/plain", "ok"); 
        }
        else
        {
          request->send(500, "text/plain", "PRINTER BUSY"); 
        }
         
    }
    
  }
  else
  {
    request->send(500, "text/plain", "NO PRINTER");
  }
  
}

void printStartInstance()
{
  reset559();
  delay(250);
  espReleaseSD();
  delay(200); 
  String print_cmd = "M23 "+current_file+"\n";
  sendCmdByPackage(print_cmd);
  delay(200);
  sendCmdByPackage("M24\n");
  g_status = PRINTING;
  saveCurrentPrintStatus("PRINTING");
  last_power_status = 1;

}

void espRestart(AsyncWebServerRequest *request)
{
   request->send(200, "text/plain", "ok");
   ESP.restart(); 
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
    server.on("/getrssi",HTTP_GET,getRSSI);
    server.on("/resetusb",HTTP_GET,resetUSBHost);
    server.on("/esprestart", HTTP_GET, espRestart);
    server.on("/setsdtype", HTTP_GET, setSDType);

    // server.on("/capture", HTTP_GET, testCaptureImage);

    //  server.on("/rename", HTTP_GET, deviceRename);
    server.on("/operate", HTTP_GET, printerControl);
    server.on("/status", HTTP_GET, printerStatus);

    server.on("/print", HTTP_GET, printStart);
    server.on("/list", HTTP_GET, printDirectory);
    server.on("/remove", HTTP_GET, handlerRemove);
    server.on("/edit", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
    }, handleFileUpload);
    server.onNotFound(handleNotFound);
  server.addHandler(&events);
  
  server.begin();

  // DBG_OUTPUT_PORT.println("HTTP server started");

}

void hardwareReleaseSD()
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
    if(printer_sd_type==0)
    {
      SD.end();  
    }
    else if(printer_sd_type==1)
    {
      SD_MMC.end();
    } 
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

  if(printer_sd_type==0)
  {
    SD.end();  
  }
  else if(printer_sd_type==1)
  {
    SD_MMC.end();
  }
    digitalWrite(18, HIGH);  
    delay(50);
    
    //2.send gcode to marlin, init and reload the sd card
    sendCmdByPackage("M21\n");
    delay(50);  
    
}

void espGetSDCard()
{
    //0. release sd from marlin
    sendCmdByPackageNow("M22\n");
    delay(50);
    digitalWrite(18, LOW); 
    delay(50);
    //1.初始化SD
    writeLog(cf_node_name+"SD Type:"+String(printer_sd_type));

    if(printer_sd_type==0)
    {   
        SPI.begin(14,2,15,13);
        int sd_get_count = 0;
        while((!SD.begin(13,SPI,4000000,"/sd",5,false))&&(sd_get_count<5))
        {
            digitalWrite(RED_LED, LOW);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(BLUE_LED, HIGH);

            // DBG_OUTPUT_PORT.println("Not Found SD Card.");
            // message_display("Not Found SD Card.");        
            delay(500);
            digitalWrite(RED_LED, HIGH);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(BLUE_LED, HIGH);  
            delay(500);
            sd_get_count++;
    //        break;      
        } 
    }
    else if(printer_sd_type==1)
    {
        int sd_get_count = 0;
        while((!SD_MMC.begin())&&(sd_get_count<5))
        {
            digitalWrite(RED_LED, LOW);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(BLUE_LED, HIGH);

            // message_display("Not Found SD Card.");        
            delay(500);
            digitalWrite(RED_LED, HIGH);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(BLUE_LED, HIGH);  
            delay(500);
            sd_get_count++;
        }     
    }
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
   
}

void filamentDetect()
{
  if((cf_filament==1)&&((!paused_for_filament)))
  {
    if(digitalRead(19)==HIGH)
    {
      delay(100);
      if(digitalRead(19)==HIGH)  
      {
        sendCmdByPackage("M600 X0 Y0\n");
        String filament_cmd = cf_node_name+"-FilamentOut";
        sendHttpMsg(filament_cmd);
        paused_for_filament = true;
      }
    }  
  }  
}


void ServerProcess::serverLoop()
{
//    server.handleClient();
    if(rst_usb == true)
    {
        reset559();
        rst_usb = false;
    }
    if(print_start_flag)
    {
      print_start_flag = 0;
      printStartInstance();
    }
    if (g_status==PRINTING)
    {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, LOW);
      filamentDetect();
    }
    else
    {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);

    }
    if(paused_for_user&&(!paused_for_filament))
    {
      paused_for_user = false;
      sendCmdByPackage("M108\n");
    }
}
