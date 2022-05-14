#include "shop.h"
#include "main.h"
#include "keyfliter.h"
#include "stdio.h"
#include "string.h"
#include "i2c_hal.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"

uint8_t key_trigger(key *key_t);
behavior_e behaviour=BUY;
shop_data_t shop_data;

uint8_t LED_state[2];
extern keydata_t keydata;

void buy_task();
void price_task();
void rep_task();
void LED_ALLOFF();
void change_mode();
void EEPROM_Write(uint8_t add,uint8_t date);
uint8_t EEPROM_Read(uint8_t add);
void LED1_ON();
void LED_OFF(uint8_t NO);
void LED_task();
void shop_task()
{
	
	LED_task();
	change_mode();
	if(behaviour==BUY)
	{
		buy_task();
	}
	else if(behaviour== PRICE)
	{
		price_task();
	}
	else if(behaviour ==REP)
	{
		rep_task();
	}
}
void update_data()
{
		EEPROM_Write(1,shop_data.X.rep_num);
		EEPROM_Write(2,shop_data.Y.rep_num);
		EEPROM_Write(3,(uint8_t)(shop_data.X.price*10));
		EEPROM_Write(4,(uint8_t)(shop_data.Y.price*10));
}

char rebuffer;
void shop_init()
{
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);
	LED_ALLOFF();
	shop_data.X.buy_num=0;		
	shop_data.Y.buy_num=0;
	HAL_UART_Receive_IT(&huart1,&rebuffer,sizeof(char));
	HAL_TIM_Base_Stop_IT(&htim4);
	if(EEPROM_Read(0x42)!=42)
	{
		shop_data.Y.rep_num=10;
		shop_data.X.price=1.0f;
		shop_data.X.rep_num=10;
		shop_data.Y.price=1.0f;
		EEPROM_Write(0x42,42);
		update_data();
	}
	else{
		shop_data.X.rep_num=EEPROM_Read(1);
		shop_data.Y.rep_num=EEPROM_Read(2);
		shop_data.X.price=(float)EEPROM_Read(3)/10;
		shop_data.Y.price=(float)EEPROM_Read(4)/10;
	}
	HAL_TIM_Base_Start_IT(&htim3);
}
void buy_task()
{
	char cbuffer[20];
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"       SHOP");
	LCD_DisplayStringLine(20,(uint8_t*)cbuffer);
	
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"     X:%d",shop_data.X.buy_num);
	LCD_DisplayStringLine(80,(uint8_t*)cbuffer);
	
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"     Y:%d",shop_data.Y.buy_num);
	LCD_DisplayStringLine(130,(uint8_t*)cbuffer);
	
	
	if(key_trigger(&keydata.B2))
	{
		if(shop_data.X.buy_num<shop_data.X.rep_num)
		{
			shop_data.X.buy_num++;
		}
		else
		{
				LCD_ClearLine(80);
				shop_data.X.buy_num=0;
				update_data();
		}
	}
	if(key_trigger(&keydata.B3))	
	{
		if(shop_data.Y.buy_num<shop_data.Y.rep_num)
		{
			shop_data.Y.buy_num++;

		}
		else
		{
				LCD_ClearLine(130);
				shop_data.Y.buy_num=0;
				update_data();
		}
	}
	
	
	if(key_trigger(&keydata.B4))
	{
		if(shop_data.X.rep_num>=shop_data.X.buy_num&&shop_data.Y.rep_num>=shop_data.Y.buy_num)
		{
			char printbuffer[13];
			memset(printbuffer,sizeof(char),sizeof(printbuffer));
			sprintf(printbuffer,"X:%d,Y:%d,Z:%.1f",shop_data.X.buy_num,shop_data.Y.buy_num,shop_data.X.buy_num*shop_data.X.price+shop_data.Y.buy_num*shop_data.Y.price);
			HAL_UART_Transmit(&huart1,(uint8_t *)printbuffer,sizeof(printbuffer),20);
			
			
			shop_data.X.rep_num=shop_data.X.rep_num-shop_data.X.buy_num;
			shop_data.Y.rep_num=shop_data.Y.rep_num-shop_data.Y.buy_num;
			shop_data.X.buy_num=0;
			shop_data.Y.buy_num=0;
			update_data();
			
			
			LED_state[0]=1;
			LCD_ClearLine(80);
			LCD_ClearLine(130);
			__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,1500);
			HAL_TIM_Base_Start_IT(&htim4);
		}
	}
}


void price_task()
{
	char cbuffer[20];
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"       PRICE");
	LCD_DisplayStringLine(20,(uint8_t*)cbuffer);
	
	
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"     X:%.1f",shop_data.X.price);
	LCD_DisplayStringLine(80,(uint8_t*)cbuffer);
	
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"     Y:%.1f",shop_data.Y.price);
	LCD_DisplayStringLine(130,(uint8_t*)cbuffer);
	
	
	if(key_trigger(&keydata.B2))
	{
		if(shop_data.X.price<2)
		{
			shop_data.X.price+=0.1;
						update_data();
		}
				else
		{
				shop_data.X.price=1.0;
				update_data();
		}
	}
	if(key_trigger(&keydata.B3))	
	{
		if(shop_data.Y.price<2)
		{
			shop_data.Y.price+=0.1;
						update_data();
		}
		else
		{
				shop_data.Y.price=1.0;
				update_data();
		}
	}
}
void change_mode()
{
	if(key_trigger(&keydata.B1))
	{
		LCD_Clear(Black);
		behaviour=(behaviour+1)%3;
	}
}

uint8_t key_trigger(key *key_t)
{
	if(key_t->last_state==0&&key_t->state==1)
	{
		key_t->last_state=key_t->state;
		return 1;
	}
	else
	{
		key_t->last_state=key_t->state;
		return 0;
	}
}

void rep_task()
{
	
		char cbuffer[20];
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"       REP");
	LCD_DisplayStringLine(20,(uint8_t*)cbuffer);
	
	
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"     X:%d",shop_data.X.rep_num);
	LCD_DisplayStringLine(80,(uint8_t*)cbuffer);
	
	memset(cbuffer,sizeof(char),sizeof(cbuffer));
	sprintf(cbuffer,"     Y:%d",shop_data.Y.rep_num);
	LCD_DisplayStringLine(130,(uint8_t*)cbuffer);
	
	
	if(key_trigger(&keydata.B2))
	{
		shop_data.X.rep_num++;
					update_data();
	}
	if(key_trigger(&keydata.B3))	
	{
		shop_data.Y.rep_num++;
					update_data();
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart==&huart1)
	{

		if(rebuffer=='?')
		{
			char printbuffer[11];
			memset(printbuffer,sizeof(char),sizeof(printbuffer));
			sprintf(printbuffer,"X:%.1f,Y:%.1f",shop_data.X.price,shop_data.Y.price);
			HAL_UART_Transmit(&huart1,(uint8_t *)printbuffer,sizeof(printbuffer),20);
			HAL_UART_Receive_IT(&huart1,&rebuffer,sizeof(char));
		}
	}
	
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		if(htim==&htim4)
		{
			static uint8_t count=0;
			if(count==0)
			{
				count++;
				return;
			}
			
			LED_state[0]=0;
			__HAL_TIM_SetCompare(&htim2,TIM_CHANNEL_2,150);
			HAL_TIM_Base_Stop_IT(&htim4);
		}
		if(htim==&htim3)
		{
			if(shop_data.X.rep_num==0&&shop_data.Y.rep_num==0)
			{
				if(LED_state[1]==1)
				{
					LED_state[1]=0;
				}
				else
				{
					LED_state[1]=1;
				}
			}
			else
			{
				LED_state[1]=0;
			}
		}
}

void LED_ALLOFF()
{
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_SET);
	
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}
void LED1_ON()
{
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}
void LED2_ON()
{
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}

void LED12_ON()
{
	LED_ALLOFF();
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}


void LED_task()
{
	if(LED_state[0]&&LED_state[1])
	{
		 LED12_ON();
	}
	else if(LED_state[0])
	{
		LED1_ON();
	}
	else if(LED_state[1])
	{
		LED2_ON();
	}
	else{
		LED_ALLOFF();
	}
}
#define M24C02_Write   0xa0
#define M24C02_Read    0xa1
 
 
 
void EEPROM_Write(uint8_t add,uint8_t date)
{
	I2CStart();
	I2CSendByte(M24C02_Write);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	I2CSendByte(date);
	I2CWaitAck();
	I2CStop();//??
	HAL_Delay(1);
}

uint8_t EEPROM_Read(uint8_t add)
{
	I2CStart();
	I2CSendByte(M24C02_Write);
	I2CWaitAck();
	I2CSendByte(add);
	I2CWaitAck();
	I2CStart();
	I2CSendByte(M24C02_Read);
	I2CWaitAck();
	uint8_t val_EEPROM=I2CReceiveByte();
	I2CSendNotAck();
	I2CStop();//??
	HAL_Delay(1);
	return val_EEPROM;
}
