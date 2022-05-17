#include "nodeconfig.h"

AsyncWebServer server(88);
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

OP_STATUS g_status=P_IDEL;
ERROR_CODE g_error=NORMAL;

bool recv_ok = false;
bool recvl_ok = false;
bool hasSD = false;
bool rst_usb = false;
bool paused_for_user = false;
bool paused_for_filament = false;

//printer sd type: 0==spi  1==sdio
uint8_t printer_sd_type = 0;

//last power status 0==idle  1==printing
uint8_t last_power_status = 0;
uint8_t print_start_flag = 0;
uint8_t reset_sd_559 = 0;
unsigned char current_usb_status = 0;
unsigned char pre_usb_status = 0;

uint8_t cmd_length=0;

void sendCaptureImage(String);
void sendHttpMsg(String);
void writeLog(String);

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
NodeConfig::NodeConfig()
{

}
