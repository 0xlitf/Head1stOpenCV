#ifndef HPDETECTIONPARTSTRAINER_H
#define HPDETECTIONPARTSTRAINER_H
#pragma once
#include "cfg.h"
#include "halcon.h"
#include "halconDLL.h"
#include "qt_halcon.h"
#include <HalconCpp.h>

#ifdef __APPLE__
#include <HALCONCpp/HALCONCpp.h>
#include <HDevEngineCpp/HDevEngineCpp.h>
#else
#include "HDevEngineCpp.h"
#include "HalconCpp.h"
#endif

using namespace std;
using namespace HalconCpp;
using namespace HDevEngineCpp;

struct ResultData {
    cv::Mat image;
    HObject region;
    HTuple modelID;
    HTuple modelWidth;
    HTuple modelHeight;
    HTuple modelArea;
    HTuple ModelGrayVal;
    int train_count;
};

struct MyTrainData {
    HObject RegionROI;
    HObject hModelRegions;
    HTuple WindowHandle;
    HTuple ModelWidthAll;
    HTuple ModelHeightAll;
    HTuple ModelIDs;
    HTuple ModelGrayAll;
    HTuple hThreshold;
    HTuple hWdiffValue;

    bool bTraining;
    std::string strError;
    std::string strProject;
    ResultData resultData;
    vector<ResultData> resultList;
};

class HPDetectionPartsTrainer {
public:
    HPDetectionPartsTrainer();
    ~HPDetectionPartsTrainer();

    std::string project();
    bool updateParam(int threshValue, int wdiffValue);
    void setProject(std::string strProject);

    bool train(cv::Mat &image, cv::Mat &trainResult);

    bool start(std::string strProject, std::string roiPath);
    bool finish();
    void stop();
    bool isTraining();
    int getDetectObjNum();               // 获取检测到样本种类数量
    int getTrainedObjNum();              // 获取已训练样本个数
    void trainCountAdd();                // 手动增加训练次数
    void HD_dynamicTrainMode(bool flag); // 设置halcon动态训练模式
    void setTrainTarget(int num);        // 设置目标训练次数，达到才能保存数据

    cv::Mat getTrainedImage();             // 获取当前正在训练的图像
    cv::Size getTrainedSize();             // 获取当前正在训练的目标尺寸
    double getCalibFactor(cv::Mat &image); // 根据标定图片获取比例系数
    std::string errorString();

private:
    void init();
    void uninit();
    void saveTrainData();

#ifndef HALCONCPlus
    std::shared_ptr<HDevEngine> pDevEngine;
    std::shared_ptr<HDevProcedure> pDevProcedure;
    std::shared_ptr<HDevProcedureCall> pDevProcedureCall;
#endif

    // roi
    int m_roiLTX;
    int m_roiLTY;
    int m_roiWidth;
    int m_roiHeight;

    int m_trainNumHD;         // 物料训练深度
    int m_isDynamicHD;        // halcon动态训练模式
    int m_isOutBoundryHD = 0; // 出视野边界flag
    HTuple HP_matchID;        // 当前检测到的modelID

    struct MyTrainData *m_pData;
};

#endif // HPDETECTIONPARTSTRAINER_H
