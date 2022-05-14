#ifndef __KEY_H
#define __KEY_H
#include "main.h"




typedef struct{
	uint8_t state;
	uint8_t last_state;
	int count;
}key;

typedef struct{
	key B1;
	key B2;
	key B3;
	key B4;
}keydata_t;

void key_fliter();
#endif