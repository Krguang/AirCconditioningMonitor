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


static uint32_t timerMsCount;

/** User area the current device state structure*/
dataPoint_t currentDataPoint;


UART_HandleTypeDef USART_Handler; //UART handle
TIM_HandleTypeDef TIM3_Handler;   //timer handle

								  /**@} */
								  /**@name Gizwits User Interface
								  * @{
								  */

								  /**
								  * @brief Event handling interface

								  * Description:

								  * 1. Users can customize the changes in WiFi module status

								  * 2. Users can add data points in the function of event processing logic, such as calling the relevant hardware peripherals operating interface

								  * @param [in] info: event queue
								  * @param [in] data: protocol data
								  * @param [in] len: protocol data length
								  * @return NULL
								  * @ref gizwits_protocol.h
								  */
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
			}
			else
			{
				//user handle    
			}
			break;
		case EVENT_SW_ZhiBan:
			currentDataPoint.valueSW_ZhiBan = dataPointPtr->valueSW_ZhiBan;
			GIZWITS_LOG("Evt: EVENT_SW_ZhiBan %d \n", currentDataPoint.valueSW_ZhiBan);
			if (0x01 == currentDataPoint.valueSW_ZhiBan)
			{
				//user handle
			}
			else
			{
				//user handle    
			}
			break;
		case EVENT_SW_FuYa:
			currentDataPoint.valueSW_FuYa = dataPointPtr->valueSW_FuYa;
			GIZWITS_LOG("Evt: EVENT_SW_FuYa %d \n", currentDataPoint.valueSW_FuYa);
			if (0x01 == currentDataPoint.valueSW_FuYa)
			{
				//user handle
			}
			else
			{
				//user handle    
			}
			break;


		case EVENT_WenDuSet:
			currentDataPoint.valueWenDuSet = dataPointPtr->valueWenDuSet;
			GIZWITS_LOG("Evt:EVENT_WenDuSet %d\n", currentDataPoint.valueWenDuSet);
			//user handle
			break;
		case EVENT_ShiDuSet:
			currentDataPoint.valueShiDuSet = dataPointPtr->valueShiDuSet;
			GIZWITS_LOG("Evt:EVENT_ShiDuSet %d\n", currentDataPoint.valueShiDuSet);
			//user handle
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
	/*
	currentDataPoint.valueZS_JiZuYunXing = ;//Add Sensor Data Collection
	currentDataPoint.valueZS_ZhiBanYunXing = ;//Add Sensor Data Collection
	currentDataPoint.valueZS_FuYaYunXing = ;//Add Sensor Data Collection
	currentDataPoint.valueZS_JiZuGuZhang = ;//Add Sensor Data Collection
	currentDataPoint.valueZS_GaoXiaoZuSe = ;//Add Sensor Data Collection
	currentDataPoint.valueWenDuZhi = ;//Add Sensor Data Collection
	currentDataPoint.valueShiDuZhi = ;//Add Sensor Data Collection
	currentDataPoint.valueYaChaZhi = ;//Add Sensor Data Collection
	currentDataPoint.valueLengShuiFa = ;//Add Sensor Data Collection
	currentDataPoint.valueReShuiFa = ;//Add Sensor Data Collection
	currentDataPoint.valueJiaShuiQi = ;//Add Sensor Data Collection

	*/

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

void uartInit() {

}

void timerInit() {

}


/**
* @brief Timer TIM3 interrupt handler

* @param none
* @return none
*/
void TIM3_IRQHandler(void)
{
	if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET)
	{
		if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET)
		{
			__HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
			gizTimerMs();
		}
	}
}



/**
* @brief USART2 interrupt handler

* Receive the serial protocol data from the WiFi module
* @param none
* @return none
*/
void USART2_IRQHandler(void)
{
	uint8_t value = 0;

	if ((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET))
	{
		value = huart2.Instance->DR;
		gizPutData(&value, 1);
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
		HAL_UART_Transmit(&USART_Handler, &buf[i], 1, 1000);
		while (__HAL_UART_GET_FLAG(&USART_Handler, UART_FLAG_TXE) == RESET);//Loop until the end of transmission

		if (i >= 2 && buf[i] == 0xFF)
		{
			HAL_UART_Transmit(&USART_Handler, data55, 1, 1000);
			while (__HAL_UART_GET_FLAG(&USART_Handler, UART_FLAG_TXE) == RESET);//Loop until the end of transmission
		}
	}

	return len;
}

