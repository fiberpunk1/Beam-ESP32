#ifndef NODECONFIG_H
#define NODECONFIG_H

#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <SPI.h>
#include <Arduino.h>
#include <SD.h>

#include "cmdfifo.h"

#define DBG_OUTPUT_PORT Serial
#define PRINTER_PORT Serial
#define RED_LED 27
#define GREEN_LED 26
#define BLUE_LED 25

enum OP_STATUS
{
  P_IDEL=0, 
  PRINTING=1,
  PAUSE=2,
  RECOVER=3,
  CANCLE=4,
  HEEATING=5,
};

enum ERROR_CODE
{
  NORMAL=0,
  OPEN_FAILED=1,
};

extern CMDFIFO cmd_fifo;
extern CMDFIFO setting_fifo;
extern WebServer server;
extern WiFiClient client;

extern File uploadFile;
extern File g_printfile;

extern String pre_line;
extern String cf_ssid;
extern String cf_password;
extern String cf_node_name;
extern String total_layers;
extern String current_layers;
extern String current_temp;
extern String current_bed_temp;
extern String pc_ipaddress;

extern unsigned int stop_x;
extern unsigned int stop_y;
extern unsigned char b_time_laspe;
extern float react_length;



extern OP_STATUS g_status;
extern ERROR_CODE g_error;


extern bool hasSD;
extern bool recv_ok;
extern bool recvl_ok;


extern unsigned long sendGcode_cnt;
extern unsigned long recGok_cnt;


extern unsigned long sendGcode_cnt;
extern unsigned long recGok_cnt;


class NodeConfig
{
public:
    NodeConfig();
};

#endif // NODECONFIG_H
