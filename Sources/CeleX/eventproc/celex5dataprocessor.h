/*
* Copyright (c) 2017-2018  CelePixel Technology Co. Ltd.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef CELEX5DATAPROCESSOR_H
#define CELEX5DATAPROCESSOR_H

#include "../include/celex5/celex5.h"
#include "../include/celex5/celex5processeddata.h"
#include "../include/celex5/celex5datamanager.h"

#define FORMAT1_T_MAX    65536 // 2^16
#define FORMAT2_T_MAX    4096  // 2^12

class CeleX5DataProcessor
{
public:
	CeleX5DataProcessor();
	~CeleX5DataProcessor();

	void getFullPicBuffer(unsigned char* buffer);
	void getFullPicBuffer(unsigned char* buffer, std::time_t& time_stamp);
	//
	void getEventPicBuffer(unsigned char* buffer, CeleX5::emEventPicType type);
	void getEventPicBuffer(unsigned char* buffer, std::time_t& time_stamp, CeleX5::emEventPicType type);
	//
	void getOpticalFlowPicBuffer(unsigned char* buffer, CeleX5::emFullPicType type);
	void getOpticalFlowPicBuffer(unsigned char* buffer, std::time_t& time_stamp, CeleX5::emFullPicType type);

	bool getEventDataVector(std::vector<EventData> &vecData);
	bool getEventDataVector(std::vector<EventData> &vecData, uint64_t& frameNo);
	bool getEventDataVectorEx(std::vector<EventData> &vecData, std::time_t& time_stamp, bool bDenoised);

	void processMIPIData(uint8_t* pData, int dataSize, std::time_t time_stamp_end, vector<IMURawData> imu_data);

	void disableFrameModule();
	void enableFrameModule();
	bool isFrameModuleEnabled();

	void disableIMUModule();
	void enableIMUModule();
	bool isIMUModuleEnabled();

	CX5SensorDataServer *getSensorDataServer();
	CeleX5ProcessedData *getProcessedData();

	bool setFpnFile(const std::string& fpnFile);
	void generateFPN(std::string filePath);

	CeleX5::CeleX5Mode getSensorFixedMode();
	CeleX5::CeleX5Mode getSensorLoopMode(int loopNum);
	void setSensorFixedMode(CeleX5::CeleX5Mode mode);
	void setSensorLoopMode(CeleX5::CeleX5Mode mode, int loopNum);
	void setLoopModeEnabled(bool enable);
	void setISOLevel(uint32_t value);

	void setMIPIDataFormat(int format);

	void setEventFrameTime(uint32_t value, uint32_t clock);
	uint32_t getEventFrameTime();
	void setEventShowMethod(EventShowType type, int value);
	EventShowType getEventShowMethod();
	void setEventFrameStartPos(uint32_t value);
	void setRotateType(int type);
	int  getRotateType();
	void setEventCountStep(uint32_t step);
	uint32_t getEventCountStep();
	int getIMUData(std::vector<IMUData>& data);

	void saveFullPicRawData();
	void resetTimestamp();
	uint32_t getEventRate();

private:
	void processFullPicData(uint8_t* pData, int dataSize, std::time_t time_stamp_end);
	void parseEventDataFormat0(uint8_t* pData, int dataSize); //Format0: 24-bit packet with ADC data (CSR_73=2'b00)
	void parseEventDataFormat1(uint8_t* pData, int dataSize); //Format1: 28-bit packet with ADC data (CSR_73=2'b01)
	void parseEventDataFormat2(uint8_t* pData, int dataSize); //Format2: 14-bit packet without ADC data (CSR_73=2'b10)
	void parseIMUData(std::time_t time_stamp);

	void checkIfShowImage(); //only for mipi
	bool createImage(std::time_t time_stamp_end);
	unsigned int normalizeADC(unsigned int adc);
	void generateFPNimpl();
	int calculateDenoiseScore(unsigned char* pBuffer, unsigned int pos);
	int calMean(unsigned char* pBuffer, unsigned int pos);
	void calDirectionAndSpeed(int i, int j, unsigned char* pBuffer, unsigned char* &speedBuffer, unsigned char* &dirBuffer);

	void saveFullPicRawData(uint8_t* pData);

	inline void processMIPIEventTimeStamp() 
	{
		if (m_bFirstEventTimestamp)
		{
			m_uiEventTCounter_Total = m_iRowTimeStamp;
			m_bFirstEventTimestamp = false;
		}
		//cout << "m_iRowTimeStamp = " << m_iRowTimeStamp << ", m_iLastRowTimeStamp = " << m_iLastRowTimeStamp << endl;
		int diffT = m_iRowTimeStamp - m_iLastRowTimeStamp;
		if (diffT < 0)
		{
			if (1 == m_iMIPIDataFormat)
				diffT += FORMAT1_T_MAX;
			else
				diffT += FORMAT2_T_MAX;
		}	
		/*if (diffT > 1)
			cout << __FUNCTION__ << ": T is not continuous!" << endl;*/
		if (m_iLastRowTimeStamp != -1 && diffT < 5)
		{
			m_uiEventTCounter += diffT;
			m_uiEventTCounter_Total += diffT;
			m_uiEventTCounter_EPS += diffT;
			m_uiPackageTCounter += diffT;
			//cout << "m_uiEventTCounter_Total = " << m_uiEventTCounter_Total << endl;
		}
		if (!m_bLoopModeEnabled)
		{
			checkIfShowImage();
		}
		if (m_uiEventTCounter_EPS > m_uiEventTCountForEPS)
		{
			//cout << "m_uiPixelCountForER = " << m_uiPixelCountForER << endl;
			m_uiEventNumberEPS = m_uiPixelCountForEPS;
			m_uiPixelCountForEPS = 0;
			m_uiEventTCounter_EPS = 0;
		}
	}

private:
	CeleX5ProcessedData*     m_pCX5ProcessedData;
	CX5SensorDataServer*     m_pCX5Server;
	//
	unsigned char*           m_pEventCountBuffer;
	unsigned char*           m_pEventADCBuffer;
	uint16_t*                m_pLastADC;
	//for fpn
	long*                    m_pFpnGenerationBuffer;
	int*                     m_pFpnBuffer;
	//
	//
	unsigned char*           m_pFullFrameBuffer_ForUser;

	unsigned char*           m_pEventFrameBuffer1_ForUser;
	unsigned char*           m_pEventFrameBuffer2_ForUser;
	unsigned char*           m_pEventFrameBuffer3_ForUser;
	unsigned char*           m_pEventFrameBuffer4_ForUser;
	unsigned char*           m_pEventFrameBuffer5_ForUser;
	unsigned char*           m_pEventFrameBuffer6_ForUser;
	unsigned char*           m_pEventFrameBuffer7_ForUser;

	unsigned char*           m_pOpticalFrameBuffer1_ForUser;
	unsigned char*           m_pOpticalFrameBuffer2_ForUser;
	unsigned char*           m_pOpticalFrameBuffer3_ForUser;
	//
	CeleX5::CeleX5Mode       m_emCurrentSensorMode;
	CeleX5::CeleX5Mode       m_emSensorFixedMode;
	CeleX5::CeleX5Mode       m_emSensorLoopAMode;
	CeleX5::CeleX5Mode       m_emSensorLoopBMode;
	CeleX5::CeleX5Mode       m_emSensorLoopCMode;
	//
	string                   m_strFpnFilePath;
	//
	uint32_t                 m_uiPixelCount;
	uint32_t                 m_uiPixelCountForEPS; //For event rate
	uint32_t                 m_uiEventNumberEPS; //The number of events that being produced per second
	uint32_t                 m_uiEventTCounter; //This value will be reset after the end of a frame
	uint64_t                 m_uiEventTCounter_Total; //This value won't be reset, it's a monotonically increasing value
	uint64_t                 m_uiEventTCounter_EPS; //This value will be reset after the end of a second
	uint32_t                 m_uiEventRowCycleCount;

	uint32_t                 m_uiEventTCountForShow;
	uint32_t                 m_uiEventTCountForRemove;//The data between 0 and m_uiEventTCountForRemove will be removed
	uint32_t                 m_uiEventTCountForEPS; //The number of T units corresponding to 1s.
	uint32_t                 m_uiEventCountForShow;  
	uint32_t                 m_uiEventRowCycleCountForShow;

	uint32_t                 m_uiISOLevel; //range: 1 ~ 6
	EventShowType            m_emEventShowType;

	int32_t                  m_iLastRowTimeStamp;
	int32_t                  m_iRowTimeStamp;
	
	int32_t                  m_iLastRow;
	int32_t                  m_iCurrentRow;
	uint32_t                 m_uiRowCount;

	int                      m_iFpnCalculationTimes;
	uint32_t                 m_uiEventFrameTime;

	bool                     m_bIsGeneratingFPN;
	bool                     m_bLoopModeEnabled;
	int						 m_iRotateType;	//rotate param

	vector<EventData>        m_vecEventData;
	vector<EventData>        m_vecEventData_ForUser;

	int                      m_iMIPIDataFormat;
	uint16_t                 m_uiEventTUnitList[16];
	
	std::ofstream            m_ofLogFile;
	uint64_t                 m_uiEventFrameNo;
	std::time_t              m_lEventFrameTimeStamp;
	std::time_t              m_lLastPackageTimeStamp;
	uint32_t                 m_uiPackageTCounter;
	uint32_t                 m_uiEventCountStep;

	vector<IMUData>          m_vectorIMUData;
	vector<IMURawData>       m_vectorIMU_Raw_data;

	bool                     m_bSaveFullPicRawData;
	uint32_t                 m_uiCurrentEventTUnit;
	//
	bool                     m_bFrameModuleEnabled;
	bool                     m_bIMUModuleEnabled;
	//
	std::time_t              m_lFullFrameTimeStamp_ForUser;
	std::time_t              m_lEventFrameTimeStamp_ForUser;
	std::time_t              m_lOpticalFrameTimeStamp_ForUser;
	bool                     m_bFirstEventTimestamp;
};

#endif // CELEX5DATAPROCESSOR_H
