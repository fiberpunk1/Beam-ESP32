#include "printerprocess.h"
#include "nodeconfig.h"
#include "soc/rtc_wdt.h"

extern void cancleOrFinishPrint();
void printLoop(void * parameter);


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
      
      if(inData.indexOf("setusb")!=-1)
      {
        if(g_status!=PRINTING)
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
        current_usb_status = 0;
      }

      if(g_status==PRINTING)
      {
        if(inData.indexOf("paused for")!=-1)
        {
          paused_for_user = true;
        }

        if(inData.indexOf("SD printing")!=-1)
        {
          current_layers = inData;
        } 
        if(inData.indexOf("X:")!=-1)
        {
          String capture_cmd = "Camera-"+cf_node_name+"-TakeImg";
          sendHttpMsg(capture_cmd);
        }
        
        if(inData.indexOf("Finish")!=-1)
        {
          cancleOrFinishPrint();
          
        }
        else if(inData.indexOf("Done")!=-1)
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
