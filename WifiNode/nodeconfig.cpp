#include "nodeconfig.h"

AsyncWebServer server(88);
AsyncWebServer octo_server(80);
WebServer config_wifi_server(81);
AsyncEventSource events("/events");

HTTPClient http_client;
AsyncClient socket_client;
CRC8 gcrc;

File uploadFile;

String pre_line="";
String cf_ssid = "";
String cf_password = "";
String cf_node_name = "";
unsigned int  cf_filament = 0;
String total_layers = "";
String current_layers = "";
String current_temp = "";
String current_bed_temp = "";
String pc_ipaddress = "";
String current_file="";
String current_upload_file = "";

String user1_cmd_f_name = "user1";
String user2_cmd_f_name = "user2";
String user3_cmd_f_name = "user3";
String user4_cmd_f_name = "user4";
String user1_cmd = "";
String user2_cmd = "";
String user3_cmd = "";
String user4_cmd = "";

LED_STATUS g_led_status=LED_RED;
OP_STATUS g_status=P_IDEL;
ERROR_CODE g_error=NORMAL;

bool recv_ok = false;
bool recvl_ok = false;
bool hasSD = false;
bool rst_usb = false;
bool paused_for_user = false;
bool paused_for_filament = false;
bool b_print_after_upload = false;

//printer sd type: 0==spi  1==sdio
#if MB(MARLIN_VER)
  uint8_t printer_sd_type = 0;
#elif MB(PRUSA_VER)
  uint8_t printer_sd_type = 1;
#endif


//last power status 0==idle  1==printing
uint8_t last_power_status = 0;

//power with sd 0=without  1=with
uint8_t b_init_with_sd = 0;

uint8_t print_start_flag = 0;
uint8_t reset_sd_usb = 0;

unsigned char current_usb_status = 0;
unsigned char pre_usb_status = 0;


uint8_t cmd_length=0;

void sendCaptureImage(String);
void sendHttpMsg(String);
void writeLog(String);
String genRandomHeader(int length);
String renameRandom(String filename);
String convertToShortName(String filename);

void writeLog(String log_txt)
{
    // if(g_status==PRINTING)
    {
        events.send(log_txt.c_str(), "gcode_cli");
        sendHttpMsg(log_txt);
    }
}
void sendCaptureImage(String url)
{
#if 1
    String http_url = "http://"+pc_ipaddress+":8002/"+"api/"+url;
    http_client.begin(http_url);
    int httpRsponCode = http_client.POST(http_url);
    // Serial.print(http_url);
    if(httpRsponCode>0)
    {
        //message send succesful
    }
    else
    {
        //send failed
    }
#endif 
}
void sendHttpMsg(String url)
{
  if(socket_client.connected())
      socket_client.write(url.c_str());
}

String genRandomHeader(int length)
{
    String result = "";
    String charters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int chater_len = charters.length();
    int random_in = 0;
    for(int i=0; i<length; i++)
    {
        random_in = random(0,61);
        result.concat(charters.charAt(random_in));
    }
    return result;
}

String renameRandom(String filename)
{
  filename = filename.substring(1,filename.length());
  String header = "/"+genRandomHeader(3)+"-";
  header = header + filename;
  return header;
}

String convertToShortName(String filename)
{
    filename = filename.substring(1,filename.length()-5);
    String base_name = "";
    if(filename.length()>=6)
    {
        base_name = filename.substring(0,6) + "~1";
    }
    else
    {
        base_name = filename + "~1";
    }
    String full_name = "/"+base_name+".gco";
    return full_name;
}

NodeConfig::NodeConfig()
{

}
