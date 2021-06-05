#include "serverprocess.h"
#include "printerprocess.h"
#include "nodeconfig.h"

extern void printFile(String);
extern  String  readLine(File&);

const char* host = "EdgeeWifi";


void returnOK() 
{
  server.send(200, "text/plain", "");
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
    if (SD.exists((char *)upload.filename.c_str())) 
    {
      SD.remove((char *)upload.filename.c_str());
    }
    uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
    DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(upload.filename);
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) 
  {
    if (uploadFile) 
    {
      uploadFile.write(upload.buf, upload.currentSize);
    }
    DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: "); DBG_OUTPUT_PORT.println(upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) 
  {
    if (uploadFile) 
    {
      uploadFile.close();
    }
    DBG_OUTPUT_PORT.print("Upload: END, Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void deleteRecursive(String path) 
{
  File file = SD.open((char *)path.c_str());
  if (!file.isDirectory()) 
  {
    file.close();
    SD.remove((char *)path.c_str());
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
      SD.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SD.rmdir((char *)path.c_str());
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
  DBG_OUTPUT_PORT.print(path);
  if (path == "/" || !SD.exists((char *)path.c_str())) 
  {
    returnFail("BAD PATH");
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
  if (path == "/" || SD.exists((char *)path.c_str())) 
  {
    returnFail("BAD PATH");
    return;
  }

  if (path.indexOf('.') > 0) {
    File file = SD.open((char *)path.c_str(), FILE_WRITE);
    if (file) 
    {
      file.write(0);
      file.close();
    }
  } 
  else 
  {
    SD.mkdir((char *)path.c_str());
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
  if (path != "/" && !SD.exists((char *)path.c_str())) 
  {
    return returnFail("BAD PATH");
  }
  File dir = SD.open((char *)path.c_str());
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
  DBG_OUTPUT_PORT.print(message);
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
void printerStatus()
{
  String result = total_layers+","+current_layers+","+current_temp; 
 
  server.send(200, "text/plain",result);
}

void printerControl()
{
  if (!server.hasArg("op")) 
  {
    return returnFail("BAD ARGS");
  }
  String op = server.arg("op");

  if(op=="PAUSE")
  {
    g_status = PAUSE;
    //just for test
    client.print("TakeImg");
  }
  else if(op=="CANCLE")
  {
    g_status = CANCLE;
    cmd_fifo.clear();
    g_status = P_IDEL;
    g_printfile.close();
    client.stop();
  }
  else if(op=="RECOVER")
  {
    if(g_status == PAUSE)
    {
      g_status = PRINTING;
      PRINTER_PORT.print("G4 S1\n");
    }

  }
  returnOK();
}

void getPCAddress()
{
    if (!server.hasArg("ip")) 
    {
        return returnFail("BAD ARGS");
    }
    pc_ipaddress = server.arg("ip");
    DBG_OUTPUT_PORT.print(pc_ipaddress);

    if (!client.connect(pc_ipaddress.c_str(), 1688)) 
    {
        Serial.println("connection pc socket failed");
        return returnFail("Connect PC failed!");
    }

    returnOK();
}

void sendGcode()
{
    if (!server.hasArg("gc")) 
    {
        return returnFail("BAD ARGS");
    }
    String op = server.arg("gc")+"\n";
    DBG_OUTPUT_PORT.print(op);
    returnOK();
}

void printStart()
{
  if (!server.hasArg("filename")) 
  {
    return returnFail("BAD ARGS");
  }
  String path = server.arg("filename");
  if (path != "/" && !SD.exists((char *)path.c_str())) 
  {
    return returnFail("BAD PATH");
  }
  PRINTER_PORT.print("G4 S1\n");
  delay(1000);
  printFile(path);
  returnOK();
}

ServerProcess::ServerProcess()
{

}

void ServerProcess::serverInit()
{
    File uploadFile;
    String pre_line="";

    File g_printfile;
    OP_STATUS g_status=P_IDEL;
    ERROR_CODE g_error=NORMAL;

    if (MDNS.begin(host)) 
    {
        MDNS.addService("http", "tcp", 80);
        DBG_OUTPUT_PORT.println("MDNS responder started");
        DBG_OUTPUT_PORT.print("You can now connect to http://");
        DBG_OUTPUT_PORT.print(host);
        DBG_OUTPUT_PORT.println(".local");
    }

    server.on("/find", HTTP_GET, reportDevice);
    server.on("/gcode", HTTP_GET, sendGcode);
    server.on("/pcsocket", HTTP_GET, getPCAddress);

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

  DBG_OUTPUT_PORT.println("HTTP server started");

}

void sendPrintCmd()
{
    if(g_status==PRINTING)
    {
      if(recv_ok)
      {
        if(cmd_fifo.size()>0)
        {
            String send_line = cmd_fifo.pop();
            pre_line = send_line;
            if(send_line=="end")
            {
              g_status=P_IDEL;
              if(g_printfile)
                g_printfile.close();
              DBG_OUTPUT_PORT.print("Print finish!!!");
              client.stop();
              sendGcode_cnt=0;
              recGok_cnt = 0;
            }
            else
            {
              PRINTER_PORT.print(send_line);
            }
            recv_ok = false;
            recvl_ok = false;
        }
      }
       
    }
    else if(g_status==PAUSE)
    {
      recv_ok = false;
      recvl_ok = false;
    }
    else if(g_status==CANCLE)
    {
      if(g_printfile)
        g_printfile.close();
      g_status = P_IDEL;
      recv_ok = false;
      recvl_ok = false;
    }
}


void readLineFromFile()
{
  if(g_printfile && (g_status==PRINTING) && cmd_fifo.size()<FIFO_SIZE)
  {
    if(b_time_laspe && setting_fifo.size()>0)
    {
      String setting_cmd = setting_fifo.pop();
      cmd_fifo.push(setting_cmd);
      return;
    }
    //1. 读取一行Gcode, 如果是注释行则跳过
    String line = readLine(g_printfile);

    while(line.startsWith(";")||(line.length()<=1))
    {
      if(line.indexOf("_COUNT:")!=-1)
      {
        total_layers = line;

      }
      if(line.indexOf("LAYER:")!=-1)
      {
        current_layers = line;
        if(b_time_laspe)
        {
          //move to the side, and delay 2 seconds, to make time 
          String relative_mode = "G91 \n";
          String absolute_mode = "G90 \n";
          String e_react = "G1 E-"+String(react_length/2)+"\n";
          String e_exrude = "G1 E"+String(react_length)+"\n";
          String stop_up = "G0 F4500 X"+ String(stop_x)+" Y"+String(stop_y)+"\n";
          String capture_time = "G4 S1\n";
          String capture_flag = "M114\n";
          String z_up = "G0 Z2 \n";
          String z_down = "G0 Z-2 \n";

          setting_fifo.push(relative_mode);
          setting_fifo.push(e_react);
          setting_fifo.push(z_up);
          setting_fifo.push(absolute_mode);

          setting_fifo.push(stop_up);
          setting_fifo.push(capture_flag);
          setting_fifo.push(capture_time);

          setting_fifo.push(relative_mode);
          setting_fifo.push(z_down);
          setting_fifo.push(e_exrude);
          setting_fifo.push(absolute_mode);
        }

      }
      line = readLine(g_printfile);
    }  
    cmd_fifo.push(line);

    sendGcode_cnt++;

    if (sendGcode_cnt == 20)
    {
      String CMD_M105 = "M105\n";
      setting_fifo.push(CMD_M105);
    }
    if ((sendGcode_cnt >= 200) && (sendGcode_cnt % 200 == 0))
    {
      String CMD_M105 = "M105\n";
      setting_fifo.push(CMD_M105);
    }   
  }
}


void ServerProcess::serverLoop()
{
    server.handleClient();
    readLineFromFile();
    sendPrintCmd();
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
}
