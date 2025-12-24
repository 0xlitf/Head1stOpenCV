/*****************************************************************************
name: partstrainer.h
date:  2016.8.23(start)
******************************************************************************/
#ifndef PARTSTRAINER_H
#define PARTSTRAINER_H

#include "cfg.h"
#include "hp_detection_partstrainer.h"
using namespace cv;

struct CvResultData {
    cv::Mat image;
    int modelID;
    int modelWidth;
    int modelHeight;
    int modelArea;
    int train_count;
};

class PartsTrainer {
public:
    PartsTrainer();
    ~PartsTrainer();
    bool updateParam_train(); // 更新系统参数
    void reset(std::string strProjectPath, std::string strRoiPath);
    bool training(cv::Mat *trainImg, cv::Mat *trainResult, int thresh = 200);
    void estimateTrainObj(); // 评估物料是否未训练过
    void saveData();
    void cleanData();
    int dataCount() { return (int)m_trainContours.size(); }
    bool writeImage(cv::Mat *img);
    bool writeTrainDataToXml();
    void run(cv::Mat *img, std::string strProjectPath, std::string strRoiPath);

    bool HPDetection_start(std::string strProject, std::string roiPath);
    bool HPDetection_train(cv::Mat &image, cv::Mat &trainResult);
    bool HPDetection_finish();
    int getDetectObjNum();  // 获取已训练缓存模型个数
    int getTrainedObjNum(); // 获取达到训练深度的模型个数
    cv::Mat getTrainedImage();
    cv::Size getTrainedSize();
    double getCalibFactor(cv::Mat &image);
    void dynamicTrainModeOpen();  // 开启动态训练
    void dynamicTrainModeClose(); // 关闭动态训练
    void setCountDebug();         // 使用debug模式，基于视觉计数
    void setHalonMode(bool flag); // 使用halcon做精细检测
    void trainCountAdd();
    void setTrainTarget(int num); // 设置目标训练次数，达到才能保存数据
private:
    int m_openElementSize;
    int m_objHeight, m_objWidth, m_objArea,
        m_modelID; // 目标高度、宽度、面积、模型ID

    int m_trainNum = 3;       // 物料训练深度
    int m_isOutBoundry = 0;   // 出视野边界flag
    bool m_isDynamic = false; // false - 手动模式  && true - 动态模式
    bool m_useHalcon = false; // 是否使用halcon， true-使用
    std::vector<int> ModelAreaAll;
    std::vector<int> ModelWidthAll;
    std::vector<int> ModelHeightAll;
    std::vector<int> ModelIDs;
    std::vector<std::vector<cv::Point>> m_tempContours;
    std::vector<std::vector<cv::Point>> m_trainContours;

    CvResultData resultData;
    vector<CvResultData> resultList;
    // roi
    int m_roiLTX;
    int m_roiLTY;
    int m_roiWidth;
    int m_roiHeight;
    std::string m_strProjectPath;
    int isTrained; // 是否已经训练, 0-未训练， 1-训练成功， 2-已训练，
    // 3-出视野边界中

    struct CvTrainData *m_pTrainData;

    std::shared_ptr<HPDetectionPartsTrainer> ptr_HPDetectionPartsTrainer;
};

#endif PARTSTRAINER_H
