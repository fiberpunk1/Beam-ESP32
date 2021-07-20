
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "esp_task_wdt.h"

#include "wifinode.h"
#include "nodeconfig.h"

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

WifiNode::WifiNode()
{

}

FiberPunk_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void WifiNode::init()
{
    //开启双核设置
    enableCore0WDT(); enableCore1WDT();
    esp_task_wdt_init(3, false);

    //0.初始化串口和OLED屏
    PRINTER_PORT.begin(100000);
    PRINTER_PORT.setTimeout(120);
    PRINTER_PORT.setRxBufferSize(512);
    PRINTER_PORT.setDebugOutput(true);
    // DBG_OUTPUT_PORT.print("\n");

    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);

    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);

    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    {
        // DBG_OUTPUT_PORT.println(F("OLED SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.setTextColor(SSD1306_WHITE);
    display.display();
    delay(2000); // Pause for 2 seconds
    // Clear the buffer
    display.clearDisplay();
    display.display();
   // text display tests
    display.setTextSize(1);
    display.setCursor(0,15);
    display.print("Checking SD Card ...");
    display.display(); // actually display all of the above   
    delay(2000);
    //1.初始化SD
    while(!SD_MMC.begin())
    {
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);

        // DBG_OUTPUT_PORT.println("Not Found SD Card.");
        display.clearDisplay();
        display.display();
        display.setTextSize(1);
        display.setCursor(0,15);
        display.print("Not Found SD Card.");
        display.display(); // actually display all of the above           
        delay(500);
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);  
        delay(500);      
    } 
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, HIGH);
    uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE){
        // Serial.println("No SD_MMC card attached");
        return;
    }
    // DBG_OUTPUT_PORT.println("SD Card initialized.");
    hasSD = true;
   
    //2.初始化wifi
    initwifi:
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setCursor(0,15);
    display.print("Waiting for WiFi...");
    display.display(); // actually display all of the above  
    delay(2000);
    //读取wifi账号密码，还有打印机名字
    File config_file = SD_MMC.open("/config.txt",FILE_READ);

    if(config_file)
    {
        String tmp_str = "";

        tmp_str = readConfig(config_file);
        cf_ssid = getValue(tmp_str, ':', 1);

        tmp_str = readConfig(config_file);
        cf_password = getValue(tmp_str, ':', 1);

        //device name
        tmp_str = readConfig(config_file);
        cf_node_name = getValue(tmp_str, ':', 1);

        //stop_x
        tmp_str = readConfig(config_file);
        tmp_str = getValue(tmp_str, ':', 1);
        stop_x = tmp_str.toInt();

        //stop_y
        tmp_str = readConfig(config_file);
        tmp_str = getValue(tmp_str, ':', 1);
        stop_y = tmp_str.toInt();

        //react e
        tmp_str = readConfig(config_file);
        tmp_str = getValue(tmp_str, ':', 1);
        react_length = tmp_str.toFloat();

        //b_time_laspe
        tmp_str = readConfig(config_file);
        tmp_str = getValue(tmp_str, ':', 1);
        b_time_laspe = (unsigned char)tmp_str.toInt();
    }
    config_file.close();

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
        display.clearDisplay();
        display.display();
        display.setTextSize(1);
        display.setCursor(0,15);
        display.print("Connect WiFi Wrong.");
        display.display(); // actually display all of the above  
        delay(2000);
        goto initwifi;
    }
    // DBG_OUTPUT_PORT.print("Connected! IP address: ");
    // DBG_OUTPUT_PORT.println(WiFi.localIP());
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Name:");
    display.println(cf_node_name);
    display.print("IP:");
    display.println(WiFi.localIP());
    display.println("Printer Ready!");
    display.display(); // actually display all of the above  
    //3. server init
    serverprocesser.serverInit();

    //4. crc init
    gcrc.begin();
}

void WifiNode::process()
{
    serverprocesser.serverLoop();
}
