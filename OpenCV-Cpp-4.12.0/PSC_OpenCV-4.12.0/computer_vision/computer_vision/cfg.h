/*****************************************************************************
name: cfg.h
date:  2016.8.24(start)
******************************************************************************/
#ifndef CFG_H
#define CFG_H

#include <direct.h> //_mkdir fun
#include <io.h>     //_access fun
#include <string>
#include <vector>
#include <windows.h>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>

#define CONFIG_PATH "./config"

using namespace std;
using namespace cv;
#define HALCONCPlus // 决定采用halcon导出c++，还是直接调用halcon算子

struct RoiData {
    int roiLTX;
    int roiLTY;
    int roiWidth;
    int roiHeight;
    double roiLineLimitC;
};

struct ObjData {
    cv::Mat objRoiImage; // 物料区域ROI图像
    Rect objRoi;         // 物料水平外包围矩形框坐标

    double x = 0.0;     // 物料尾部的col坐标
    double y = 0.0;     // 物料中心高度row坐标
    double width = 0.0; // 物料水平宽度
    double area = 0.0;  // 物料面积
    int model_id = -1;  // 匹配的模版ID，应用于多分类任务
    int isOK_halcon = -1; // 物料识别结果  0-合格，1-杂质，2-其他（不作判断）
};

struct _TrainData {
    std::vector<std::vector<cv::Point>> trainContours;
    std::vector<int> trainArea;
    std::vector<int> trainHeight;
    std::vector<int> trainWidth;
    int modelID;
};

struct SysParam {
    int imgWidth;
    int imgHeight;
    int threshValue = 200;
    int openElementSize = 6;
    int xCompensate = -20;
    int matchMethod = 2;
    double similarity = 0.95;
    double lcoefficient = 0.9;
    double hcoefficient = 1.1;
    double adiffValue = 0.1;
    int wdiffValue = 3;
    int hdiffValue = 3;
    double roiLineLimitC = 0.98;

    int allowContourArea0 = 3000000;
    int allowContourArea1 = 50;

    double minScore = 0.7;
    int minDefectInnerRadian = 4;
    int NGEndingThresh = 0;

    int minDefectArea = 200;
    int defectThresh = 80;
};

// bool writeRoiDataToXml(const std::string strPath, RoiData* myRoiData);
bool readRoiDataFromXml(const std::string strPath, RoiData *myRoiData);
bool readTrainDataFromXml(const std::string strPath, _TrainData *trainData);
bool readSysParamFromXml(const std::string strPath);
bool saveTrainDataXML(const _TrainData &resultList, std::string save_path);
cv::Mat inverseColor6(cv::Mat srcImage);
std::vector<std::string> split(const std::string &str,
                               const std::string &pattern);
BOOL isFolderExist(const char *folder);
int32_t createDirectory(const char *directoryPath);
void getFiles(std::string path, vector<std::string> &files);
vector<std::string> fileFilter(vector<std::string> &Files,
                               const std::string &keyWord);
std::string getTimeString(bool bLocal = true, bool bIncludeMS = true);
bool compContours(const vector<Point> &a, const vector<Point> &b);
int get_max(std::vector<int> a);
void setObjAreaThresh(int areaThresh);

extern bool m_useCountDebug; // 0 - 关闭图像计数  && 非0 - 开启图像计数

enum {
    // halcon深度学习模型类型
    DL_CLASSIFICATION, // 分类模型  && 2-目标检测模型 && 3-异常检测模型  &&
    // 4-目标检测模型-水平框
    DL_OBJECT_DETECTION,     // 目标检测模型
    DL_ANOMALY_DETECTION,    // 异常检测模型
    DL_OBJECT_DETECTION_HOR, // 目标检测模型-水平框
    DL_MODE_CLOSED           // 关闭DL模式，采用传统halcon精细检测模型
};

extern int m_halconDLModel; // halcon深度学习模型类型

extern SysParam mySysParam; // 系统参数
#endif
