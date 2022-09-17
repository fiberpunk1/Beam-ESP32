#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "esp_task_wdt.h"

#include "wifinode.h"
#include "nodeconfig.h"
#include "rBase64.h"

void Write_String(int a,int b,String str);
String Read_String(int, int);
String getValue(String data, char separator, int index);
void saveCurrentPrintStatus(String status_str);
uint8_t lastPowerOffPrinting();

extern void hardwareReleaseSD();
extern void getFMDfile();
extern void espGetSDCard();
extern void espReleaseSD();
extern void resetUsbHostInstance();
extern void sendCmdByPackage(String cmd);
extern void sendCmdByPackageNow(String cmd);

FiberPunk_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//wifi重连
unsigned long previousMillis = 0;
unsigned long interval = 30000;


uint8_t lastPowerOffPrinting()
{
    String instore_sd_type = Read_String(EEPROM.read(21),180);
    if(instore_sd_type.length()>0)
    {
      
        if(instore_sd_type.indexOf("PRINTING")!=-1)
        {
            last_power_status = 1;
            return 1;
        }
        else if(instore_sd_type.indexOf("NONE")!=-1)
        {
            last_power_status = 0;
            return 0;
        }
        else
        {
            last_power_status = 0;
            return 0;
        }
        
    }
    else
    {
        last_power_status = 0;
        return 0;
    }
}

void saveCurrentPrintStatus(String status_str)
{
    Write_String(21,180,status_str);
    delay(100);
}

String readConfig(File& file)
{
  String ret="";
  if(file.available())
  {
    ret = file.readStringUntil('\n');
    ret.replace("\r", "");
  }
  
  return ret;
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;


    for (int i = 0; i <= maxIndex && found <= index; i++) 
    {
        if (data.charAt(i) == separator || i == maxIndex) 
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//a写入字符串长度，b是起始位，str为要保存的字符串
void Write_String(int a,int b,String str){
    EEPROM.write(a, str.length());//EEPROM第a位，写入str字符串的长度
    //把str所有数据逐个保存在EEPROM
    for (int i = 0; i < str.length(); i++){
        EEPROM.write(b + i, str[i]);
    }
    EEPROM.commit();
}
//a位是字符串长度，b是起始位
String Read_String(int a, int b){ 
    String data = "";
    //从EEPROM中逐个取出每一位的值，并链接
    for (int i = 0; i < a; i++)
    {
        data += char(EEPROM.read(b + i));
    }
    return data;
}

void camera_trigger()
{
  digitalWrite(19, HIGH);
  delay(8000);
  digitalWrite(19, LOW);
}

void message_display(String content)
{
    display.clearDisplay();
    display.display();
   // text display tests
    display.setTextSize(1);
    display.setCursor(0,8);
    display.print(content);
    display.display(); // actually display all of the above  
  
}

void page_display(String content)
{
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Name:");
    display.println(cf_node_name);
    display.print("IP:");
    display.println(WiFi.localIP());
    display.println(content);
    display.display(); // actually display all of the above 
    
}

WifiNode::WifiNode()
{

}

//定时检测
void WifiNode::checkwifi()
{
    unsigned long currentMillis = millis();
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) 
    {
        WiFi.disconnect();
        WiFi.reconnect();
        previousMillis = currentMillis;
    }
}

void WifiNode::init()
{
	uint8_t pw_exist = 0;
    
    String b_filament = "";
	EEPROM.begin(256);
    //0.初始化串口和OLED屏
    PRINTER_PORT.begin(115200);
    Serial2.begin(115200);
    PRINTER_PORT.setTimeout(120);
    PRINTER_PORT.setRxBufferSize(512);
    PRINTER_PORT.setDebugOutput(true);

    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    last_power_status = lastPowerOffPrinting();

    //default to SDIO method
    String instore_sd_type = Read_String(EEPROM.read(17),150);
    if(instore_sd_type.length()>0)
    {
      
        if(instore_sd_type.indexOf("SPI")!=-1)
        {
            printer_sd_type = 0;
        }
        else if(instore_sd_type.indexOf("SDIO")!=-1)
        {
            printer_sd_type = 1;
        }
        else
        {
            printer_sd_type = 1;
        }
        
    }
    else
    {
        printer_sd_type = 1;
    }
    


    pinMode(5, OUTPUT);
    digitalWrite(5, LOW);

    //faliment detector
    pinMode(19, INPUT);
    pinMode(18, OUTPUT);

    //SD switcher
    if(last_power_status)
    {
        digitalWrite(18, HIGH);//default to 3D printer
    }
    else
    {
        digitalWrite(18, LOW);
    }
    
    
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    display.setRotation(2);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    {
        // DBG_OUTPUT_PORT.println(F("OLED SSD1306 allocation failed"));
        //for(;;); // Don't proceed, loop forever
    }
    
    display.setTextColor(SSD1306_WHITE);
    display.display();
    delay(2000); // Pause for 2 seconds  
    String version = String(VERSION);
    message_display(version); 
    delay(2000); 
    message_display("Checking SD Card ...");
    delay(2000);
    //让打印机释放SD卡
    resetUsbHostInstance();
    delay(1000);
//    sendCmdByPackageNow("G28\n");
//    delay(1000);
    sendCmdByPackageNow("M22\n");
    delay(500);
    //1.初始化SD
    if(!last_power_status)
    {
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
                    message_display("Not Found SD Card.");        
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

                    message_display("Not Found SD Card.");        
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
        
            // DBG_OUTPUT_PORT.println("SD Card initialized.");
            hasSD = true;
    }

    
   
    //2.初始化wifi
    uint8_t wifi_count = 0;
    initwifi:
    message_display("Wait Connect WiFi..."); 
    delay(500);
    //读取wifi账号密码，还有打印机名字
    
    File config_file;
    //如果上一次断电，不是打印状态
    if(!last_power_status)
    {
        if(printer_sd_type==0)
            config_file = SD.open("/config.txt",FILE_READ);
        else if(printer_sd_type==1)
            config_file = SD_MMC.open("/config.txt",FILE_READ);

        pw_exist = 0;
    }

    

    if(config_file)
    {
        String tmp_str = "";
        //ssid
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("ssid")!=-1)
        {
            cf_ssid = getValue(tmp_str, ':', 1);
        }
        //password
        tmp_str = readConfig(config_file);   
        if(tmp_str.indexOf("pass_word")!=-1)
        {
            pw_exist = 1; 
            cf_password = getValue(tmp_str, ':', 1);

            // if(rbase64.decode(tmp_str)==RBASE64_STATUS_OK)
            // {
            //     cf_password = rbase64.result();
            // }    
        }
        //device name
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("device_name")!=-1)
        {
            cf_node_name = getValue(tmp_str, ':', 1);
        }

        //filament
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("filament_detect")!=-1)
        {
            b_filament = getValue(tmp_str, ':', 1);
        }
        
               
    }
    config_file.close();
	
    if (pw_exist == 1)
    {   
        Write_String(1,30,cf_ssid);
        cf_ssid = Read_String(EEPROM.read(1),30);
        delay(100);
        Write_String(5,60,cf_password);
        cf_password = Read_String(EEPROM.read(5),60);
        delay(100);
        Write_String(9,90,cf_node_name);
        cf_node_name = Read_String(EEPROM.read(9),90);
        delay(100);

        Write_String(13,120,b_filament);
        b_filament = Read_String(EEPROM.read(13),120);
        delay(50);
        cf_filament = b_filament.toInt();
        if(printer_sd_type==0)
            SD.remove("/config.txt");
        else if(printer_sd_type==1)
            SD_MMC.remove("/config.txt");
    }
    else
    {
        cf_ssid = Read_String(EEPROM.read(1),30);
        delay(100);
        cf_password = Read_String(EEPROM.read(5),60);
        delay(100);
        cf_node_name = Read_String(EEPROM.read(9),90);
        delay(100);
        b_filament = Read_String(EEPROM.read(13),120);
        delay(100);
        cf_filament = b_filament.toInt();
    }
    
    WiFi.mode(WIFI_STA);

    WiFi.persistent(false); 
    WiFi.setAutoConnect(false);
    WiFi.setHostname(cf_node_name.c_str());

    WiFi.begin((const char*)cf_ssid.c_str(), (const char*)cf_password.c_str());

    
    // if(MDNS.begin(cf_node_name.c_str()))
    // {
    //     MDNS.addService("http", "tcp", 88);
    // }

    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20) 
    {
        //totaly wait 10 seconds
        delay(500);
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
    }
    if (i == 21) 
    {
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
        // DBG_OUTPUT_PORT.print("Could not connect to:");
        // DBG_OUTPUT_PORT.println(cf_ssid.c_str());
        // DBG_OUTPUT_PORT.println(cf_password.c_str());
         message_display("Connect WiFi Wrong.");
        delay(500);
        if(wifi_count<4)
        {
            wifi_count++;
            goto initwifi;  
        }
        // goto initwifi;
    }
    // DBG_OUTPUT_PORT.print("Connected! IP address: ");
    // DBG_OUTPUT_PORT.println(WiFi.localIP());
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
    if(wifi_count<=3)
    {
        page_display("Wifi Ready!");  
    }
    else
    {
        page_display("Wifi Error!");
    }
    //3. server init
    serverprocesser.serverInit();

    //4. crc init
    gcrc.begin();
    getFMDfile();
    delay(500);
    resetUsbHostInstance();
    PRINTER_PORT.flush();
    // camera_trigger();
    
   // hardwareReleaseSD();
   // delay(2000);
   // espGetSDCard();
}

void WifiNode::process()
{
    serverprocesser.serverLoop();
    checkwifi();
    if(reset_sd_usb)
    {
        resetUsbHostInstance();
        espReleaseSD();
        delay(50);
        espGetSDCard();
        reset_sd_usb = 0;
    }
    if(pre_usb_status!=current_usb_status)
    {
        if(current_usb_status)//connected
        {
            page_display("Printer Connected!");
        }
        else
        {
            String pub_msg = "offline";
            espReleaseSD();
            page_display("Printer Not Connect!");
            writeLog(cf_node_name+":offline");
            events.send(pub_msg.c_str(), "gcode_cli");
        }
        
        pre_usb_status = current_usb_status;
    }
}
