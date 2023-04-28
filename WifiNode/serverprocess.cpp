#include "serverprocess.h"
#include "printerprocess.h"
#include "nodeconfig.h"
#include "ArduinoJson.h"

#include "apconfig.h"

const char* host = "Fiberpunk";
void hardwareReleaseSD();
void espGetSDCard();
void espReleaseSD();
void resetUsbHostInstance();
void sendCmdByPackage(String cmd);
void sendCmdByPackageNow(String cmd);
void cancleOrFinishPrint();
void getFMDfile();


extern void pageDisplayIP(String ip,String content);
extern void writeString(int a,int b,String str);
extern void writeLog(String);

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void returnOK(AsyncWebServerRequest *request){
  request->send(200, "text/plain", "ok");
}


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

  if(g_status==PRINTING){
    request->send(500, "text/plain", "UPLOAD OPEN SF FAILED");
    return;
  }

  //start
  if (!index) {
    espGetSDCard();
    if(uploadFile){
        uploadFile.close();
    }
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
      int written_len = uploadFile.write(t_data, len);
      if(written_len != len){
        DBG_OUTPUT_PORT.print("Faile written: WRITE, Bytes: "); 
        DBG_OUTPUT_PORT.println(written_len);    
      }
    }
    else
    {
       request->send(500, "text/plain", "UPLOAD OPEN SF FAILED");
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
    espReleaseSD();
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
      espGetSDCard();
      if(printer_sd_type==0)
      {
        if (path == "/" || !SD.exists((char *)path.c_str())) 
        {
          espReleaseSD();
          request->send(500, "text/plain", "BAD PATH");
          Serial.println("path not exists");
        }else
        {
          deleteRecursive(path);
          Serial.println("send ok"); 
          espReleaseSD();
          request->send(200, "text/plain", "ok");
        }

      }
      else if(printer_sd_type==1)
      {
        if (path == "/" || !SD_MMC.exists((char *)path.c_str())) 
        {
          espReleaseSD();
          request->send(500, "text/plain", "BAD PATH");
          Serial.println("path not exists");
        }else
        {
          deleteRecursive(path);
          Serial.println("send ok"); 
          espReleaseSD();
          request->send(200, "text/plain", "ok");
        }
      }


  }
}

String readFMDfile(String file_name)
{
  String retmd = "";
  if(file_name.startsWith("/fmd")&&(file_name.endsWith("txt")))
  {
    File fmd_file;
    if(printer_sd_type==0)
        fmd_file = SD.open(file_name,FILE_READ);
    else if(printer_sd_type==1)
        fmd_file = SD_MMC.open(file_name,FILE_READ);

    while(fmd_file.available())
    {
      String ret = fmd_file.readStringUntil('\n');
      ret.replace("\r", "");
      retmd = retmd+ret+"#";
    }
    retmd=retmd+"G4 P50";
  }
  return retmd;
  
}

void readAllFDMcmd()
{
  if(user1_cmd_f_name.length()>6)
  {
    user1_cmd = readFMDfile(user1_cmd_f_name);
    writeLog(user1_cmd);
  }

  if(user2_cmd_f_name.length()>6)
  {
    user2_cmd = readFMDfile(user2_cmd_f_name);
    writeLog(user2_cmd);
  }

  if(user3_cmd_f_name.length()>6)
  {
    user3_cmd = readFMDfile(user3_cmd_f_name);
    writeLog(user3_cmd);
  }

  if(user4_cmd_f_name.length()>6)
  {
    user4_cmd = readFMDfile(user4_cmd_f_name);
    writeLog(user4_cmd);
  }
}

void getFMDfile()
{
  String path = "/";
  if(printer_sd_type==0)
  {
    if (path != "/" && !SD.exists((char *)path.c_str())) {
      return;
    }

    File dir = SD.open((char *)path.c_str());
    path = String();
    if (!dir.isDirectory()) {
      dir.close();
    }
    dir.rewindDirectory();
    
    for (int cnt = 0; true; ++cnt) 
    {
      File entry = dir.openNextFile();
      if (!entry) {
        break;
      }
      if(!entry.isDirectory())
      {
        String fmd_name = entry.name();
        Serial.println(fmd_name);
        int fmd_name_len = fmd_name.length();
        if(fmd_name.startsWith("/fmd")&&(fmd_name.endsWith("txt")))
        {
            String count = fmd_name.substring(fmd_name_len-5,fmd_name_len-4);
            int c_count = count.toInt();
            if(c_count==1)
            {
                user1_cmd_f_name = fmd_name;
            }
            else if(c_count==2)
            {
                user2_cmd_f_name = fmd_name;
            }
            else if(c_count==3)
            {
                user3_cmd_f_name = fmd_name;
            }
            else if(c_count==4)
            {
                user4_cmd_f_name = fmd_name;
            }
        }
      }
      
      entry.close();
    }
    dir.close();
  }
  else if(printer_sd_type==1)
  {
    if (path != "/" && !SD_MMC.exists((char *)path.c_str())) {
      return;
    }

    File dir = SD_MMC.open((char *)path.c_str());
    path = String();
    if (!dir.isDirectory()) {
      dir.close();
    }
    dir.rewindDirectory();
    
    for (int cnt = 0; true; ++cnt) 
    {
      File entry = dir.openNextFile();
      if (!entry) {
        break;
      }
      if(!entry.isDirectory())
      {
        String fmd_name = entry.name();
        int fmd_name_len = fmd_name.length();
        if(fmd_name.startsWith("/fmd")&&(fmd_name.endsWith("txt")))
        {
            String count = fmd_name.substring(fmd_name_len-5,fmd_name_len-4);
            int c_count = count.toInt();
            if(c_count==1)
            {
                user1_cmd_f_name = fmd_name;
            }
            else if(c_count==2)
            {
                user2_cmd_f_name = fmd_name;
            }
            else if(c_count==3)
            {
                user3_cmd_f_name = fmd_name;
            }
            else if(c_count==4)
            {
                user4_cmd_f_name = fmd_name;
            }
        }
      }
      
      entry.close();
    }
    dir.close();
  }
  readAllFDMcmd();

}

void getFMDBtnName(AsyncWebServerRequest * request)
{
  String fmd_btn = "##"+user1_cmd_f_name + ";" + user2_cmd_f_name + ";" + user3_cmd_f_name + ";" +user4_cmd_f_name + "&&";
  events.send(fmd_btn.c_str(), "gcode_cli");
  request->send(200, "text/plain", "ok");
}
void sendUserGcodeCmd(String usrgcodes)
{
    for(int i=0; i<8; i++)
    {
      String gcode_cmd = getValue(usrgcodes,'#',i);
      if(gcode_cmd.length()<2){
        break;
      }
      else{
        writeLog("user:"+gcode_cmd);
        sendCmdByPackage(gcode_cmd+"\n"); 
        delay(100);
      }
    }
}
void user1Btn(AsyncWebServerRequest * request)
{
  if(user1_cmd_f_name.length()>6)
  {
    sendUserGcodeCmd(user1_cmd);
  }
  request->send(200, "text/plain", "ok");
}

void user2Btn(AsyncWebServerRequest * request)
{
  if(user2_cmd_f_name.length()>6)
  {
    sendUserGcodeCmd(user2_cmd);
  }
  request->send(200, "text/plain", "ok");
}


void user3Btn(AsyncWebServerRequest * request)
{
  if(user3_cmd_f_name.length()>6)
  {
    sendUserGcodeCmd(user3_cmd);
  }
  request->send(200, "text/plain", "ok");
}


void user4Btn(AsyncWebServerRequest * request)
{
  if(user4_cmd_f_name.length()>6)
  {
    sendUserGcodeCmd(user4_cmd);
  }
  request->send(200, "text/plain", "ok");
}



void printDirectory(AsyncWebServerRequest * request) {

  int params = request->params();
  if (params == 0) {
    request->send(500, "text/plain","BAD ARGS");
    return;
  }
  AsyncWebParameter* p = request->getParam(0);
  String path = p->value();

  if(g_status==PRINTING)
  {
    request->send(500, "text/plain","PRINTER BUSY");
    return;
  }


  {
    espGetSDCard();

      if(last_power_status)
      {
          writeLog(cf_node_name+" Node init without SD");
      }
      else{
        writeLog(cf_node_name+" Node init with SD");
      }
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
          espReleaseSD();
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
          output += ",\"size\":\"";
          output += String(entry.size());
          output += "\"";
          output += "}";
          entry.close();
        }
        output += "]";
        request->send(200, "text/json", output);
        dir.close();
        espReleaseSD();
        return;
      }
      else if(printer_sd_type==1)
      {
        writeLog(cf_node_name+"SD Type: SDIO");
        if (path != "/" && !SD_MMC.exists((char *)path.c_str())) {
          request->send(500, "text/plain","BAD PATH");
          espReleaseSD();
          return;
        }
        File dir = SD_MMC.open((char *)path.c_str());
          path = String();
        if (!dir.isDirectory()) {
          dir.close();
          request->send(500, "text/plain", "NOT DIR");
          espReleaseSD();
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
          output += ",\"size\":\"";
          output += String(entry.size());
          output += "\"";
          output += "}";
          entry.close();
        }
        output += "]";
        request->send(200, "text/json", output);
        dir.close();
        espReleaseSD();
      }
      else
      {
        writeLog(cf_node_name+"SD Type:"+String(printer_sd_type));
        espReleaseSD();
        request->send(500, "text/plain", "NOT SD");
      }
  
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
        reset_sd_usb = 1;
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

void resetUsbHostInstance()
{
  digitalWrite(5, LOW);
  delay(50);
  digitalWrite(5, HIGH);
  delay(500);
  digitalWrite(5, LOW);
}



void resetUSBHost(AsyncWebServerRequest *request)
{
  resetUsbHostInstance();
  request->send(200, "text/plain","ok");
}

void cancleOrFinishPrint()
{
    // saveCurrentPrintStatus("NONE");
    last_power_status = 0;
    String finish_cmd = cf_node_name+":Finish";

    g_status = P_IDEL;
    recv_ok = false;
    recvl_ok = false;



  #if MB(MARLIN_VER)
    sendCmdByPackage("M524\n");
    delay(50);
    sendCmdByPackage("M118 @Stop\n");
  #elif MB(PRUSA_VER)
    sendCmdByPackage("M603\n");
    espGetSDCard();
  #endif

    // espGetSDCard();
    sendHttpMsg(finish_cmd);
    current_bed_temp = "";
    current_layers = "";
    current_temp = "";
    
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
      writeString(17,150,op);
      delay(100);
      request->send(200, "text/plain","ok");
      ESP.restart(); 
  }
}

void setInitSDBoolean(AsyncWebServerRequest *request)
{
  if (!request->hasArg("type")) 
  {
    request->send(500, "text/plain", "NO Command");
  }
  else
  {
      AsyncWebParameter* p = request->getParam(0);
      String op = p->value();

      saveCurrentPrintStatus(op);
      espReleaseSD();
      // writeString(17,150,op);
      // delay(100);
      // request->send(200, "text/plain","ok");
      // ESP.restart(); 
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
           
            #if MB(MARLIN_VER)
              sendCmdByPackage("M0\n");
            #elif MB(PRUSA_VER)
              sendCmdByPackage("M601\n");
            #endif
            g_status = PAUSE;
           
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
              
            #if MB(MARLIN_VER)
              sendCmdByPackage("M108\n");
            #elif MB(PRUSA_VER)
              sendCmdByPackage("M602\n");
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
        writeLog("cmd:"+op);
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
void cleanEEPROM(AsyncWebServerRequest * request)
{
  String tmp=" ";
  writeString(1,30,tmp); 
  delay(100);
  writeString(5,60,tmp);
  delay(100);
  writeString(9,90,tmp);
  delay(100);
  writeString(13,120,tmp);
  delay(100);
  writeString(21,180,tmp);
  request->send(200, "text/plain","ok");
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

#if MB(PRUSA_VER)
  String savePaincFileName(String filename)
  {
    int len = filename.length();
    filename.toLowerCase();
    String pure_name = filename.substring(1,len-4);
    len = pure_name.length();
    String cmd = "D3 Ax0f95 C"+String(len);
    String filename_hex=" X";
    for(int i=0; i<len; i++)
    {
      int tmp = pure_name[i];
      String num(tmp, HEX);
      
      if(i==(len-1))
      {
        filename_hex = filename_hex + num;
        if(len<8)
          filename_hex = filename_hex + "00\n";
        else
          filename_hex = filename_hex + "\n";
          
      }
      else
      {
        filename_hex = filename_hex + num + " ";   
      }
    }
    cmd = cmd + filename_hex;
    return cmd;
  }

  String dirDepth(int depth)
  {
    String cmd = "D3 Ax3930 C1 X";
    String depth_str(depth, HEX);
    cmd = cmd + depth_str + "\n";
    return cmd;
  }

  String dirsPath()
  {
    String cmd = "D3 Ax3850 C1 X2f\n";
    return cmd;
  }

#endif

void printStartInstance()
{

#if MB(MARLIN_VER)
  resetUsbHostInstance();
  delay(250);
  espReleaseSD();
  delay(200); 
  String print_cmd = "M23 "+current_file+"\n";
  sendCmdByPackage(print_cmd);
  delay(200);
  sendCmdByPackage("M24\n");
  g_status = PRINTING;
  // saveCurrentPrintStatus("PRINTING");
  last_power_status = 1;

#elif MB(PRUSA_VER)
  resetUsbHostInstance();
  delay(250);
  espReleaseSD();
  delay(200); 
  String print_cmd = "M23 "+current_file+"\n";
  sendCmdByPackage(print_cmd);
  delay(200);
  String save_panic_file = savePaincFileName(current_file);
  sendCmdByPackage(save_panic_file);
  writeLog("select file:"+current_file);
  writeLog(save_panic_file);
  delay(200);
  sendCmdByPackage(dirDepth(1));
  writeLog(dirDepth(1));
  delay(200);
  sendCmdByPackage(dirsPath());
  writeLog(dirsPath());
  delay(200);
  sendCmdByPackage("M24\n");
  g_status = PRINTING;
  // saveCurrentPrintStatus("PRINTING");
  last_power_status = 1;
#endif

}

void espRestart(AsyncWebServerRequest *request)
{
   request->send(200, "text/plain", "ok");
   ESP.restart(); 
}


//for compatible octoprint API
void octoPrinter(AsyncWebServerRequest *request)
{
  String result = "{";
  result += "\"sd\":{\"ready\": true},";
  result += "\"state\":{\"flags\":{";
    
  result +="\"operational\": false,\"sdReady\": true,\"ready\": true,";
  // result += "\"paused\":false,";
  // result += "\"printing\":false,";
  // result += "\"cancelling\":false,";
  // result += "\"pausing\":false,";
  // result += "\"error\":false,";
  // result += "\"closedOrError\":false";
  result += "}}}";

  request->send(200, "text/json", result);
}
void octoVersion(AsyncWebServerRequest *request)
{
  //  {"api":"0.1","server":"1.7.3","text":"OctoPrint 1.7.3"}
  String version_octo_api = "{\"api\":\"0.1\",\"server\":\"2.0.6\",\"text\":\"OctoPrint Node 2.0.6\"}";
  request->send(200, "text/plain", version_octo_api);
}

void octoFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *t_data, size_t len, bool final)
{
  filename = "/"+filename;
  if(g_status==PRINTING)
  {
    request->send(500, "text/plain", "PRINTER BUSY"); 
    return;
  }
  //start
  if (!index) 
  {
    espGetSDCard();
    filename = renameRandom(filename);
    if(request->hasArg("print"))
    {
      DBG_OUTPUT_PORT.println("get print value");
      AsyncWebParameter* b_print_result = request->getParam("print", true, false);
      DBG_OUTPUT_PORT.println(b_print_result->value());
      if(b_print_result->value()=="true")
      {
        b_print_after_upload = true;
        current_upload_file = filename;
      }
    }

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

    // size_t headers = request->headers();
    // DBG_OUTPUT_PORT.println(headers);

    // DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(filename);
  } 

  //write
  if (len) 
  {
    if (uploadFile) 
    {
      uploadFile.write(t_data, len);
    }

  }

  //finish
  if (final)
  {
    if (uploadFile) 
    {
      uploadFile.close();
    }
    // read_header_flag = 0;
    String logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // DBG_OUTPUT_PORT.print(logmessage); 
    if(b_print_after_upload)
    {
        String path = current_upload_file;
        path.toLowerCase();
        current_file = convertToShortName(path);
        if((g_status==P_IDEL)&&current_usb_status)
        {
           print_start_flag = 1;
        }
        b_print_after_upload = false;
    }
    espReleaseSD();
  }
}


void octohandleNotFound(AsyncWebServerRequest *request) 
{  
  request->send(200, "text/plain", "ok");
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
    server.on("/setsdinit", HTTP_GET, setInitSDBoolean);
    
    server.on("/cleaneeprom", HTTP_GET, cleanEEPROM);
    server.on("/getfmdname", HTTP_GET, getFMDBtnName);

    server.on("/user1", HTTP_GET, user1Btn);
    server.on("/user2", HTTP_GET, user2Btn);
    server.on("/user3", HTTP_GET, user3Btn);
    server.on("/user4", HTTP_GET, user4Btn);

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

      //for compatible octoprint API
    octo_server.on("/api/printer", HTTP_GET, octoPrinter);
    octo_server.on("/api/version", HTTP_GET, octoVersion); //{"api":"0.1","server":"1.7.3","text":"OctoPrint 1.7.3"}
    octo_server.on("/api/files", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200);
    }, octoFileUpload);
    octo_server.onNotFound(octohandleNotFound);
    octo_server.begin();

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
    delay(50);  
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
    delay(10);
    
    //2.send gcode to marlin, init and reload the sd card
    sendCmdByPackageNow("M21\n");
    delay(50);  
    
}

void espGetSDCard()
{
    //0. release sd from marlin
    digitalWrite(18, LOW); 
    delay(10);

    sendCmdByPackageNow("M22\n");
    delay(50);

    //1.初始化SD
    writeLog(cf_node_name+"SD Type:"+String(printer_sd_type));

    if(printer_sd_type==0)
    {   
        SPI.begin(14,2,15,13);
        int sd_get_count = 0;
        while((sd_get_count<5))
        {
          if(SD.begin(13,SPI,4000000,"/sd",5,false))
          {
            // Serial.println("SD card init successful!");
            break;
          }
          // else
          // {
          //   Serial.println("SD card init failed!");
          // }

            digitalWrite(RED_LED, LOW);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(BLUE_LED, HIGH);

            // DBG_OUTPUT_PORT.println("Not Found SD Card.");
            // messageDisplay("Not Found SD Card.");        
            delay(20);
            digitalWrite(RED_LED, HIGH);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(BLUE_LED, HIGH);  
            delay(20);
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

            // messageDisplay("Not Found SD Card.");        
            delay(20);
            digitalWrite(RED_LED, HIGH);
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(BLUE_LED, HIGH);  
            delay(20);
            sd_get_count++;
        }     
    }
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
   
}

void filamentDetect()
{
  // if((cf_filament==1)&&((!paused_for_filament)))
  // {
  //   if(digitalRead(19)==HIGH)
  //   {
  //     delay(100);
  //     if(digitalRead(19)==HIGH)  
  //     {
  //       sendCmdByPackage("M600 X0 Y0\n");
  //       String filament_cmd = cf_node_name+"-FilamentOut";
  //       sendHttpMsg(filament_cmd);
  //       paused_for_filament = true;
  //     }
  //   }  
  // }  
}


void checkLEDstatus()
{
  if(g_led_status==LED_RED)
  {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, HIGH);
  }
  else if(g_led_status==LED_GREEN)
  {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);
  }
  else if(g_led_status==LED_BLUE)
  {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(BLUE_LED, LOW);
  }
  else if(g_led_status==LED_YELLOW)
  {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
  }
  else if(g_led_status==LED_WHITE)
  {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
  }

}


void apHandleNotFound()
{
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += config_wifi_server.uri();
  message += "\nMethod: ";
  message += (config_wifi_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += config_wifi_server.args();
  message += "\n";
  for (uint8_t i = 0; i < config_wifi_server.args(); i++) {
    message += " NAME:" + config_wifi_server.argName(i) + "\n VALUE:" + config_wifi_server.arg(i) + "\n";
  }
  config_wifi_server.send(404, "text/plain", message);
}

void apHandleWiFiList() 
{
  String wifiList = "[";

  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; i++) {
    if (i > 0) {
      wifiList += ",";
    }
    wifiList += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + WiFi.RSSI(i) + "}";
    Serial.print("Wifi ssid:");
    Serial.println(WiFi.SSID(i));
  }

  wifiList += "]";
  config_wifi_server.send(200, "application/json", wifiList);
}
void apHandleRoot() 
{
  Serial.println("in the config index page");
  config_wifi_server.send(200, "text/html", wifi_config_string);
}

void apHandleConfig() {
  if (config_wifi_server.method() == HTTP_POST) {
    String device_name  = config_wifi_server.arg("devicename");
    String ssid = config_wifi_server.arg("ssid");
    String password = config_wifi_server.arg("password");
    WiFi.softAPdisconnect(true); // 断开AP模式
    WiFi.mode(WIFI_MODE_STA); // 切换到STA模式
    WiFi.begin(ssid.c_str(), password.c_str());
    int i=0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20) 
    {
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);

        delay(500);
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
        delay(500);
    }
    if (i == 21) 
    {
        Serial.print("connect failed!");
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
        delay(3000);
        g_led_status = LED_WHITE;

    }
    else{
        g_led_status = LED_GREEN;
        g_status = P_IDEL;
        //保存密码到eeprom
        writeString(1,30,ssid);
        cf_ssid = ssid;
        delay(100);
        writeString(5,60,password);
        cf_password = password;
        delay(100);
        writeString(9,90,device_name);
        cf_node_name = device_name;
        delay(100);
        Serial.print("connect successful!");

        String ip = "Beam-Holo:" + WiFi.localIP().toString();
        Serial.print(ip);
        pageDisplayIP(WiFi.localIP().toString(), "Please reboot Node Max");

        config_wifi_server.close();
        octo_server.begin();
        
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, HIGH);
        
    }
    // config_wifi_server.send(200, "text/html", "<html><body><h1>WiFi Configuration</h1><p>Connecting to WiFi network...</p></body></html>");

    
  } else {
    config_wifi_server.send(200, "text/html", "<html><body><form method=\"post\" action=\"/config\"><label for=\"ssid\">WiFi SSID:</label><input type=\"text\" id=\"ssid\" name=\"ssid\" required><label for=\"password\">WiFi Password:</label><input type=\"password\" id=\"password\" name=\"password\" required><input type=\"submit\" value=\"Connect\"></form></body></html>");
  }
}
void wifi_config_mode_init()
{
  
  WiFi.disconnect();
  delay(100);
  WiFi.setTxPower(WIFI_POWER_5dBm);
  WiFi.mode(WIFI_MODE_AP); 
  WiFi.softAP("NodeMax_ap", "fiberpunk");
  // IPAddress Ip(192, 168, 88, 88);    //setto IP Access Point same as gateway
  // IPAddress NMask(255, 255, 255, 0);
  // WiFi.softAPConfig(Ip, Ip, NMask);
  // octo_server.reset();
  octo_server.end();
  

  config_wifi_server.on("/", HTTP_GET,apHandleRoot);
  config_wifi_server.on("/wifi-list", HTTP_GET,apHandleWiFiList);
  config_wifi_server.on("/connect", apHandleConfig);
  config_wifi_server.onNotFound(apHandleNotFound);
  config_wifi_server.begin(80);
  pageDisplayIP("192.168.4.1","AP Mode Ready");

}


void apModeDetect()
{
    if(digitalRead(23)==LOW)
    {
      delay(500);
      if(digitalRead(23)==LOW)
      {
        writeLog("AP mode pressed");
        g_status=AP_MODE;
        wifi_config_mode_init();
      } 
    } 
}


void ServerProcess::serverLoop()
{
//    server.handleClient();
    if(rst_usb == true)
    {
        resetUsbHostInstance();
        rst_usb = false;
    }
    if(print_start_flag)
    {
      print_start_flag = 0;
      printStartInstance();
    }
    apModeDetect();
    checkLEDstatus();
    if (g_status==PRINTING)
    {
      g_led_status = LED_BLUE;
      filamentDetect();
    }
    else if(g_status==AP_MODE)
    {
      g_led_status = LED_WHITE;
      config_wifi_server.handleClient();
    }
    else
    {
      g_led_status = LED_GREEN;

    }
    if(paused_for_user&&(!paused_for_filament))
    {
      paused_for_user = false;
      sendCmdByPackage("M108\n");
    }
}
