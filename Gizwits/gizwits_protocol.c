/**
************************************************************
* @file         gizwits_protocol.c
* @brief        Corresponding gizwits_product.c header file (including product hardware and software version definition)
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
#include "ringBuffer.h"
#include "gizwits_product.h"
#include "dataPointTools.h"

/** Protocol global variables **/
gizwitsProtocol_t gizwitsProtocol;


/**@name The serial port receives the ring buffer implementation
* @{	串口接收环缓冲实现
*/
rb_t pRb;                                               ///< Ring buffer structure variable 循环缓冲区结构变量
static uint8_t rbBuf[RB_MAX_LEN];                       ///< Ring buffer data cache buffer  环缓冲数据缓存缓冲区


/**@} */

/**
* @brief Write data to the ring buffer 将数据写入环缓冲区
* @param [in] buf        : buf adress 
* @param [in] len        : byte length
* @return   correct : Returns the length of the written data
failure : -1
*/
int32_t gizPutData(uint8_t *buf, uint32_t len)
{
	int32_t count = 0;

	if (NULL == buf)
	{
		GIZWITS_LOG("ERR: gizPutData buf is empty \n");
		return -1;
	}

	count = rbWrite(&pRb, buf, len);
	if (count != len)
	{
		GIZWITS_LOG("ERR: Failed to rbWrite \n");
		return -1;
	}

	return count;
}



/**
* @brief Protocol header initialization  协议头初始化
*
* @param [out] head         : Protocol header pointer
*
* @return 0， success; other， failure
*/
static int8_t gizProtocolHeadInit(protocolHead_t *head)
{
	if (NULL == head)
	{
		GIZWITS_LOG("ERR: gizProtocolHeadInit head is empty \n");
		return -1;
	}

	memset((uint8_t *)head, 0, sizeof(protocolHead_t));
	head->head[0] = 0xFF;
	head->head[1] = 0xFF;

	return 0;
}

/**
* @brief Protocol ACK check processing function 协议ACK检查处理功能
*
* @param [in] data            : data adress
* @param [in] len             : data length
*
* @return 0， suceess; other， failure
*/
static int8_t gizProtocolWaitAck(uint8_t *gizdata, uint32_t len)
{
	if (NULL == gizdata)
	{
		GIZWITS_LOG("ERR: data is empty \n");
		return -1;
	}

	memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
	memcpy((uint8_t *)gizwitsProtocol.waitAck.buf, gizdata, len);
	gizwitsProtocol.waitAck.dataLen = (uint16_t)len;

	gizwitsProtocol.waitAck.flag = 1;
	gizwitsProtocol.waitAck.sendTime = gizGetTimerCount();

	return 0;
}
/**
* @brief generates "controlled events" according to protocol 根据协议生成“受控事件”

* @param [in] issuedData: Controlled data			受控数据
* @param [out] info: event queue					事件队列
* @param [out] dataPoints: data point data			数据点数据
* @return 0, the implementation of success, non-0, failed
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoint2Event(gizwitsIssued_t *issuedData, eventInfo_t *info, dataPoint_t *dataPoints)
{
	if ((NULL == issuedData) || (NULL == info) || (NULL == dataPoints))
	{
		GIZWITS_LOG("gizDataPoint2Event Error , Illegal Param\n");
		return -1;
	}

	/** Greater than 1 byte to do bit conversion **/
	if (sizeof(issuedData->attrFlags) > 1)
	{
		if (-1 == gizByteOrderExchange((uint8_t *)&issuedData->attrFlags, sizeof(attrFlags_t)))
		{
			GIZWITS_LOG("gizByteOrderExchange Error\n");
			return -1;
		}
	}

	if (0x01 == issuedData->attrFlags.flagSW_KongTiao)
	{
		info->event[info->num] = EVENT_SW_KongTiao;
		info->num++;
		dataPoints->valueSW_KongTiao = gizStandardDecompressionValue(SW_KongTiao_BYTEOFFSET, SW_KongTiao_BITOFFSET, SW_KongTiao_LEN, (uint8_t *)&issuedData->attrVals.wBitBuf, sizeof(issuedData->attrVals.wBitBuf));
	}

	if (0x01 == issuedData->attrFlags.flagSW_ZhiBan)
	{
		info->event[info->num] = EVENT_SW_ZhiBan;
		info->num++;
		dataPoints->valueSW_ZhiBan = gizStandardDecompressionValue(SW_ZhiBan_BYTEOFFSET, SW_ZhiBan_BITOFFSET, SW_ZhiBan_LEN, (uint8_t *)&issuedData->attrVals.wBitBuf, sizeof(issuedData->attrVals.wBitBuf));
	}

	if (0x01 == issuedData->attrFlags.flagSW_FuYa)
	{
		info->event[info->num] = EVENT_SW_FuYa;
		info->num++;
		dataPoints->valueSW_FuYa = gizStandardDecompressionValue(SW_FuYa_BYTEOFFSET, SW_FuYa_BITOFFSET, SW_FuYa_LEN, (uint8_t *)&issuedData->attrVals.wBitBuf, sizeof(issuedData->attrVals.wBitBuf));
	}


	if (0x01 == issuedData->attrFlags.flagWenDuSet)
	{
		info->event[info->num] = EVENT_WenDuSet;
		info->num++;
		dataPoints->valueWenDuSet = gizX2Y(WenDuSet_RATIO, WenDuSet_ADDITION, exchangeBytes(issuedData->attrVals.valueWenDuSet));
	}

	if (0x01 == issuedData->attrFlags.flagShiDuSet)
	{
		info->event[info->num] = EVENT_ShiDuSet;
		info->num++;
		dataPoints->valueShiDuSet = gizX2Y(ShiDuSet_RATIO, ShiDuSet_ADDITION, exchangeBytes(issuedData->attrVals.valueShiDuSet));
	}

	if (0x01 == issuedData->attrFlags.flagYaChaSet)
	{
		info->event[info->num] = EVENT_YaChaSet;
		info->num++;
		dataPoints->valueYaChaSet = gizX2Y(YaChaSet_RATIO, YaChaSet_ADDITION, exchangeBytes(issuedData->attrVals.valueYaChaSet));
	}

	return 0;
}

/**
* @brief contrasts the current data with the last data		将当前数据与最后的数据进行对比
*
* @param [in] cur: current data point data					当前数据点数据
* @param [in] last: last data point data					最后一个数据点的数据
*
* @return: 0, no change in data; 1, data changes
*/
static int8_t ICACHE_FLASH_ATTR gizCheckReport(dataPoint_t *cur, dataPoint_t *last)
{
	int8_t ret = 0;
	static uint32_t lastReportTime = 0;

	if ((NULL == cur) || (NULL == last))
	{
		GIZWITS_LOG("gizCheckReport Error , Illegal Param\n");
		return -1;
	}
	if (last->valueSW_KongTiao != cur->valueSW_KongTiao)
	{
		GIZWITS_LOG("valueSW_KongTiao Changed\n");
		ret = 1;
	}
	if (last->valueSW_ZhiBan != cur->valueSW_ZhiBan)
	{
		GIZWITS_LOG("valueSW_ZhiBan Changed\n");
		ret = 1;
	}
	if (last->valueSW_FuYa != cur->valueSW_FuYa)
	{
		GIZWITS_LOG("valueSW_FuYa Changed\n");
		ret = 1;
	}
	if (last->valueWenDuSet != cur->valueWenDuSet)
	{
		GIZWITS_LOG("valueWenDuSet Changed\n");
		ret = 1;
	}
	if (last->valueShiDuSet != cur->valueShiDuSet)
	{
		GIZWITS_LOG("valueShiDuSet Changed\n");
		ret = 1;
	}
	if (last->valueYaChaSet != cur->valueYaChaSet)
	{
		GIZWITS_LOG("valueYaChaSet Changed\n");
		ret = 1;
	}
	if (last->valueZS_JiZuYunXing != cur->valueZS_JiZuYunXing)
	{
		GIZWITS_LOG("valueZS_JiZuYunXing Changed\n");
		ret = 1;
	}
	if (last->valueZS_ZhiBanYunXing != cur->valueZS_ZhiBanYunXing)
	{
		GIZWITS_LOG("valueZS_ZhiBanYunXing Changed\n");
		ret = 1;
	}
	if (last->valueZS_FuYaYunXing != cur->valueZS_FuYaYunXing)
	{
		GIZWITS_LOG("valueZS_FuYaYunXing Changed\n");
		ret = 1;
	}
	if (last->valueZS_JiZuGuZhang != cur->valueZS_JiZuGuZhang)
	{
		GIZWITS_LOG("valueZS_JiZuGuZhang Changed\n");
		ret = 1;
	}
	if (last->valueZS_GaoXiaoZuSe != cur->valueZS_GaoXiaoZuSe)
	{
		GIZWITS_LOG("valueZS_GaoXiaoZuSe Changed\n");
		ret = 1;
	}

	if (last->valueWenDuZhi != cur->valueWenDuZhi)
	{
		if (gizGetTimerCount() - lastReportTime >= REPORT_TIME_MAX)
		{
			GIZWITS_LOG("valueWenDuZhi Changed\n");
			lastReportTime = gizGetTimerCount();
			ret = 1;
		}
	}
	if (last->valueShiDuZhi != cur->valueShiDuZhi)
	{
		if (gizGetTimerCount() - lastReportTime >= REPORT_TIME_MAX)
		{
			GIZWITS_LOG("valueShiDuZhi Changed\n");
			lastReportTime = gizGetTimerCount();
			ret = 1;
		}
	}
	if (last->valueYaChaZhi != cur->valueYaChaZhi)
	{
		if (gizGetTimerCount() - lastReportTime >= REPORT_TIME_MAX)
		{
			GIZWITS_LOG("valueYaChaZhi Changed\n");
			lastReportTime = gizGetTimerCount();
			ret = 1;
		}
	}
	if (last->valueLengShuiFa != cur->valueLengShuiFa)
	{
		if (gizGetTimerCount() - lastReportTime >= REPORT_TIME_MAX)
		{
			GIZWITS_LOG("valueLengShuiFa Changed\n");
			lastReportTime = gizGetTimerCount();
			ret = 1;
		}
	}
	if (last->valueReShuiFa != cur->valueReShuiFa)
	{
		if (gizGetTimerCount() - lastReportTime >= REPORT_TIME_MAX)
		{
			GIZWITS_LOG("valueReShuiFa Changed\n");
			lastReportTime = gizGetTimerCount();
			ret = 1;
		}
	}
	if (last->valueJiaShuiQi != cur->valueJiaShuiQi)
	{
		if (gizGetTimerCount() - lastReportTime >= REPORT_TIME_MAX)
		{
			GIZWITS_LOG("valueJiaShuiQi Changed\n");
			lastReportTime = gizGetTimerCount();
			ret = 1;
		}
	}

	return ret;
}

/**
* @brief User data point data is converted to wit the cloud to report data point data
			用户数据点数据转换为机智云来报告数据点数据
*
* @param [in] dataPoints: user data point data address				用户数据点数据地址
* @param [out] devStatusPtr: wit the cloud data point data address	机智云数据点地址
*
* @return 0, the correct return; -1, the error returned
*/
static int8_t ICACHE_FLASH_ATTR gizDataPoints2ReportData(dataPoint_t *dataPoints, devStatus_t *devStatusPtr)
{
	if ((NULL == dataPoints) || (NULL == devStatusPtr))
	{
		GIZWITS_LOG("gizDataPoints2ReportData Error , Illegal Param\n");
		return -1;
	}

	gizMemset((uint8_t *)devStatusPtr->wBitBuf, 0, sizeof(devStatusPtr->wBitBuf));
	gizMemset((uint8_t *)devStatusPtr->rBitBuf, 0, sizeof(devStatusPtr->rBitBuf));

	gizStandardCompressValue(SW_KongTiao_BYTEOFFSET, SW_KongTiao_BITOFFSET, SW_KongTiao_LEN, (uint8_t *)devStatusPtr, dataPoints->valueSW_KongTiao);
	gizStandardCompressValue(SW_ZhiBan_BYTEOFFSET, SW_ZhiBan_BITOFFSET, SW_ZhiBan_LEN, (uint8_t *)devStatusPtr, dataPoints->valueSW_ZhiBan);
	gizStandardCompressValue(SW_FuYa_BYTEOFFSET, SW_FuYa_BITOFFSET, SW_FuYa_LEN, (uint8_t *)devStatusPtr, dataPoints->valueSW_FuYa);
	gizStandardCompressValue(ZS_JiZuYunXing_BYTEOFFSET, ZS_JiZuYunXing_BITOFFSET, ZS_JiZuYunXing_LEN, (uint8_t *)devStatusPtr, dataPoints->valueZS_JiZuYunXing);
	gizStandardCompressValue(ZS_ZhiBanYunXing_BYTEOFFSET, ZS_ZhiBanYunXing_BITOFFSET, ZS_ZhiBanYunXing_LEN, (uint8_t *)devStatusPtr, dataPoints->valueZS_ZhiBanYunXing);
	gizStandardCompressValue(ZS_FuYaYunXing_BYTEOFFSET, ZS_FuYaYunXing_BITOFFSET, ZS_FuYaYunXing_LEN, (uint8_t *)devStatusPtr, dataPoints->valueZS_FuYaYunXing);
	gizStandardCompressValue(ZS_JiZuGuZhang_BYTEOFFSET, ZS_JiZuGuZhang_BITOFFSET, ZS_JiZuGuZhang_LEN, (uint8_t *)devStatusPtr, dataPoints->valueZS_JiZuGuZhang);
	gizStandardCompressValue(ZS_GaoXiaoZuSe_BYTEOFFSET, ZS_GaoXiaoZuSe_BITOFFSET, ZS_GaoXiaoZuSe_LEN, (uint8_t *)devStatusPtr, dataPoints->valueZS_GaoXiaoZuSe);
	gizByteOrderExchange((uint8_t *)devStatusPtr->wBitBuf, sizeof(devStatusPtr->wBitBuf));
	gizByteOrderExchange((uint8_t *)devStatusPtr->rBitBuf, sizeof(devStatusPtr->rBitBuf));


	devStatusPtr->valueWenDuSet = exchangeBytes(gizY2X(WenDuSet_RATIO, WenDuSet_ADDITION, dataPoints->valueWenDuSet));
	devStatusPtr->valueShiDuSet = exchangeBytes(gizY2X(ShiDuSet_RATIO, ShiDuSet_ADDITION, dataPoints->valueShiDuSet));
	devStatusPtr->valueYaChaSet = exchangeBytes(gizY2X(YaChaSet_RATIO, YaChaSet_ADDITION, dataPoints->valueYaChaSet));
	devStatusPtr->valueWenDuZhi = exchangeBytes(gizY2X(WenDuZhi_RATIO, WenDuZhi_ADDITION, dataPoints->valueWenDuZhi));
	devStatusPtr->valueShiDuZhi = exchangeBytes(gizY2X(ShiDuZhi_RATIO, ShiDuZhi_ADDITION, dataPoints->valueShiDuZhi));
	devStatusPtr->valueYaChaZhi = exchangeBytes(gizY2X(YaChaZhi_RATIO, YaChaZhi_ADDITION, dataPoints->valueYaChaZhi));
	devStatusPtr->valueLengShuiFa = exchangeBytes(gizY2X(LengShuiFa_RATIO, LengShuiFa_ADDITION, dataPoints->valueLengShuiFa));
	devStatusPtr->valueReShuiFa = exchangeBytes(gizY2X(ReShuiFa_RATIO, ReShuiFa_ADDITION, dataPoints->valueReShuiFa));
	devStatusPtr->valueJiaShuiQi = exchangeBytes(gizY2X(JiaShuiQi_RATIO, JiaShuiQi_ADDITION, dataPoints->valueJiaShuiQi));



	return 0;
}


/**
* @brief This function is called by the Gagent module to receive the relevant protocol data from the cloud or APP
		这个函数由Gagent模块调用，以接收来自云或应用程序的相关协议数据
* @param [in] inData The protocol data entered				输入的协议数据	
* @param [in] inLen Enter the length of the data			输入数据的长度
* @param [out] outData The output of the protocol data		协议数据的输出
* @param [out] outLen The length of the output data			输出数据的长度
* @return 0, the implementation of success, non-0, failed	返回0，成功的实现，非0，失败
*/
static int8_t gizProtocolIssuedProcess(char *did, uint8_t *inData, uint32_t inLen, uint8_t *outData, uint32_t *outLen)
{
	uint8_t issuedAction = inData[0];

	if ((NULL == inData) || (NULL == outData) || (NULL == outLen))
	{
		GIZWITS_LOG("gizProtocolIssuedProcess Error , Illegal Param\n");
		return -1;
	}

	if (NULL == did)
	{
		memset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0, sizeof(eventInfo_t));
		switch (issuedAction)
		{
		case ACTION_CONTROL_DEVICE:
			gizDataPoint2Event((gizwitsIssued_t *)&inData[1], &gizwitsProtocol.issuedProcessEvent, &gizwitsProtocol.gizCurrentDataPoint);
			gizwitsProtocol.issuedFlag = ACTION_CONTROL_TYPE;
			outData = NULL;
			*outLen = 0;
			break;

		case ACTION_READ_DEV_STATUS:
			if (0 == gizDataPoints2ReportData(&gizwitsProtocol.gizLastDataPoint, &gizwitsProtocol.reportData.devStatus))
			{
				memcpy(outData + 1, (uint8_t *)&gizwitsProtocol.reportData.devStatus, sizeof(gizwitsReport_t));
				outData[0] = ACTION_READ_DEV_STATUS_ACK;
				*outLen = sizeof(gizwitsReport_t) + 1;
			}
			else
			{
				return -1;
			}
			break;
		case ACTION_W2D_TRANSPARENT_DATA:
			memcpy(gizwitsProtocol.transparentBuff, &inData[1], inLen - 1);
			gizwitsProtocol.transparentLen = inLen - 1;

			gizwitsProtocol.issuedProcessEvent.event[gizwitsProtocol.issuedProcessEvent.num] = TRANSPARENT_DATA;
			gizwitsProtocol.issuedProcessEvent.num++;
			gizwitsProtocol.issuedFlag = ACTION_W2D_TRANSPARENT_TYPE;
			outData = NULL;
			*outLen = 0;
			break;

		default:
			break;
		}
	}

	return 0;
}
/**
* @brief The protocol sends data back , P0 ACK		协议发送数据返回，P0 ACK
*
* @param [in] head                  : Protocol head pointer		协议头指针
* @param [in] data                  : Payload data				有效载荷数据
* @param [in] len                   : Payload data length		有效载荷数据长度
* @param [in] proFlag               : DID flag ,1 for Virtual sub device did ,0 for single product or gateway 是否有标志，1为虚拟子设备，0为单一产品或网关
*
* @return : 0,Ack success;
*           -1，Input Param Illegal
*           -2，Serial send faild
*/
static int32_t gizProtocolIssuedDataAck(protocolHead_t *head, uint8_t *gizdata, uint32_t len, uint8_t proFlag)//协议数据发布应答
{
	int32_t ret = 0;
	uint8_t tx_buf[RB_MAX_LEN];
	uint32_t offset = 0;
	uint8_t sDidLen = 0;
	uint16_t data_len = 0;
	uint8_t *pTxBuf = tx_buf;
	if (NULL == gizdata)
	{
		GIZWITS_LOG("[ERR]  data Is Null \n");
		return -1;
	}


	if (0x1 == proFlag)
	{
		sDidLen = *((uint8_t *)head + sizeof(protocolHead_t));
		data_len = 5 + 1 + sDidLen + len;
	}
	else
	{
		data_len = 5 + len;
	}
	GIZWITS_LOG("len = %d , sDidLen = %d ,data_len = %d\n", len, sDidLen, data_len);
	*pTxBuf++ = 0xFF;
	*pTxBuf++ = 0xFF;
	*pTxBuf++ = (uint8_t)(data_len >> 8);
	*pTxBuf++ = (uint8_t)(data_len);
	*pTxBuf++ = head->cmd + 1;
	*pTxBuf++ = head->sn;
	*pTxBuf++ = 0x00;
	*pTxBuf++ = proFlag;
	offset = 8;
	if (0x1 == proFlag)
	{
		*pTxBuf++ = sDidLen;
		offset += 1;
		memcpy(&tx_buf[offset], (uint8_t *)head + sizeof(protocolHead_t) + 1, sDidLen);
		offset += sDidLen;
		pTxBuf += sDidLen;

	}
	if (0 != len)
	{
		memcpy(&tx_buf[offset], gizdata, len);
	}
	tx_buf[data_len + 4 - 1] = gizProtocolSum(tx_buf, (data_len + 4));

	ret = uartWrite(tx_buf, data_len + 4);
	if (ret < 0)
	{
		GIZWITS_LOG("uart write error %d \n", ret);
		return -2;
	}

	return 0;
}

/**
* @brief Report data interface		上报数据接口
*
* @param [in] action            : PO action				PO行动
* @param [in] data              : Payload data			有效载荷数据
* @param [in] len               : Payload data length	有效载荷数据长度
*
* @return : 0,Ack success;
*           -1，Input Param Illegal	输入参数不合法
*           -2，Serial send faild	串行发送失败
*/
static int32_t gizReportData(uint8_t action, uint8_t *gizdata, uint32_t len)
{
	int32_t ret = 0;
	protocolReport_t protocolReport;

	if (NULL == gizdata)
	{
		GIZWITS_LOG("gizReportData Error , Illegal Param\n");
		return -1;
	}
	gizProtocolHeadInit((protocolHead_t *)&protocolReport);
	protocolReport.head.cmd = CMD_REPORT_P0;
	protocolReport.head.sn = gizwitsProtocol.sn++;
	protocolReport.action = action;
	protocolReport.head.len = exchangeBytes(sizeof(protocolReport_t) - 4);
	memcpy((gizwitsReport_t *)&protocolReport.reportData, (gizwitsReport_t *)gizdata, len);
	protocolReport.sum = gizProtocolSum((uint8_t *)&protocolReport, sizeof(protocolReport_t));

	ret = uartWrite((uint8_t *)&protocolReport, sizeof(protocolReport_t));
	if (ret < 0)
	{
		GIZWITS_LOG("ERR: uart write error %d \n", ret);
		return -2;
	}

	gizProtocolWaitAck((uint8_t *)&protocolReport, sizeof(protocolReport_t));

	return ret;
}/**
 * @brief Datapoints reporting mechanism		数据点报告机制
 *
 * 1. Changes are reported immediately			立即更改报告

 * 2. Data timing report , 600000 Millisecond	数据定时报告，600000毫秒
 *
 *@param [in] currentData       : Current datapoints value	当前数据点值
 * @return : NULL
 */
static void gizDevReportPolicy(dataPoint_t *currentData)
{
	static uint32_t lastRepTime = 0;
	uint32_t timeNow = gizGetTimerCount();

	if ((1 == gizCheckReport(currentData, (dataPoint_t *)&gizwitsProtocol.gizLastDataPoint)))
	{
		GIZWITS_LOG("changed, report data\n");
		if (0 == gizDataPoints2ReportData(currentData, &gizwitsProtocol.reportData.devStatus))
		{
			gizReportData(ACTION_REPORT_DEV_STATUS, (uint8_t *)&gizwitsProtocol.reportData.devStatus, sizeof(devStatus_t));
		}
		memcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)currentData, sizeof(dataPoint_t));
	}

	if ((0 == (timeNow % (600000))) && (lastRepTime != timeNow))
	{
		GIZWITS_LOG("Info: 600S report data\n");
		if (0 == gizDataPoints2ReportData(currentData, &gizwitsProtocol.reportData.devStatus))
		{
			gizReportData(ACTION_REPORT_DEV_STATUS, (uint8_t *)&gizwitsProtocol.reportData.devStatus, sizeof(devStatus_t));
		}
		memcpy((uint8_t *)&gizwitsProtocol.gizLastDataPoint, (uint8_t *)currentData, sizeof(dataPoint_t));

		lastRepTime = timeNow;
	}
}

/**
* @brief Get a packet of data from the ring buffer 从环形缓冲区获取数据包
*
* @param [in]  rb                  : Input data address		输入数据的地址
* @param [out] data                : Output data address	输出数据的地址
* @param [out] len                 : Output data length		输出数据长度
*
* @return : 0,Return correct ;-1，Return failure;-2，Data check failure
*/
static int8_t gizProtocolGetOnePacket(rb_t *rb, uint8_t *gizdata, uint16_t *len)
{
	int32_t ret = 0;
	uint8_t sum = 0;
	int32_t i = 0;
	uint8_t tmpData;
	uint8_t tmpLen = 0;
	uint16_t tmpCount = 0;
	static uint8_t protocolFlag = 0;
	static uint16_t protocolCount = 0;
	static uint8_t lastData = 0;
	static uint8_t debugCount = 0;
	uint8_t *protocolBuff = gizdata;
	protocolHead_t *head = NULL;

	if ((NULL == rb) || (NULL == gizdata) || (NULL == len))
	{
		GIZWITS_LOG("gizProtocolGetOnePacket Error , Illegal Param\n");
		return -1;
	}

	tmpLen = rbCanRead(rb);
	if (0 == tmpLen)
	{
		return -1;
	}

	for (i = 0; i<tmpLen; i++)
	{
		ret = rbRead(rb, &tmpData, 1);
		if (0 != ret)
		{
			if ((0xFF == lastData) && (0xFF == tmpData))
			{
				if (0 == protocolFlag)
				{
					protocolBuff[0] = 0xFF;
					protocolBuff[1] = 0xFF;
					protocolCount = 2;
					protocolFlag = 1;
				}
				else
				{
					if ((protocolCount > 4) && (protocolCount != tmpCount))
					{
						protocolBuff[0] = 0xFF;
						protocolBuff[1] = 0xFF;
						protocolCount = 2;
					}
				}
			}
			else if ((0xFF == lastData) && (0x55 == tmpData))
			{
			}
			else
			{
				if (1 == protocolFlag)
				{
					protocolBuff[protocolCount] = tmpData;
					protocolCount++;

					if (protocolCount > 4)
					{
						head = (protocolHead_t *)protocolBuff;
						tmpCount = exchangeBytes(head->len) + 4;
						if (protocolCount == tmpCount)
						{
							break;
						}
					}
				}
			}

			lastData = tmpData;
			debugCount++;
		}
	}

	if ((protocolCount > 4) && (protocolCount == tmpCount))
	{
		sum = gizProtocolSum(protocolBuff, protocolCount);

		if (protocolBuff[protocolCount - 1] == sum)
		{
			memcpy(gizdata, protocolBuff, tmpCount);
			*len = tmpCount;
			protocolFlag = 0;

			protocolCount = 0;
			debugCount = 0;
			lastData = 0;

			return 0;
		}
		else
		{
			return -2;
		}
	}

	return 1;
}



/**
* @brief Protocol data resend		协议数据重发

* The protocol data resend when check timeout and meet the resend limiting
	当检查超时并满足重新发送限制时，协议数据重新发送

* @param none
*
* @return none
*/
static void gizProtocolResendData(void)
{
	int32_t ret = 0;

	if (0 == gizwitsProtocol.waitAck.flag)
	{
		return;
	}

	GIZWITS_LOG("Warning: timeout, resend data \n");

	ret = uartWrite(gizwitsProtocol.waitAck.buf, gizwitsProtocol.waitAck.dataLen);
	if (ret != gizwitsProtocol.waitAck.dataLen)
	{
		GIZWITS_LOG("ERR: resend data error\n");
	}

	gizwitsProtocol.waitAck.sendTime = gizGetTimerCount();
}

/**
* @brief Clear the ACK protocol message	清除ACK协议消息
*
* @param [in] head : Protocol header address	协议头地址
*
* @return 0， success; other， failure
*/
static int8_t gizProtocolWaitAckCheck(protocolHead_t *head)
{
	protocolHead_t *waitAckHead = (protocolHead_t *)gizwitsProtocol.waitAck.buf;

	if (NULL == head)
	{
		GIZWITS_LOG("ERR: data is empty \n");
		return -1;
	}

	if (waitAckHead->cmd + 1 == head->cmd)
	{
		memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
	}

	return 0;
}

/**
* @brief Send general protocol message data		发送通用协议消息数据
*
* @param [in] head              : Protocol header address	协议头地址
*
* @return : Return effective data length;-1，return failure	返回有效数据长度;1,返回失败
*/
static int32_t gizProtocolCommonAck(protocolHead_t *head)
{
	int32_t ret = 0;
	protocolCommon_t ack;

	if (NULL == head)
	{
		GIZWITS_LOG("ERR: gizProtocolCommonAck data is empty \n");
		return -1;
	}
	memcpy((uint8_t *)&ack, (uint8_t *)head, sizeof(protocolHead_t));
	ack.head.cmd = ack.head.cmd + 1;
	ack.head.len = exchangeBytes(sizeof(protocolCommon_t) - 4);
	ack.sum = gizProtocolSum((uint8_t *)&ack, sizeof(protocolCommon_t));

	ret = uartWrite((uint8_t *)&ack, sizeof(protocolCommon_t));
	if (ret < 0)
	{
		GIZWITS_LOG("ERR: uart write error %d \n", ret);
	}

	return ret;
}

/**
* @brief ACK processing function	ACK处理函数

* Time-out 200ms no ACK resend，resend two times at most 超时200 ms没有ACK重发,重发两次

* @param none
*
* @return none
*/
static void gizProtocolAckHandle(void)
{
	if (1 == gizwitsProtocol.waitAck.flag)
	{
		if (SEND_MAX_NUM > gizwitsProtocol.waitAck.num)
		{
			// Time-out no ACK resend
			if (SEND_MAX_TIME < (gizGetTimerCount() - gizwitsProtocol.waitAck.sendTime))
			{
				GIZWITS_LOG("Warning:gizProtocolResendData %d %d %d\n", gizGetTimerCount(), gizwitsProtocol.waitAck.sendTime, gizwitsProtocol.waitAck.num);
				gizProtocolResendData();
				gizwitsProtocol.waitAck.num++;
			}
		}
		else
		{
			//memset((uint8_t *)&gizwitsProtocol.waitAck, 0, sizeof(protocolWaitAck_t));
			mcuRestart();
		}
	}
}

/**
* @brief Protocol 4.1 WiFi module requests device information	WiFi模块请求设备信息
*
* @param[in] head : Protocol header address		协议头地址
*
* @return Return effective data length;-1，return failure	返回有效数据长度;1,返回失败
*/
static int32_t gizProtocolGetDeviceInfo(protocolHead_t * head)
{
	int32_t ret = 0;
	protocolDeviceInfo_t deviceInfo;

	if (NULL == head)
	{
		GIZWITS_LOG("gizProtocolGetDeviceInfo Error , Illegal Param\n");
		return -1;
	}

	gizProtocolHeadInit((protocolHead_t *)&deviceInfo);
	deviceInfo.head.cmd = ACK_GET_DEVICE_INFO;
	deviceInfo.head.sn = head->sn;
	memcpy((uint8_t *)deviceInfo.protocolVer, protocol_VERSION, 8);
	memcpy((uint8_t *)deviceInfo.p0Ver, P0_VERSION, 8);
	memcpy((uint8_t *)deviceInfo.softVer, SOFTWARE_VERSION, 8);
	memcpy((uint8_t *)deviceInfo.hardVer, HARDWARE_VERSION, 8);
	memcpy((uint8_t *)deviceInfo.productKey, PRODUCT_KEY, strlen(PRODUCT_KEY));
	memcpy((uint8_t *)deviceInfo.productSecret, PRODUCT_SECRET, strlen(PRODUCT_SECRET));
	memset((uint8_t *)deviceInfo.devAttr, 0, 8);
	deviceInfo.devAttr[7] |= DEV_IS_GATEWAY << 0;
	deviceInfo.devAttr[7] |= (0x01 << 1);
	deviceInfo.ninableTime = exchangeBytes(NINABLETIME);
	deviceInfo.head.len = exchangeBytes(sizeof(protocolDeviceInfo_t) - 4);
	deviceInfo.sum = gizProtocolSum((uint8_t *)&deviceInfo, sizeof(protocolDeviceInfo_t));

	ret = uartWrite((uint8_t *)&deviceInfo, sizeof(protocolDeviceInfo_t));
	if (ret < 0)
	{
		GIZWITS_LOG("ERR: uart write error %d \n", ret);
	}

	return ret;
}

/**
* @brief Protocol 4.7 Handling of illegal message notification	处理非法消息通知

* @param[in] head  : Protocol header address		协议头地址
* @param[in] errno : Illegal message notification type	非法消息通知类型
* @return 0， success; other， failure
*/
static int32_t gizProtocolErrorCmd(protocolHead_t *head, errorPacketsType_t errno)
{
	int32_t ret = 0;
	protocolErrorType_t errorType;

	if (NULL == head)
	{
		GIZWITS_LOG("gizProtocolErrorCmd Error , Illegal Param\n");
		return -1;
	}
	gizProtocolHeadInit((protocolHead_t *)&errorType);
	errorType.head.cmd = ACK_ERROR_PACKAGE;
	errorType.head.sn = head->sn;

	errorType.head.len = exchangeBytes(sizeof(protocolErrorType_t) - 4);
	errorType.error = errno;
	errorType.sum = gizProtocolSum((uint8_t *)&errorType, sizeof(protocolErrorType_t));

	ret = uartWrite((uint8_t *)&errorType, sizeof(protocolErrorType_t));
	if (ret < 0)
	{
		GIZWITS_LOG("ERR: uart write error %d \n", ret);
	}

	return ret;
}

/**
* @brief Protocol 4.13 Get and process network time 获取并处理网络时间
*
* @param [in] head : Protocol header address 协议头地址
*
* @return 0， success; other， failure
*/
static int8_t gizProtocolNTP(protocolHead_t *head)
{
	protocolUTT_t *UTTInfo = (protocolUTT_t *)head;

	if (NULL == head)
	{
		GIZWITS_LOG("ERR: NTP is empty \n");
		return -1;
	}

	memcpy((uint8_t *)&gizwitsProtocol.TimeNTP, (uint8_t *)UTTInfo->time, (7 + 4));
	gizwitsProtocol.TimeNTP.year = exchangeBytes(gizwitsProtocol.TimeNTP.year);
	gizwitsProtocol.TimeNTP.ntp = exchangeWord(gizwitsProtocol.TimeNTP.ntp);

	gizwitsProtocol.NTPEvent.event[gizwitsProtocol.NTPEvent.num] = WIFI_NTP;
	gizwitsProtocol.NTPEvent.num++;

	gizwitsProtocol.issuedFlag = GET_NTP_TYPE;


	return 0;
}

/**
* @brief Protocol 4.4 Device MCU restarts function 设备MCU重启功能

* @param none
* @return none
*/
static void gizProtocolReboot(void)
{
	uint32_t timeDelay = gizGetTimerCount();

	/*Wait 600ms*/
	while ((gizGetTimerCount() - timeDelay) <= 600);
	mcuRestart();
}

/**
* @brief Protocol 4.5 :The WiFi module informs the device MCU of working status about the WiFi module
						WiFi模块告知设备MCU关于WiFi模块的工作状态

* @param[in] status WiFi module working status   状态WiFi模块工作状态
* @return none
*/
static int8_t gizProtocolModuleStatus(protocolWifiStatus_t *status)
{
	static wifiStatus_t lastStatus;

	if (NULL == status)
	{
		GIZWITS_LOG("gizProtocolModuleStatus Error , Illegal Param\n");
		return -1;
	}

	status->ststus.value = exchangeBytes(status->ststus.value);

	//OnBoarding mode status
	if (lastStatus.types.onboarding != status->ststus.types.onboarding)
	{
		if (1 == status->ststus.types.onboarding)
		{
			if (1 == status->ststus.types.softap)
			{
				gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_SOFTAP;
				gizwitsProtocol.wifiStatusEvent.num++;
				GIZWITS_LOG("OnBoarding: SoftAP or Web mode\n");
			}

			if (1 == status->ststus.types.station)
			{
				gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_AIRLINK;
				gizwitsProtocol.wifiStatusEvent.num++;
				GIZWITS_LOG("OnBoarding: AirLink mode\n");
			}
		}
		else
		{
			if (1 == status->ststus.types.softap)
			{
				gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_SOFTAP;
				gizwitsProtocol.wifiStatusEvent.num++;
				GIZWITS_LOG("OnBoarding: SoftAP or Web mode\n");
			}

			if (1 == status->ststus.types.station)
			{
				gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_STATION;
				gizwitsProtocol.wifiStatusEvent.num++;
				GIZWITS_LOG("OnBoarding: Station mode\n");
			}
		}
	}

	//binding mode status
	if (lastStatus.types.binding != status->ststus.types.binding)
	{
		lastStatus.types.binding = status->ststus.types.binding;
		if (1 == status->ststus.types.binding)
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_OPEN_BINDING;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: in binding mode\n");
		}
		else
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CLOSE_BINDING;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: out binding mode\n");
		}
	}

	//router status
	if (lastStatus.types.con_route != status->ststus.types.con_route)
	{
		lastStatus.types.con_route = status->ststus.types.con_route;
		if (1 == status->ststus.types.con_route)
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_ROUTER;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: connected router\n");
		}
		else
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_ROUTER;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: disconnected router\n");
		}
	}

	//M2M server status
	if (lastStatus.types.con_m2m != status->ststus.types.con_m2m)
	{
		lastStatus.types.con_m2m = status->ststus.types.con_m2m;
		if (1 == status->ststus.types.con_m2m)
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_M2M;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: connected m2m\n");
		}
		else
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_M2M;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: disconnected m2m\n");
		}
	}

	//APP status
	if (lastStatus.types.app != status->ststus.types.app)
	{
		lastStatus.types.app = status->ststus.types.app;
		if (1 == status->ststus.types.app)
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CON_APP;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: app connect\n");
		}
		else
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_DISCON_APP;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: no app connect\n");
		}
	}

	//test mode status
	if (lastStatus.types.test != status->ststus.types.test)
	{
		lastStatus.types.test = status->ststus.types.test;
		if (1 == status->ststus.types.test)
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_OPEN_TESTMODE;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: in test mode\n");
		}
		else
		{
			gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_CLOSE_TESTMODE;
			gizwitsProtocol.wifiStatusEvent.num++;
			GIZWITS_LOG("WiFi status: out test mode\n");
		}
	}

	gizwitsProtocol.wifiStatusEvent.event[gizwitsProtocol.wifiStatusEvent.num] = WIFI_RSSI;
	gizwitsProtocol.wifiStatusEvent.num++;
	gizwitsProtocol.wifiStatusData.rssi = status->ststus.types.rssi;
	GIZWITS_LOG("RSSI is %d \n", gizwitsProtocol.wifiStatusData.rssi);

	gizwitsProtocol.issuedFlag = WIFI_STATUS_TYPE;

	return 0;
}


/**@name Gizwits User API interface 机智云用户API接口
* @{
*/

/**
* @brief gizwits Protocol initialization interface gizwits	协议初始化接口

* Protocol-related timer, serial port initialization  与协议相关的定时器，串口初始化

* Datapoint initialization		数据初始化

* @param none
* @return none
*/
void gizwitsInit(void)
{
	pRb.rbCapacity = RB_MAX_LEN;
	pRb.rbBuff = rbBuf;
	if (0 == rbCreate(&pRb))
	{
		GIZWITS_LOG("rbCreate Success \n");
	}
	else
	{
		GIZWITS_LOG("rbCreate Faild \n");
	}

	memset((uint8_t *)&gizwitsProtocol, 0, sizeof(gizwitsProtocol_t));
}

/**
* @brief WiFi configure interface  WIFI配置接口

* Set the WiFi module into the corresponding configuration mode or reset the module
	将WiFi模块设置为相应的配置模式或重置模块

* @param[in] mode ：	0x0， reset the module ;
					0x01， SoftAp mode ;
					0x02，AirLink mode ;
					0x03， Production test mode; 
					0x04:allow users to bind devices 允许用户绑定设备

* @return Error command code
*/
int32_t gizwitsSetMode(uint8_t mode)
{
	int32_t ret = 0;
	protocolCfgMode_t cfgMode;
	protocolCommon_t setDefault;

	switch (mode)
	{
	case WIFI_RESET_MODE:
		gizProtocolHeadInit((protocolHead_t *)&setDefault);
		setDefault.head.cmd = CMD_SET_DEFAULT;
		setDefault.head.sn = gizwitsProtocol.sn++;
		setDefault.head.len = exchangeBytes(sizeof(protocolCommon_t) - 4);
		setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		if (ret < 0)
		{
			GIZWITS_LOG("ERR: uart write error %d \n", ret);
		}

		gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		break;
	case WIFI_SOFTAP_MODE:
		gizProtocolHeadInit((protocolHead_t *)&cfgMode);
		cfgMode.head.cmd = CMD_WIFI_CONFIG;
		cfgMode.head.sn = gizwitsProtocol.sn++;
		cfgMode.cfgMode = mode;
		cfgMode.head.len = exchangeBytes(sizeof(protocolCfgMode_t) - 4);
		cfgMode.sum = gizProtocolSum((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
		ret = uartWrite((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
		if (ret < 0)
		{
			GIZWITS_LOG("ERR: uart write error %d \n", ret);
		}
		gizProtocolWaitAck((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
		break;
	case WIFI_AIRLINK_MODE:
		gizProtocolHeadInit((protocolHead_t *)&cfgMode);
		cfgMode.head.cmd = CMD_WIFI_CONFIG;
		cfgMode.head.sn = gizwitsProtocol.sn++;
		cfgMode.cfgMode = mode;
		cfgMode.head.len = exchangeBytes(sizeof(protocolCfgMode_t) - 4);
		cfgMode.sum = gizProtocolSum((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
		ret = uartWrite((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
		if (ret < 0)
		{
			GIZWITS_LOG("ERR: uart write error %d \n", ret);
		}
		gizProtocolWaitAck((uint8_t *)&cfgMode, sizeof(protocolCfgMode_t));
		break;
	case WIFI_PRODUCTION_TEST:
		gizProtocolHeadInit((protocolHead_t *)&setDefault);
		setDefault.head.cmd = CMD_PRODUCTION_TEST;
		setDefault.head.sn = gizwitsProtocol.sn++;
		setDefault.head.len = exchangeBytes(sizeof(protocolCommon_t) - 4);
		setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		if (ret < 0)
		{
			GIZWITS_LOG("ERR: uart write error %d \n", ret);
		}

		gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		break;
	case WIFI_NINABLE_MODE:
		gizProtocolHeadInit((protocolHead_t *)&setDefault);
		setDefault.head.cmd = CMD_NINABLE_MODE;
		setDefault.head.sn = gizwitsProtocol.sn++;
		setDefault.head.len = exchangeBytes(sizeof(protocolCommon_t) - 4);
		setDefault.sum = gizProtocolSum((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		ret = uartWrite((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		if (ret < 0)
		{
			GIZWITS_LOG("ERR: uart write error %d \n", ret);
		}

		gizProtocolWaitAck((uint8_t *)&setDefault, sizeof(protocolCommon_t));
		break;
	default:
		GIZWITS_LOG("ERR: CfgMode error!\n");
		break;
	}

	return ret;
}

/**
* @brief Get the the network time 获取网络时间

* Protocol 4.13:"Device MCU send" of "the MCU requests access to the network time" “单片机发送”的“MCU请求访问网络时间”

* @param[in] none
* @return none
*/
void gizwitsGetNTP(void)
{
	int32_t ret = 0;
	protocolCommon_t getNTP;

	gizProtocolHeadInit((protocolHead_t *)&getNTP);
	getNTP.head.cmd = CMD_GET_NTP;
	getNTP.head.sn = gizwitsProtocol.sn++;
	getNTP.head.len = exchangeBytes(sizeof(protocolCommon_t) - 4);
	getNTP.sum = gizProtocolSum((uint8_t *)&getNTP, sizeof(protocolCommon_t));
	ret = uartWrite((uint8_t *)&getNTP, sizeof(protocolCommon_t));
	if (ret < 0)
	{
		GIZWITS_LOG("ERR[NTP]: uart write error %d \n", ret);
	}

	gizProtocolWaitAck((uint8_t *)&getNTP, sizeof(protocolCommon_t));
}


/**
* @brief Get Module Info  获取模块信息

*

* @param[in] none
* @return none
*/
void gizwitsGetModuleInfo(void)
{
	int32_t ret = 0;
	protocolGetModuleInfo_t getModuleInfo;

	gizProtocolHeadInit((protocolHead_t *)&getModuleInfo);
	getModuleInfo.head.cmd = CMD_ASK_MODULE_INFO;
	getModuleInfo.head.sn = gizwitsProtocol.sn++;
	getModuleInfo.type = 0x0;
	getModuleInfo.head.len = exchangeBytes(sizeof(protocolGetModuleInfo_t) - 4);
	getModuleInfo.sum = gizProtocolSum((uint8_t *)&getModuleInfo, sizeof(protocolGetModuleInfo_t));
	ret = uartWrite((uint8_t *)&getModuleInfo, sizeof(protocolGetModuleInfo_t));
	if (ret < 0)
	{
		GIZWITS_LOG("ERR[NTP]: uart write error %d \n", ret);
	}

	gizProtocolWaitAck((uint8_t *)&getModuleInfo, sizeof(protocolGetModuleInfo_t));
}


/**
* @brief Module Info Analyse  模块信息分析
*
* @param [in] head :
*
* @return 0, Success， , other,Faild
*/
static int8_t gizProtocolModuleInfoHandle(protocolHead_t *head)
{
	protocolModuleInfo_t *moduleInfo = (protocolModuleInfo_t *)head;

	if (NULL == head)
	{
		GIZWITS_LOG("NTP is empty \n");
		return -1;
	}

	memcpy((uint8_t *)&gizwitsProtocol.wifiModuleNews, (uint8_t *)&moduleInfo->wifiModuleInfo, sizeof(moduleInfo_t));

	gizwitsProtocol.moduleInfoEvent.event[gizwitsProtocol.moduleInfoEvent.num] = MODULE_INFO;
	gizwitsProtocol.moduleInfoEvent.num++;

	gizwitsProtocol.issuedFlag = GET_MODULEINFO_TYPE;


	return 0;
}

/**
* @brief Protocol handling function   协议处理功能

*

* @param [in] currentData :The protocol data pointer  
* @return none
*/
int32_t gizwitsHandle(dataPoint_t *currentData)
{
	int8_t ret = 0;
#ifdef PROTOCOL_DEBUG
	uint16_t i = 0;
#endif
	uint8_t ackData[RB_MAX_LEN];
	uint16_t protocolLen = 0;
	uint32_t ackLen = 0;
	protocolHead_t *recvHead = NULL;
	char *didPtr = NULL;
	uint16_t offset = 0;


	if (NULL == currentData)
	{
		GIZWITS_LOG("GizwitsHandle Error , Illegal Param\n");
		return -1;
	}

	/*resend strategy*/
	gizProtocolAckHandle();
	ret = gizProtocolGetOnePacket(&pRb, gizwitsProtocol.protocolBuf, &protocolLen);

	if (0 == ret)
	{
		GIZWITS_LOG("Get One Packet!\n");

#ifdef PROTOCOL_DEBUG
		GIZWITS_LOG("WiFi2MCU[%4d:%4d]: ", gizGetTimerCount(), protocolLen);
		for (i = 0; i<protocolLen; i++)
		{
			GIZWITS_LOG("%02x ", gizwitsProtocol.protocolBuf[i]);
		}
		GIZWITS_LOG("\n");
#endif

		recvHead = (protocolHead_t *)gizwitsProtocol.protocolBuf;
		switch (recvHead->cmd)
		{
		case CMD_GET_DEVICE_INTO:
			gizProtocolGetDeviceInfo(recvHead);
			break;
		case CMD_ISSUED_P0:
			GIZWITS_LOG("flag %x %x \n", recvHead->flags[0], recvHead->flags[1]);
			//offset = 1;

			if (0 == gizProtocolIssuedProcess(didPtr, gizwitsProtocol.protocolBuf + sizeof(protocolHead_t) + offset, protocolLen - (sizeof(protocolHead_t) + offset + 1), ackData, &ackLen))
			{
				gizProtocolIssuedDataAck(recvHead, ackData, ackLen, recvHead->flags[1]);
				GIZWITS_LOG("AckData : \n");
			}
			break;
		case CMD_HEARTBEAT:
			gizProtocolCommonAck(recvHead);
			break;
		case CMD_WIFISTATUS:
			gizProtocolCommonAck(recvHead);
			gizProtocolModuleStatus((protocolWifiStatus_t *)recvHead);
			break;
		case ACK_REPORT_P0:
		case ACK_WIFI_CONFIG:
		case ACK_SET_DEFAULT:
		case ACK_NINABLE_MODE:
			gizProtocolWaitAckCheck(recvHead);
			break;
		case CMD_MCU_REBOOT:
			gizProtocolCommonAck(recvHead);
			GIZWITS_LOG("report:MCU reboot!\n");

			gizProtocolReboot();
			break;
		case CMD_ERROR_PACKAGE:
			break;
		case ACK_PRODUCTION_TEST:
			gizProtocolWaitAckCheck(recvHead);
			GIZWITS_LOG("Ack PRODUCTION_MODE success \n");
			break;
		case ACK_GET_NTP:
			gizProtocolWaitAckCheck(recvHead);
			gizProtocolNTP(recvHead);
			GIZWITS_LOG("Ack GET_UTT success \n");
			break;
		case ACK_ASK_MODULE_INFO:
			gizProtocolWaitAckCheck(recvHead);
			gizProtocolModuleInfoHandle(recvHead);
			GIZWITS_LOG("Ack GET_Module success \n");
			break;

		default:
			gizProtocolErrorCmd(recvHead, ERROR_CMD);
			GIZWITS_LOG("ERR: cmd code error!\n");
			break;
		}
	}
	else if (-2 == ret)
	{
		//Check failed, report exception
		recvHead = (protocolHead_t *)gizwitsProtocol.protocolBuf;
		gizProtocolErrorCmd(recvHead, ERROR_ACK_SUM);
		GIZWITS_LOG("ERR: check sum error!\n");
		return -2;
	}

	switch (gizwitsProtocol.issuedFlag)
	{
	case ACTION_CONTROL_TYPE:
		gizwitsProtocol.issuedFlag = STATELESS_TYPE;
		gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)&gizwitsProtocol.gizCurrentDataPoint, sizeof(dataPoint_t));
		memset((uint8_t *)&gizwitsProtocol.issuedProcessEvent, 0x0, sizeof(gizwitsProtocol.issuedProcessEvent));
		break;
	case WIFI_STATUS_TYPE:
		gizwitsProtocol.issuedFlag = STATELESS_TYPE;
		gizwitsEventProcess(&gizwitsProtocol.wifiStatusEvent, (uint8_t *)&gizwitsProtocol.wifiStatusData, sizeof(moduleStatusInfo_t));
		memset((uint8_t *)&gizwitsProtocol.wifiStatusEvent, 0x0, sizeof(gizwitsProtocol.wifiStatusEvent));
		break;
	case ACTION_W2D_TRANSPARENT_TYPE:
		gizwitsProtocol.issuedFlag = STATELESS_TYPE;
		gizwitsEventProcess(&gizwitsProtocol.issuedProcessEvent, (uint8_t *)gizwitsProtocol.transparentBuff, gizwitsProtocol.transparentLen);
		break;
	case GET_NTP_TYPE:
		gizwitsProtocol.issuedFlag = STATELESS_TYPE;
		gizwitsEventProcess(&gizwitsProtocol.NTPEvent, (uint8_t *)&gizwitsProtocol.TimeNTP, sizeof(protocolTime_t));
		memset((uint8_t *)&gizwitsProtocol.NTPEvent, 0x0, sizeof(gizwitsProtocol.NTPEvent));
		break;
	case GET_MODULEINFO_TYPE:
		gizwitsProtocol.issuedFlag = STATELESS_TYPE;
		gizwitsEventProcess(&gizwitsProtocol.moduleInfoEvent, (uint8_t *)&gizwitsProtocol.wifiModuleNews, sizeof(moduleInfo_t));
		memset((uint8_t *)&gizwitsProtocol.moduleInfoEvent, 0x0, sizeof(moduleInfo_t));
		break;
	default:
		break;
	}

	gizDevReportPolicy(currentData);

	return 0;
}

/**
* @brief gizwits report transparent data interface  机智云上报透传数据接口

* The user can call the interface to complete the reporting of private protocol data
	用户可以调用接口来完成对私有协议数据的上报

* @param [in] data :Private protocol data
* @param [in] len  :Private protocol data length
* @return 0，success ;other，failure
*/
int32_t gizwitsPassthroughData(uint8_t * gizdata, uint32_t len)
{
	int32_t ret = 0;
	uint8_t tx_buf[MAX_PACKAGE_LEN];
	uint8_t *pTxBuf = tx_buf;
	uint16_t data_len = 6 + len;
	if (NULL == gizdata)
	{
		GIZWITS_LOG("[ERR] gizwitsPassthroughData Error \n");
		return (-1);
	}

	*pTxBuf++ = 0xFF;
	*pTxBuf++ = 0xFF;
	*pTxBuf++ = (uint8_t)(data_len >> 8);//len
	*pTxBuf++ = (uint8_t)(data_len);
	*pTxBuf++ = CMD_REPORT_P0;//0x1b cmd
	*pTxBuf++ = gizwitsProtocol.sn++;//sn
	*pTxBuf++ = 0x00;//flag
	*pTxBuf++ = 0x00;//flag
	*pTxBuf++ = ACTION_D2W_TRANSPARENT_DATA;//P0_Cmd

	memcpy(&tx_buf[9], gizdata, len);
	tx_buf[data_len + 4 - 1] = gizProtocolSum(tx_buf, (data_len + 4));

	ret = uartWrite(tx_buf, data_len + 4);
	if (ret < 0)
	{
		GIZWITS_LOG("ERR: uart write error %d \n", ret);
	}

	gizProtocolWaitAck(tx_buf, data_len + 4);

	return 0;
}

/**@} */
