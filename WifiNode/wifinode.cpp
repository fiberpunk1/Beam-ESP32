#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "esp_task_wdt.h"

#include "wifinode.h"
#include "nodeconfig.h"
#include "rBase64.h"

FiberPunk_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
    for (int i = 0; i < a; i++){
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
    display.setCursor(0,15);
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



void WifiNode::init()
{
    //开启双核设置
    enableCore0WDT(); enableCore1WDT();
    esp_task_wdt_init(3, false);
	uint8_t pw_exist = 0;
    String stop_x_str = "";
    String stop_y_str = "";
    String react_l_str = "";
    String b_time_l_str = "";
	EEPROM.begin(256);
    //0.初始化串口和OLED屏
    PRINTER_PORT.begin(100000);
    PRINTER_PORT.setTimeout(120);
    PRINTER_PORT.setRxBufferSize(512);
    PRINTER_PORT.setDebugOutput(true);
    // DBG_OUTPUT_PORT.print("\n");
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    pinMode(19, OUTPUT);
    digitalWrite(19, LOW);

    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);

    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    {
        // DBG_OUTPUT_PORT.println(F("OLED SSD1306 allocation failed"));
        //for(;;); // Don't proceed, loop forever
    }
    display.setTextColor(SSD1306_WHITE);
    display.display();
    delay(2000); // Pause for 2 seconds   
    message_display("Checking SD Card ...");
    delay(2000);
    //1.初始化SD
    while(!SD_MMC.begin())
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
        //break;      
    } 
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE){
        //Serial.println("No SD_MMC card attached");
        //return;
    }
    // DBG_OUTPUT_PORT.println("SD Card initialized.");
    hasSD = true;
   
    //2.初始化wifi
    initwifi:
    message_display("Wait Connect WiFi..."); 
    delay(500);
    //读取wifi账号密码，还有打印机名字
    
    File config_file = SD_MMC.open("/config.txt",FILE_READ);

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
            tmp_str = getValue(tmp_str, ':', 1);
            if(rbase64.decode(tmp_str)==RBASE64_STATUS_OK)
            {
                cf_password = rbase64.result();
            }    
        }
        //device name
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("device_name")!=-1)
        {
            cf_node_name = getValue(tmp_str, ':', 1);
        }
        
        //stop_x
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("stop_x")!=-1)
        {
            stop_x_str = getValue(tmp_str, ':', 1);
            //stop_x = tmp_str.toInt();
        }
        
        //stop_y
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("stop_y")!=-1)
        {
            stop_y_str = getValue(tmp_str, ':', 1);
            //stop_y = tmp_str.toInt();
        }

        //react e       
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("react_length")!=-1)
        {
            react_l_str = getValue(tmp_str, ':', 1);
            //react_length = tmp_str.toFloat();
        }

        //b_time_laspe
        tmp_str = readConfig(config_file);
        if (tmp_str.indexOf("b_time_laspe")!=-1)
        {
            b_time_l_str = getValue(tmp_str, ':', 1);
            //b_time_laspe = (unsigned char)tmp_str.toInt();
        }        
    }
    config_file.close();
	
    if (pw_exist == 1)
    {   
        Write_String(1,30,cf_ssid);
        cf_ssid = Read_String(EEPROM.read(1),30);
        DBG_OUTPUT_PORT.print(cf_ssid);
		
        Write_String(5,60,cf_password);
        cf_password = Read_String(EEPROM.read(5),60);
        DBG_OUTPUT_PORT.print(cf_password);

        Write_String(9,90,cf_node_name);
        cf_node_name = Read_String(EEPROM.read(9),90);
        DBG_OUTPUT_PORT.print(cf_node_name);

        Write_String(13,120,cf_node_name);
        cf_node_name = Read_String(EEPROM.read(13),120);
        DBG_OUTPUT_PORT.print(cf_node_name);

        Write_String(17,150,stop_x_str);
        stop_x_str = Read_String(EEPROM.read(17),150);
        DBG_OUTPUT_PORT.print(stop_x_str);
        stop_x = stop_x_str.toInt();

        Write_String(21,180,stop_y_str);
        stop_y_str = Read_String(EEPROM.read(21),180);
        DBG_OUTPUT_PORT.print(stop_y_str);
        stop_y = stop_y_str.toInt();

        Write_String(25,210,react_l_str);
        react_l_str = Read_String(EEPROM.read(25),210);
        DBG_OUTPUT_PORT.print(react_l_str);
        react_length = react_l_str.toFloat();

        Write_String(29,250,b_time_l_str);
        b_time_l_str = Read_String(EEPROM.read(29),250);
        DBG_OUTPUT_PORT.print(b_time_l_str);
        b_time_laspe = (unsigned char)b_time_l_str.toInt();

        SD_MMC.remove("/config.txt");
    }
    else
    {
		
        cf_ssid = Read_String(EEPROM.read(1),30);
        
        cf_password = Read_String(EEPROM.read(5),60);

        cf_node_name = Read_String(EEPROM.read(9),90);

        cf_node_name = Read_String(EEPROM.read(13),120);

        stop_x_str = Read_String(EEPROM.read(17),150);
        stop_x = stop_x_str.toInt();

        stop_y_str = Read_String(EEPROM.read(21),180);
        stop_y = stop_y_str.toInt();

        react_l_str = Read_String(EEPROM.read(25),210);
        react_length = react_l_str.toFloat();

        b_time_l_str = Read_String(EEPROM.read(29),250);
        b_time_laspe = (unsigned char)b_time_l_str.toInt();
    }
    
    WiFi.mode(WIFI_STA);

    WiFi.persistent(false); 
    WiFi.setAutoConnect(false);

    WiFi.begin((const char*)cf_ssid.c_str(), (const char*)cf_password.c_str());

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
        delay(2000);
        goto initwifi;
    }
    // DBG_OUTPUT_PORT.print("Connected! IP address: ");
    // DBG_OUTPUT_PORT.println(WiFi.localIP());
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
    page_display("Wifi Ready!");
    //3. server init
    serverprocesser.serverInit();

    //4. crc init
    gcrc.begin();
    
    digitalWrite(5, HIGH);
    delay(50);
    digitalWrite(5, LOW);
    delay(500);
    digitalWrite(5, HIGH);
    camera_trigger();
    delay(500);
}

void WifiNode::process()
{
    serverprocesser.serverLoop();
    if(pre_usb_status!=current_usb_status)
    {
        if(current_usb_status)//connected
        {
            page_display("Printer Connected!");
        }
        else
        {
            page_display("Printer Not Connect!");
        }
        
        pre_usb_status = current_usb_status;
    }
}
