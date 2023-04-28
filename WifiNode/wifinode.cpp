#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "esp_task_wdt.h"

#include "wifinode.h"
#include "nodeconfig.h"
#include "rBase64.h"

void writeString(int a,int b,String str);
String readString(int, int);
String getValue(String data, char separator, int index);
void saveCurrentPrintStatus(String status_str);
uint8_t lastPowerOffPrinting();
void pageDisplayIP(String ip,String content);


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
    String instore_sd_type = readString(EEPROM.read(21),180);
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
    writeString(21,180,status_str);
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


void writeString(int a,int b,String str)
{
    EEPROM.write(a, str.length());
    
    for (int i = 0; i < str.length(); i++)
    {
        EEPROM.write(b + i, str[i]);
    }
    EEPROM.commit();
}
String readString(int a, int b)
{ 
    String data = "";
    for (int i = 0; i < a; i++)
    {
        data += char(EEPROM.read(b + i));
    }
    return data;
}

void cameraTrigger()
{
  digitalWrite(19, HIGH);
  delay(8000);
  digitalWrite(19, LOW);
}

void messageDisplay(String content)
{
    display.clearDisplay();
    display.display();
   // text display tests
    display.setTextSize(1);
    display.setCursor(0,8);
    display.print(content);
    display.display(); // actually display all of the above  
  
}
void pageDisplayIP(String ip,String content)
{
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Name:");
    display.println(cf_node_name);
    display.print("IP:");
    display.println(ip);
    display.println(content);
    display.display(); // actually display all of the above 
    
}
void pageDisplay(String content)
{
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Name:");
    // Serial.println("cf_node_name:");
    // Serial.println(cf_node_name);
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

void WifiNode::setHeaderTitil()
{
    File file = SPIFFS.open("/index.htm", FILE_READ);
    if (!file) 
    {
        Serial.println("Failed to open file for reading");
        return;
    }
    String line;
    int lineCount = 0;
    bool titleFound = false;
    while (file.available() && lineCount < 5) 
    {
        line = file.readStringUntil('\n');
        lineCount++;
        if (line.indexOf("<title>") >= 0) 
        {
            titleFound = true;
            break;
        }
    }
    file.close();

    //if the title no exist
    if (!titleFound) 
    {
        file = SPIFFS.open("/index.htm", FILE_READ);
        if (!file) 
        {
            Serial.println("Failed to open file for reading");
            return;
        }
        String newFileContent = "<title>"+cf_node_name+"</title>\n";
        while (file.available()) 
        {
            line = file.readStringUntil('\n');
            newFileContent += line + "\n";
        }
        file.close();

        file = SPIFFS.open("/index.htm", FILE_WRITE);
        if (!file) 
        {
            Serial.println("Failed to open file for writing");
            return;
        }
        if (file.print(newFileContent)) 
        {
            Serial.println("File written");
        } 
        else 
        {
            Serial.println("Write failed");
        }
        file.close();
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
    String instore_sd_type = readString(EEPROM.read(17),150);
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
            #if MB(MARLIN_VER)
                printer_sd_type = 1;
            #elif MB(PRUSA_VER)
                printer_sd_type = 0;
            #endif
        }
        
    }
    else
    {
        #if MB(MARLIN_VER)
            printer_sd_type = 1;
        #elif MB(PRUSA_VER)
            printer_sd_type = 0;
        #endif
    }
    
    //usb host reset
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW);

    //faliment detector
    pinMode(19, INPUT_PULLUP);
    pinMode(18, OUTPUT);

    //ap mode pin
    pinMode(23, INPUT_PULLUP);
    

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
    messageDisplay(version); 
    delay(2000); 

    resetUsbHostInstance();
    delay(1000);
    
    if(!last_power_status)
    {
        messageDisplay("Checking SD Card ...");
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
                messageDisplay("Not Found SPI SD Card.");        
                delay(50);
                digitalWrite(RED_LED, HIGH);
                digitalWrite(GREEN_LED, HIGH);
                digitalWrite(BLUE_LED, HIGH);  
                delay(50);
                sd_get_count++;
                
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

                messageDisplay("Not Found SD Card.");        
                delay(50);
                digitalWrite(RED_LED, HIGH);
                digitalWrite(GREEN_LED, HIGH);
                digitalWrite(BLUE_LED, HIGH);  
                delay(50);
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
    messageDisplay("Wait Connect WiFi."); 
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
        writeString(1,30,cf_ssid);
        cf_ssid = readString(EEPROM.read(1),30);
        delay(100);
        writeString(5,60,cf_password);
        cf_password = readString(EEPROM.read(5),60);
        delay(100);
        writeString(9,90,cf_node_name);
        cf_node_name = readString(EEPROM.read(9),90);
        delay(100);

        writeString(13,120,b_filament);
        b_filament = readString(EEPROM.read(13),120);
        delay(50);
        cf_filament = b_filament.toInt();
        // if(printer_sd_type==0)
        //     SD.remove("/config.txt");
        // else if(printer_sd_type==1)
        //     SD_MMC.remove("/config.txt");

        if(cf_node_name.length()>32)
        {
            cf_node_name = "Node Max";
        }
    }
    else  //read ssid info frome eeprom directly
    {
        cf_ssid = readString(EEPROM.read(1),30);
        delay(100);
        cf_password = readString(EEPROM.read(5),60);
        delay(100);
        cf_node_name = readString(EEPROM.read(9),90);
        delay(100);
        b_filament = readString(EEPROM.read(13),120);
        delay(100);
        cf_filament = b_filament.toInt();

        if(cf_node_name.length()>32)
        {
            cf_node_name = "Node Max";
        }
    }

    setHeaderTitil();
    
    WiFi.mode(WIFI_STA);

    WiFi.persistent(false); 
    WiFi.setAutoConnect(false);
    // WiFi.setHostname(cf_node_name.c_str());

    WiFi.begin((const char*)cf_ssid.c_str(), (const char*)cf_password.c_str());
//    WiFi.begin((const char*)cf_ssid.c_str(), (const char*)cf_password.c_str(),0,0,true);


    uint8_t i = 0;
    
    while (WiFi.status() != WL_CONNECTED && i++ < 20) 
    {
        String point = "Wait Connect WiFi.";
        //totaly wait 10 seconds
        delay(500);
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
        for(int nm=0; nm<i/4; nm++)
        {
            point += ".";
        }
        if(i%4==0)
            messageDisplay(point);
    }
    if (i == 21) 
    {
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
        // DBG_OUTPUT_PORT.print("Could not connect to:");
        // DBG_OUTPUT_PORT.println(cf_ssid.c_str());
        // DBG_OUTPUT_PORT.println(cf_password.c_str());
         messageDisplay("Connect WiFi Wrong.");
        delay(500);
        if(wifi_count<3)
        {
            wifi_count++;
            goto initwifi;  
        }
        // goto initwifi;
    }
    // DBG_OUTPUT_PORT.print("Connected! IP address: ");
    // DBG_OUTPUT_PORT.println(WiFi.localIP());

    if(wifi_count<3)
    {
        pageDisplay("Wifi Ready!");  
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
    }
    else
    {
        // Serial.println("wifi connect failed!!");
        messageDisplay("Wifi Error,check SD card or password!");
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
    }
    //3. server init
    serverprocesser.serverInit();

    //4. crc init
    gcrc.begin();
    // getFMDfile();
    delay(500);
    resetUsbHostInstance();
    delay(100);
    espReleaseSD();
    PRINTER_PORT.flush();

}

void WifiNode::process()
{
    serverprocesser.serverLoop();
    checkwifi();
    if(reset_sd_usb)
    {
        resetUsbHostInstance();
        // espReleaseSD();
        // delay(50);
        // espGetSDCard();
        reset_sd_usb = 0;
    }
    if(pre_usb_status!=current_usb_status)
    {
        if(current_usb_status)//connected
        {
            pageDisplay("Printer Connected!");
        }
        else
        {
            String pub_msg = "offline";
            // espReleaseSD();
            pageDisplay("Printer Not Connect!");
            writeLog(cf_node_name+":offline");
            events.send(pub_msg.c_str(), "gcode_cli");
        }
        
        pre_usb_status = current_usb_status;
    }
}
