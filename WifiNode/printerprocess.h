#ifndef PRINTERPROCESS_H
#define PRINTERPROCESS_H

#include "Arduino.h"

extern void printFile(String file);
extern void printLoop(void * parameter);

class PrinterProcess
{
public:
    PrinterProcess();
};

#endif // PRINTERPROCESS_H
