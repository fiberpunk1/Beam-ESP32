#include "gcodefifo.h"

GCODEFIFO::GCODEFIFO() 
{
  head = 0;
  tail = 0;
  numElements = 0;
//  buffer = (char**)heap_caps_malloc(GCODE_FIFO_SIZE*sizeof(char[48]), MALLOC_CAP_SPIRAM);
//   for(int i=0; i< GCODE_FIFO_SIZE; i++)
//   {
//        buffer[i] = (char*)malloc(48*sizeof(char));
//        memset(buffer[i],0,sizeof(char)*48);
//   }
}

GCODEFIFO::~GCODEFIFO() 
{
//    for(int i=0; i< GCODE_FIFO_SIZE; i++)
//    {
//        heap_caps_free(buffer[i]);
//    }
}

void GCODEFIFO::push(String data)
{
  if(numElements == GCODE_FIFO_SIZE) 
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
      tail %= GCODE_FIFO_SIZE;
    }
    data.toCharArray(buffer[tail], data.length());
  }
}

String GCODEFIFO::pop() 
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
      head %= GCODE_FIFO_SIZE;
    }
    ret_str = ret_str + '\n';
    return ret_str;
  }
}

void GCODEFIFO::clear()
{
  head = 0;
  tail = 0;
  numElements = 0;
  for(int i=0; i< GCODE_FIFO_SIZE; i++)
  {
       memset(buffer[i],0,sizeof(char)*64);
  }
}

int GCODEFIFO::size() 
{
  return numElements;
}
