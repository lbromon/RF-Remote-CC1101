#ifndef __FAN_CONTROL__
#define __FAN_CONTROL__

#include <stdint.h>

#define NUM_FANS 4

void activarPersiana();
void activarPuerta();
void activarFan(int fanIndex, int commandIndex);

#endif //__FAN_CONTROL__