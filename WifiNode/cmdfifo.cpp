#include "cmdfifo.h"

CMDFIFO::CMDFIFO() 
{
  head = 0;
  tail = 0;
  numElements = 0;
}

CMDFIFO::~CMDFIFO() 
{
}

void CMDFIFO::push(String data)
{
  if(numElements == FIFO_SIZE) 
  {
    return;
  }
  else 
  {
    //Increment size
    numElements++;

    //Only move the tail if there is more than one element
    if(numElements > 1) 
    {
      tail++;
      tail %= FIFO_SIZE;
    }
    data.toCharArray(buffer[tail], data.length());
  }
}

String CMDFIFO::pop() 
{
  if(numElements == 0) 
  {
//    Serial.println(F("Buffer empty"));
    return "";
  }
  else 
  {
    //Decrement size
    numElements--;
    String ret_str(buffer[head]);
    if(numElements >= 1) {
      //Move head up one position
      head++;
      //Make sure head is within the bounds of the array
      head %= FIFO_SIZE;
    }
    ret_str = ret_str + '\n';
    return ret_str;
  }
}

void CMDFIFO::clear()
{
  head = 0;
  tail = 0;
  numElements = 0;
}

int CMDFIFO::size() 
{
  return numElements;
}