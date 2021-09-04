#include "printerprocess.h"
#include "nodeconfig.h"

void printFile(String file);
void printLoop(void * parameter);
String readLine(File& file);


bool is_heating = false;


String readLine(File& file)
{
  String ret="";
  if(file.available())
  {
    ret = file.readStringUntil('\n');
    int tmp = ret.indexOf(";");
    if(tmp!=-1)
    {
        if(tmp<=2)
        {
          return ret;  
        }
        else if(tmp>2)
        {
          if(tmp>64)
            tmp = 64; //Prevent the string being too long
          ret = ret.substring(0, tmp);
          return ret;
        }
        
    }

    if (ret.length() < 2)
    {
      ret = "";
    }
    else
    {
      ret = ret + "\n"; 
    }
  }
  else
  {
    ret="&&&";
  }
  
  return ret;
}

void printFile(String filename)
{
    if(g_status==P_IDEL)
    {
      //只有IDLE状态下才可以进行打印
      g_printfile = SD_MMC.open(filename,FILE_READ);
      if(g_printfile)
      {
        g_status = PRINTING;
        String line = readLine(g_printfile);
        while(line.startsWith(";")||(line.length()<=1))
        {
          line = readLine(g_printfile);
        }
        // PRINTER_PORT.print(line);
        cmd_fifo.push(line);
      }
      else
      {
        //打开文件失败，返回信息给客户端
        g_error = OPEN_FAILED;

      }
    }
    else
    {
      
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
      //在整行中检测收到ok的情况
      if((inData.indexOf("resend")!=-1)||(inData.indexOf("Unknown")!=-1))
      {
        resend = true;
        inData="";
        timecnt = 0;
        return;
      }  
      if(inData.indexOf("setusb")!=-1)
      {
        rst_usb = true;
        inData="";
        return;
      }    
      //check temp
      if (inData.indexOf("T:")!=-1)
      {
        current_temp = inData;
        timecnt = 0;
      }
      else if(inData.indexOf("B:")!=-1)
      {
        current_bed_temp = inData;
        timecnt = 0;
      }

      //在整行中检测收到ok的情况
      if(inData.indexOf("ok")!=-1)
      {
        recv_ok = true;
        timecnt = 0;
        recGok_cnt++;
      }
      else
      {
        recv_ok = false;
        recvl_ok = false;
        timecnt = 0;
      }

      if(inData.indexOf("#s")!=-1)
      {
        current_usb_status = 1;
      }
      else if(inData.indexOf("$f")!=-1)
      {
        current_usb_status = 0;
      }

      if(inData.indexOf("X:")!=-1)
      {
        String capture_cmd = "Camera-"+cf_node_name+"-TakeImg";
        // client.print(capture_cmd);
        sendHttpMsg(capture_cmd);
        timecnt = 0;
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
  //Clear count
  sendGcode_cnt = 0;
  recGok_cnt = 0;
  for(;;)
  {
    readPrinterBack();
    vTaskDelay(10);
    timecnt++;  
  }
}

PrinterProcess::PrinterProcess()
{

}
