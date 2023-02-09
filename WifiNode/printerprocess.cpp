#include "printerprocess.h"
#include "nodeconfig.h"
#include "soc/rtc_wdt.h"

extern void cancleOrFinishPrint();
extern void espGetSDCard();
void printLoop(void * parameter);


void setWifiConfigByPort(String config_str)
{
  String cmd_str = config_str.substring(2);
  String cmd_name = getValue(cmd_str, ':', 0);
  String cmd_value = getValue(cmd_str, ':', 1);
  if(cmd_name=="SSID")
  {
    cf_ssid = cmd_value.substring(0,cmd_value.length()-1);
    Serial.print("SSID");
  }
  else if(cmd_name=="PASS")
  {
    cf_password = cmd_value.substring(0,cmd_value.length()-1);
    Serial.print("PASS");
  }
  else if(cmd_name=="NAME")
  {
    cf_node_name = cmd_value.substring(0,cmd_value.length()-1);
    Serial.print("NAME");
  }
  else if(cmd_name=="SAVE")
  {
    writeString(1,30,cf_ssid);
    delay(100);
    writeString(5,60,cf_password);
    delay(100);
    writeString(9,90,cf_node_name);
    delay(100);
    Serial.print("SAVE");
  }
  else if(cmd_name=="IPADDRESS")
  {
    if ((WiFi.status() == WL_CONNECTED)) 
    {
      Serial.print("&&"); 
      Serial.print(WiFi.localIP()); 
      Serial.println("##"); 
    }
  }
  else
  {
    Serial.print("ERROR");
  }
  

}

String inData="";//Gcode Command return value
void readPrinterBack()
{
  //读取所有的数据
  while (PRINTER_PORT.available() > 0 && recvl_ok == false)
  {
    char recieved = PRINTER_PORT.read();
    if (recieved == '\n')
    {
      recvl_ok = true;
    }
    inData += recieved; 
  }

  //根据所含的inData做出具体的反应
  if (recvl_ok == true)
  {
    if(inData.length()>=2)
    {
      writeLog(cf_node_name+":"+inData);
      // writeLog(inData); 
      if(inData.startsWith("&&"))
      {
        setWifiConfigByPort(inData);
      }
      if(inData.indexOf("setusb")!=-1)
      {
//        if (g_status!=PRINTING)
            rst_usb = true;
      } 
      //check temp
      if (inData.indexOf("T:")!=-1)
      {
        current_temp = inData;
      }
      else if(inData.indexOf("B:")!=-1)
      {
        current_bed_temp = inData;
      }

      //在整行中检测收到ok的情况
      if(inData.indexOf("ok")!=-1)
      {
        recv_ok = true;
      }
      else
      {
        recv_ok = false;
        recvl_ok = false;
      }

      if(inData.indexOf("#s")!=-1)
      {
        current_usb_status = 1;
      }
      else if(inData.indexOf("$f")!=-1)
      {
        if(g_status!=PRINTING)
        {
          current_usb_status = 0;
        }
      }

      if(inData.indexOf("@Stop")!=-1)
      {
          espGetSDCard();
          reset_sd_usb = 1;
      }

      if(g_status==PRINTING)
      {
        if(inData.indexOf("paused for")!=-1)
        {
//          paused_for_user = true;
        }

        if(inData.indexOf("SD printing")!=-1)
        {
          current_layers = inData;
        } 
        if(inData.indexOf("@Capture")!=-1)
        {
          Serial2.print("#%");
          String capture_cmd = cf_node_name+"@TakeImg";
          sendCaptureImage(capture_cmd);
        }
        
        if(inData.indexOf("Finish")!=-1)
        {
          cancleOrFinishPrint();
          
        }
        else if(inData.indexOf("Done printing")!=-1)
        {
          cancleOrFinishPrint();
          
        }
        else if(inData.indexOf("resumed")!=-1)
        {
          paused_for_filament = false;  
        }
        
        if(inData.indexOf("No media")!=-1)
        {
          cancleOrFinishPrint();
         
        }
        
      }
      inData="";  
    }
    else
    {
      recvl_ok = false;  
    }    /* code */
    inData="";
  }

}

//core0执行的任务，这个核专门接收串口返回的数据，然后发送出去
void printLoop(void * parameter)
{
  // const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
  for(;;)
  {
    readPrinterBack();
    rtc_wdt_feed();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

PrinterProcess::PrinterProcess()
{

}
