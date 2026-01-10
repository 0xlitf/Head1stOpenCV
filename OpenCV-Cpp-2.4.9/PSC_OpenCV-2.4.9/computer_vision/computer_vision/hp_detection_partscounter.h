#pragma once
#ifndef HPDETECTIONPARTSCOUNTER_H
#define HPDETECTIONPARTSCOUNTER_H
#include "qt_halcon.h"
#include "cfg.h"
#ifdef HALCONCPlus
#include "halcon.h"
#else
#include <HalconCpp.h>
#endif
#include <HDevEngineCpp.h>


using namespace Halcon;
using namespace HDevEngineCpp;

enum
{
	stateNormal,
	sysParamError,
	roiError,
	projectError
};


struct CounterData
{
	std::string            strError;
	Hobject            ImageNew;
	Hobject            RegionROI;
	Hobject            hModelRegionsOrigin;
	HTuple             hModelID;
	HTuple             hModelGrays;
	HTuple             hModelPath;
	HTuple             hRegionPath;
	HTuple             WindowHandle;
	HTuple             hThreshold;
	HTuple             hWdiffValue;
	HTuple             hMinScore;
	HTuple             hMinDefectInnerRadian;
	HTuple             hMinDefectArea;
	HTuple             hDefectThresh;
	int                state;
	bool               bRun;
	bool               bRoi;
	bool               bProject;
};

class HPDetectionPartsCounter
{
public:
	HPDetectionPartsCounter();
	~HPDetectionPartsCounter();



	bool updateProject(std::string strProjectPath, int trainID = 0);
	bool updateROI(int x, int y, int width, int height);
	bool updateParamHP();
	void stop();
	bool isWorking();
	std::string errorString();
	int getStatus();
	std::vector<ObjData> getObjData()  { return resultList; }				 //获取物料信息
	//bool action(const uchar *pImageBits, vector<ObjData> &resultList);
	bool action(cv::Mat& img, vector<ObjData>& resultList, cv::Mat& result);

private:
	void init();
	void uninit();
#ifndef HALCONCPlus
	std::shared_ptr<HDevEngine> pDevEngine;
	std::shared_ptr<HDevProcedure> pDevProcedure;
	std::shared_ptr<HDevProcedureCall> pDevProcedureCall;
#endif
	std::vector<cv::Mat> m_obj_hImage;               //物料裁剪小图
	std::string m_strProjectPath;
	vector<string> m_strRegionPathList;
	vector<string> m_strModelPathList;
	vector<ObjData> resultList;              //物料检测结果
	CounterData* m_pData;
};

#endif // HPDETECTIONPARTSCOUNTER_H
