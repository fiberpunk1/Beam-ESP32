#ifndef __CMDFIFO__
#define __CMDFIFO__

#include "Arduino.h"

#define FIFO_SIZE 24

class CMDFIFO 
{
  private:
    int head;
    int tail;
    int numElements;
    char buffer[FIFO_SIZE][64];
  public:
    CMDFIFO();
    ~CMDFIFO();
    void push(String md);
    String pop();
    int size();
    void clear();
};

#endif