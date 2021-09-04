#include "nodeconfig.h"

WebServer server(80);
HTTPClient http_client;
// WiFiClient socket_client;
CRC8 gcrc;

GCODEFIFO cmd_fifo;
CMDFIFO setting_fifo;

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

unsigned int stop_x = 200;
unsigned int stop_y = 200;
unsigned char b_time_laspe = 1;
float react_length = 4.5;

File g_printfile;
OP_STATUS g_status=P_IDEL;
ERROR_CODE g_error=NORMAL;

unsigned int sendGcode_cnt = 0;
unsigned int recGok_cnt = 0;

bool recv_ok = false;
bool recvl_ok = false;
bool hasSD = false;
bool resend = false;
bool rst_usb = false;

unsigned char current_usb_status = 0;
unsigned char pre_usb_status = 0;


unsigned long timecnt = 0;
uint8_t cmd_length=0;


void sendHttpMsg(String);

void sendHttpMsg(String url)
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
    // socket_client.print(url);
    
}
NodeConfig::NodeConfig()
{

}
