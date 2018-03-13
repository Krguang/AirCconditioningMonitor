/**
************************************************************
* @file         gizwits_product.c
* @brief        Gizwits control protocol processing, and platform-related hardware initialization
* @author       Gizwits
* @date         2017-07-19
* @version      V03030000
* @copyright    Gizwits
*
* @note         机智云.只为智能硬件而生
*               Gizwits Smart Cloud  for Smart Products
*               链接|增值ֵ|开放|中立|安全|自有|自由|生态
*               www.gizwits.com
*
***********************************************************/

#include <stdio.h>
#include <string.h>
#include "gizwits_product.h"
#include "common.h"
#include "usart.h"
#include "tim.h"
#include "stmFlash.h"

static uint32_t timerMsCount;

/** User area the current device state structure*/
dataPoint_t currentDataPoint;

uint16_t localArray[128];
uint16_t tempAndHumi[2];//0:温度设定值，1：湿度设定值

#define FLASH_SAVE_ADDR  0X0800F000		//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)


int8_t gizwitsEventProcess(eventInfo_t *info, uint8_t *gizdata, uint32_t len)
{
	uint8_t i = 0;
	dataPoint_t *dataPointPtr = (dataPoint_t *)gizdata;
	moduleStatusInfo_t *wifiData = (moduleStatusInfo_t *)gizdata;
	protocolTime_t *ptime = (protocolTime_t *)gizdata;

#if MODULE_TYPE
	gprsInfo_t *gprsInfoData = (gprsInfo_t *)gizdata;
#else
	moduleInfo_t *ptModuleInfo = (moduleInfo_t *)gizdata;
#endif

	if ((NULL == info) || (NULL == gizdata))
	{
		return -1;
	}

	for (i = 0; i<info->num; i++)
	{
		switch (info->event[i])
		{
		case EVENT_SW_KongTiao:
			currentDataPoint.valueSW_KongTiao = dataPointPtr->valueSW_KongTiao;
			GIZWITS_LOG("Evt: EVENT_SW_KongTiao %d \n", currentDataPoint.valueSW_KongTiao);
			if (0x01 == currentDataPoint.valueSW_KongTiao)
			{
				//user handle
				localArray[0] = 1;
			}
			else
			{
				//user handle    
				localArray[0] = 0;
			}
			break;
		case EVENT_SW_ZhiBan:
			currentDataPoint.valueSW_ZhiBan = dataPointPtr->valueSW_ZhiBan;
			GIZWITS_LOG("Evt: EVENT_SW_ZhiBan %d \n", currentDataPoint.valueSW_ZhiBan);
			if (0x01 == currentDataPoint.valueSW_ZhiBan)
			{
				//user handle
				localArray[1] = 1;
			}
			else
			{
				//user handle    
				localArray[1] = 0;
			}
			break;
		case EVENT_SW_FuYa:
			currentDataPoint.valueSW_FuYa = dataPointPtr->valueSW_FuYa;
			GIZWITS_LOG("Evt: EVENT_SW_FuYa %d \n", currentDataPoint.valueSW_FuYa);
			if (0x01 == currentDataPoint.valueSW_FuYa)
			{
				//user handle
				localArray[3] = 1;
			}
			else
			{
				//user handle    
				localArray[3] = 0;
			}
			break;


		case EVENT_WenDuSet:
			currentDataPoint.valueWenDuSet = dataPointPtr->valueWenDuSet;
			GIZWITS_LOG("Evt:EVENT_WenDuSet %d\n", currentDataPoint.valueWenDuSet);
			//user handle
			localArray[5] = currentDataPoint.valueWenDuSet;
			break;
		case EVENT_ShiDuSet:
			currentDataPoint.valueShiDuSet = dataPointPtr->valueShiDuSet;
			GIZWITS_LOG("Evt:EVENT_ShiDuSet %d\n", currentDataPoint.valueShiDuSet);
			//user handle
			localArray[6] = currentDataPoint.valueShiDuSet;
			break;
		case EVENT_YaChaSet:
			currentDataPoint.valueYaChaSet = dataPointPtr->valueYaChaSet;
			GIZWITS_LOG("Evt:EVENT_YaChaSet %d\n", currentDataPoint.valueYaChaSet);
			//user handle
			break;


		case WIFI_SOFTAP:
			break;
		case WIFI_AIRLINK:
			break;
		case WIFI_STATION:
			break;
		case WIFI_CON_ROUTER:

			break;
		case WIFI_DISCON_ROUTER:

			break;
		case WIFI_CON_M2M:

			break;
		case WIFI_DISCON_M2M:
			break;
		case WIFI_RSSI:
			GIZWITS_LOG("RSSI %d\n", wifiData->rssi);
			break;
		case TRANSPARENT_DATA:
			GIZWITS_LOG("TRANSPARENT_DATA \n");
			//user handle , Fetch data from [data] , size is [len]
			break;
		case WIFI_NTP:
			GIZWITS_LOG("WIFI_NTP : [%d-%d-%d %02d:%02d:%02d][%d] \n", ptime->year, ptime->month, ptime->day, ptime->hour, ptime->minute, ptime->second, ptime->ntp);
			break;
		case MODULE_INFO:
			GIZWITS_LOG("MODULE INFO ...\n");
#if MODULE_TYPE
			GIZWITS_LOG("GPRS MODULE ...\n");
			//Format By gprsInfo_t
#else
			GIZWITS_LOG("WIF MODULE ...\n");
			//Format By moduleInfo_t
			GIZWITS_LOG("moduleType : [%d] \n", ptModuleInfo->moduleType);
#endif
			break;
		default:
			break;
		}
	}

	return 0;
}

/**
* User data acquisition

* Here users need to achieve in addition to data points other than the collection of data collection, can be self-defined acquisition frequency and design data filtering algorithm

* @param none
* @return none
*/
void userHandle(void)
{
	
	if (tempAndHumi[0]!=localArray[5]||tempAndHumi[1]!=localArray[6])
	{
		tempAndHumi[0] = localArray[5];
		tempAndHumi[1] = localArray[6];
		STMFLASH_Write(FLASH_SAVE_ADDR, (uint16_t *)tempAndHumi, 2);
	}
	

	currentDataPoint.valueZS_JiZuYunXing = localArray[9]&1;//Add Sensor Data Collection
	currentDataPoint.valueZS_ZhiBanYunXing = localArray[10]&1;//Add Sensor Data Collection
	currentDataPoint.valueZS_FuYaYunXing = (localArray[10]>>1)&1;//Add Sensor Data Collection
	currentDataPoint.valueZS_JiZuGuZhang = localArray[11]&1;//Add Sensor Data Collection
	currentDataPoint.valueZS_GaoXiaoZuSe = localArray[12]&1;//Add Sensor Data Collection
	currentDataPoint.valueWenDuZhi = localArray[7];//Add Sensor Data Collection
	currentDataPoint.valueShiDuZhi = localArray[8];//Add Sensor Data Collection
	currentDataPoint.valueWenDuSet = localArray[5];
	currentDataPoint.valueShiDuSet = localArray[6];
//	currentDataPoint.valueYaChaZhi = ;//Add Sensor Data Collection
	currentDataPoint.valueLengShuiFa = localArray[13];//Add Sensor Data Collection
	currentDataPoint.valueReShuiFa = localArray[14];//Add Sensor Data Collection
	currentDataPoint.valueJiaShuiQi = localArray[15];//Add Sensor Data Collection

}

/**
* Data point initialization function

* In the function to complete the initial user-related data
* @param none
* @return none
* @note The developer can add a data point state initialization value within this function
*/
void userInit(void)
{
	memset((uint8_t*)&currentDataPoint, 0, sizeof(dataPoint_t));
	
	
	STMFLASH_Read(FLASH_SAVE_ADDR, (uint16_t *)tempAndHumi, 2);

	if (tempAndHumi[0]>999||tempAndHumi[1]>999)
	{
		tempAndHumi[0] = 250;
		tempAndHumi[1] = 500;
		STMFLASH_Write(FLASH_SAVE_ADDR, (uint16_t *)tempAndHumi, 2);
		
	}

	localArray[5] = tempAndHumi[0];
	localArray[6] = tempAndHumi[1];
	

	/** Warning !!! DataPoint Variables Init , Must Within The Data Range **/
	/*
	currentDataPoint.valueSW_KongTiao = ;
	currentDataPoint.valueSW_ZhiBan = ;
	currentDataPoint.valueSW_FuYa = ;
	currentDataPoint.valueWenDuSet = ;
	currentDataPoint.valueShiDuSet = ;
	currentDataPoint.valueYaChaSet = ;
	currentDataPoint.valueZS_JiZuYunXing = ;
	currentDataPoint.valueZS_ZhiBanYunXing = ;
	currentDataPoint.valueZS_FuYaYunXing = ;
	currentDataPoint.valueZS_JiZuGuZhang = ;
	currentDataPoint.valueZS_GaoXiaoZuSe = ;
	currentDataPoint.valueWenDuZhi = ;
	currentDataPoint.valueShiDuZhi = ;
	currentDataPoint.valueYaChaZhi = ;
	currentDataPoint.valueLengShuiFa = ;
	currentDataPoint.valueReShuiFa = ;
	currentDataPoint.valueJiaShuiQi = ;
	*/

}


/**
* @brief Millisecond timing maintenance function, milliseconds increment, overflow to zero

* @param none
* @return none
*/
void gizTimerMs(void)
{
	timerMsCount++;
}

/**
* @brief Read millisecond count

* @param none
* @return millisecond count
*/
uint32_t gizGetTimerCount(void)
{
	return timerMsCount;
}


/**
* @brief MCU reset function

* @param none
* @return none
*/
void mcuRestart(void)
{
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}

/**@} */




/**
* @brief 定时器TIM中断处理函数

* @param none
* @return none
*/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3)//tim10 1ms中断，作为MCU和WIFI模组的心跳用
	{
		gizTimerMs();
	}
	if (htim->Instance == TIM4)//tim11 1ms中断，按键检测逻辑
	{

	}
}


/**
* @brief USART串口中断函数

* 接收功能，用于接收与WiFi模组间的串口协议数据
* @param none
* @return none
*/

uint8_t RxData = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if (UartHandle->Instance == USART2)
	{
		gizPutData(&RxData, 1);
		HAL_UART_Receive_IT(&huart2, &RxData, 1);
	}

}


/**
* @brief Serial port write operation, send data to WiFi module
*
* @param buf      : buf address
* @param len      : buf length
*
* @return : Return effective data length;-1，return failure
*/
int32_t uartWrite(uint8_t *buf, uint32_t len)
{
	uint32_t i = 0;
	uint8_t data55[1] = { 0x55 };

	if (NULL == buf)
	{
		return -1;
	}
#ifdef PROTOCOL_DEBUG
	GIZWITS_LOG("MCU2WiFi[%4d:%4d]: ", gizGetTimerCount(), len);
	for (i = 0; i<len; i++)
	{
		GIZWITS_LOG("%02x ", buf[i]);

		if (i >= 2 && buf[i] == 0xFF)
		{
			GIZWITS_LOG("%02x ", 0x55);
		}
	}

	GIZWITS_LOG("\n");
#endif

	for (i = 0; i<len; i++)
	{
		HAL_UART_Transmit(&huart2, &buf[i], 1, 1000);
		while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) == RESET);//Loop until the end of transmission

		if (i >= 2 && buf[i] == 0xFF)
		{
			HAL_UART_Transmit(&huart2, data55, 1, 1000);
			while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) == RESET);//Loop until the end of transmission
		}
	}

	return len;
}

void uartInit() {
	HAL_UART_Receive_IT(&huart2, &RxData, 1);
}

void timerInit() {
	HAL_TIM_Base_Start_IT(&htim3);
}