#pragma once
#ifndef HPDETECTIONPARTSCOUNTER_H
#define HPDETECTIONPARTSCOUNTER_H

#include "cfg.h"
#include "halconDLL.h"
#include <HDevEngineCpp.h>
#include <HalconCpp.h>

using namespace HalconCpp;
using namespace HDevEngineCpp;

enum { stateNormal, sysParamError, roiError, projectError };

struct CounterData {
    std::string strError;
    HObject ImageNew;
    HObject RegionROI;
    HObject hModelRegionsOrigin;
    HTuple hModelID;
    HTuple hModelGrays;
    HTuple hModelPath;
    HTuple hRegionPath;
    HTuple WindowHandle;
    HTuple hThreshold;
    HTuple hWdiffValue;
    HTuple hMinScore;
    HTuple hMinDefectInnerRadian;
    HTuple hMinDefectArea;
    HTuple hDefectThresh;
    int state;
    bool bRun;
    bool bRoi;
    bool bProject;
};

class HPDetectionPartsCounter {
public:
    HPDetectionPartsCounter();
    ~HPDetectionPartsCounter();

    bool setDLMode(int halconDLModel);
    bool updateDLMode(std::wstring strPath);
    bool updateProject(std::string strProjectPath, int trainID = 0);
    bool updateROI(int x, int y, int width, int height);
    bool updateParamHP();
    void stop();
    bool isWorking();
    std::string errorString();
    int getStatus();
    std::vector<ObjData> getObjData() { return resultList; } // 获取物料信息
    // bool action(const uchar *pImageBits, vector<ObjData> &resultList);
    bool action(cv::Mat &img, vector<ObjData> &resultList, cv::Mat &result);

private:
    void init();
    void uninit();
#ifndef HALCONCPlus
    std::shared_ptr<HDevEngine> pDevEngine;
    std::shared_ptr<HDevProcedure> pDevProcedure;
    std::shared_ptr<HDevProcedureCall> pDevProcedureCall;
#endif

    // Local control variables
    // HTuple  hv_ImageDir, hv_ExampleDataDir, hv_OutputDir;
    HTuple hv_FileExists, hv_DLDeviceHandle, hv_DLDeviceHandles;
    // HTuple  hv_DLPreprocessParam, hv_PreprocessSettings, hv_DLDataInfo;
    // HTuple  hv_ClassNames, hv_ClassIDs, hv_MaxNumInferenceImages;
    // HTuple  hv_AnomalyInferenceImagePaths, hv_WindowHandle;
    // HTuple  hv_Threshold, hv_WindowDict, hv_IndexInference;
    // HTuple  hv_Width, hv_Height, hv_NewRow, hv_NewColumn, hv_NewWidth;
    // HTuple  hv_NewArea, hv_Result, hv_DLSample, hv_DLResult;

    // Local iconic variables
    HObject ho_Image;

    // Local control variables
    HTuple hv_ImageDir, hv_ImageSubDirs, hv_OutputDir;
    HTuple hv_DLModelHandle, hv_DLPreprocessParam, hv_PreprocessSettings;
    HTuple hv_DLDatasetInfo, hv_MaxNumInferenceImages,
        hv_AnomalyInferenceImagePaths;
    HTuple hv_WindowHandle, hv_Threshold, hv_WindowDict, hv_IndexInference;
    HTuple hv_Width, hv_Height, hv_DLSample, hv_DLResult, hv_Result;
    HTuple hv_score, hv_class, hv___Tmp_Ctrl_0;
    HTuple hv_MetaData, hv_InferenceClassificationThreshold,
        hv_InferenceSegmentationThreshold;

    std::vector<cv::Mat> m_obj_hImage; // 物料裁剪小图
    std::string m_strProjectPath;
    vector<string> m_strRegionPathList;
    vector<string> m_strModelPathList;
    vector<ObjData> resultList; // 物料检测结果
    CounterData *m_pData;
};

#endif // HPDETECTIONPARTSCOUNTER_H
