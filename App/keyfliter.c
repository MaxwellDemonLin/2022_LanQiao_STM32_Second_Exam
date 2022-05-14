#include "keyfliter.h"

keydata_t keydata;


void detect(key *key_t)
{
	if(key_t->count>5)
	{
		key_t->state=1;
	}
	else{
		key_t->state=0;
	}
}
void key_fliter()
{
	if(!HAL_GPIO_ReadPin(B1_GPIO_Port,B1_Pin))
	{
		keydata.B1.count++;
	}
	else{
		keydata.B1.count=0;
	}
	
	
	if(!HAL_GPIO_ReadPin(B2_GPIO_Port,B2_Pin))
	{
		keydata.B2.count++;
	}
	else{
		keydata.B2.count=0;
	}
	
	if(!HAL_GPIO_ReadPin(B3_GPIO_Port,B3_Pin))
	{
		keydata.B3.count++;
	}
	else{
		keydata.B3.count=0;
	}
	
	if(!HAL_GPIO_ReadPin(B4_GPIO_Port,B4_Pin))
	{
		keydata.B4.count++;
	}
	else{
		keydata.B4.count=0;
	}
	
	detect(&keydata.B1);
	detect(&keydata.B2);
	detect(&keydata.B3);
	detect(&keydata.B4);
	
}