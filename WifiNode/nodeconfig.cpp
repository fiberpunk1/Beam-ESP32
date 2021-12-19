#include "nodeconfig.h"

WebServer server(80);
HTTPClient http_client;
WiFiClient socket_client;
CRC8 gcrc;

File uploadFile;

String pre_line="";
String cf_ssid = "";
String cf_password = "";
String cf_node_name = "";
String total_layers = "";
String current_layers = "";
String current_temp = "";
String current_bed_temp = "";
String pc_ipaddress = "";

OP_STATUS g_status=P_IDEL;
ERROR_CODE g_error=NORMAL;


bool recv_ok = false;
bool recvl_ok = false;
bool hasSD = false;
bool rst_usb = false;
bool paused_for_user = false;

unsigned char current_usb_status = 0;
unsigned char pre_usb_status = 0;



uint8_t cmd_length=0;


void sendHttpMsg(String);
void writeLog(String);

void writeLog(String log_txt)
{
    // if(g_status==PRINTING)
    {
        sendHttpMsg(log_txt);
    }
}

void sendHttpMsg(String url)
{
#if 0
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
    if(socket_client.connected())
        socket_client.print(url);
    
}
NodeConfig::NodeConfig()
{

}
