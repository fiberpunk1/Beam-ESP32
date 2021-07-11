#ifndef __GCODEFIFO__
#define __GCODEFIFO__

#include "Arduino.h"

#define GCODE_FIFO_SIZE 1024

class GCODEFIFO 
{
  private:
    int head;
    int tail;
    int numElements;
    char buffer[GCODE_FIFO_SIZE][48];
//    char **buffer;
  public:
    GCODEFIFO();
    ~GCODEFIFO();
    void push(String md);
    String pop();
    int size();
    void clear();
};

#endif
