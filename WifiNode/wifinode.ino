#include "wifinode.h"
#include "printerprocess.h"


#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "soc/rtc_wdt.h"

#include "esp_task_wdt.h"



WifiNode node;
PrinterProcess printer;

void setup()
{
    node.init();
    rtc_wdt_set_length_of_reset_signal(RTC_WDT_SYS_RESET_SIG, RTC_WDT_LENGTH_3_2us);
    rtc_wdt_set_stage(RTC_WDT_STAGE0, RTC_WDT_STAGE_ACTION_RESET_SYSTEM);
    rtc_wdt_set_time(RTC_WDT_STAGE0, 2000);
    xTaskCreatePinnedToCore(printLoop, "Task0", 10000, NULL, 2, NULL,  0); 
}


void loop()
{
    node.process();
}
