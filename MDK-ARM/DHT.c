#include "DHT.h"
//************************** Low Level Layer ********************************************************//
#include "delay_timer.h"

static void DHT_DelayInit(DHT_Name *DHT)
{
	DELAY_TIM_Init(DHT->Timer);
}
static void DHT_DelayUs(DHT_Name *DHT, uint16_t Time)
{
	DELAY_TIM_Us(DHT->Timer, Time);
}

static void DHT_SetPinOut(DHT_Name *DHT)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT->Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DHT->PORT, &GPIO_InitStruct);
}
static void DHT_SetPinIn(DHT_Name *DHT)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = DHT->Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(DHT->PORT, &GPIO_InitStruct);
}
static void DHT_WritePin(DHT_Name *DHT, uint8_t Value)
{
	HAL_GPIO_WritePin(DHT->PORT, DHT->Pin, Value);
}
static uint8_t DHT_ReadPin(DHT_Name *DHT)
{
	uint8_t Value;
	Value = HAL_GPIO_ReadPin(DHT->PORT, DHT->Pin);
	return Value;
}
//********************************* Middle level Layer ****************************************************//
static void DHT_Start(DHT_Name *DHT)
{
	DHT_DelayInit(DHT);
	DHT_SetPinOut(DHT);
	DHT_WritePin(DHT, 0);
	DHT_DelayUs(DHT, DHT->Type);
	DHT_SetPinIn(DHT);
}
int checkResponse(DHT_Name *DHT)
{
	int response = 0;
	DHT_DelayUs(DHT, 40);
	if (!DHT_ReadPin(DHT))
	{
		DHT_DelayUs(DHT, 80);
		if ((DHT_ReadPin(DHT)))
			response = 1;
		else
			response = -1;
	}
	while ((DHT_ReadPin(DHT)))
		;
	return response;
}
static uint8_t DHT_Read(DHT_Name *DHT)
{
	uint8_t Value = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		while (!(DHT_ReadPin(DHT)))
			;					 // wait for the pin go to high
		DHT_DelayUs(DHT, 40);	 // wait for 40 us
		if (!(DHT_ReadPin(DHT))) // if the pin is low
		{
			Value &= ~(1 << (7 - i)); // write 0
		}
		else
			Value |= (1 << (7 - i)); // if the pin is high, write 1
		while ((DHT_ReadPin(DHT)))
			; // wait for the pin go to low
	}
	return Value;
}

//************************** High Level Layer ********************************************************//
void DHT_Init(DHT_Name *DHT, uint8_t DHT_Type, TIM_HandleTypeDef *Timer, GPIO_TypeDef *DH_PORT, uint16_t DH_Pin)
{
	if (DHT_Type == DHT11)
	{
		DHT->Type = DHT11_STARTTIME;
	}
	else if (DHT_Type == DHT22)
	{
		DHT->Type = DHT22_STARTTIME;
	}
	DHT->PORT = DH_PORT;
	DHT->Pin = DH_Pin;
	DHT->Timer = Timer;
	DHT_DelayInit(DHT);
}

void DHT_ReadTempHum(DHT_Name *DHT)
{
	DHT_Start(DHT);
	uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
	unsigned int SUM;
	uint8_t Presence = 0;
	Presence = checkResponse(DHT);
	Rh_byte1 = DHT_Read(DHT);
	Rh_byte2 = DHT_Read(DHT);
	Temp_byte1 = DHT_Read(DHT);
	Temp_byte2 = DHT_Read(DHT);
	SUM = DHT_Read(DHT);
	if (SUM == (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2))
	{
		DHT->Temp = Temp_byte1;
		DHT->Humi = Rh_byte1;
	}
}
