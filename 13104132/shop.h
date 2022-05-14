#ifndef __SHOP_H
#define __SHOP_H
#include "main.h"
typedef enum
{
	BUY=0,
	PRICE,
	REP,
}behavior_e;
typedef struct
{
	float price;
	uint16_t rep_num;
	uint16_t buy_num;
}goods_t;
typedef struct
{
	goods_t X;
	goods_t Y
}shop_data_t;

void shop_task();
void shop_init();




#endif