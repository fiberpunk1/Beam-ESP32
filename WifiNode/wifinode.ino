#include "wifinode.h"
#include "printerprocess.h"

#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "esp_task_wdt.h"


WifiNode node;
PrinterProcess printer;

void setup()
{
    node.init();
    enableCore0WDT(); 
    enableCore1WDT();
    esp_task_wdt_init(3, false);
    // disableCore0WDT();
    // disableCore1WDT();
    xTaskCreatePinnedToCore(printLoop, "Task0", 10000, NULL, 1, NULL,  0); 
}


void loop()
{
    node.process();
}
