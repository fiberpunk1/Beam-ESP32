#ifndef NODECONFIG_H
#define NODECONFIG_H

#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <SD_MMC.h>
#include <FS.h>

#include "cmdfifo.h"
#include "gcodefifo.h"
#include "crc8.h"
#include "FiberPunk_SSD1306.h"

#define VERSION 1001
#define DBG_OUTPUT_PORT Serial
#define PRINTER_PORT Serial
#define RED_LED 27
#define GREEN_LED 26
#define BLUE_LED 25

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


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

extern GCODEFIFO cmd_fifo;
extern CMDFIFO setting_fifo;
extern WebServer server;
extern WiFiClient client;
extern CRC8 gcrc;

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

extern unsigned long timecnt;
extern bool time_out;


extern bool hasSD;
extern bool recv_ok;
extern bool recvl_ok;
extern bool resend;

extern unsigned int sendGcode_cnt;
extern unsigned int recGok_cnt;

extern uint8_t cmd_length;

class NodeConfig
{
public:
    NodeConfig();
};

#endif // NODECONFIG_H
