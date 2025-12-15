#ifndef HPDETECTIONPARTSTRAINER_DYNAMIC_H
#define HPDETECTIONPARTSTRAINER_DYNAMIC_H

#ifdef __APPLE__
#include <HALCONCpp/HALCONCpp.h>
#include <HDevEngineCpp/HDevEngineCpp.h>
#else
#include "HalconCpp.h"
#include "HDevEngineCpp.h"
#endif
#include <QVector>
#include <QDir>
#include <QPainter>
#include <QString>
#include <QImage>
#include <QRect>
#include <QTextStream>
#include <iostream>
#include <string>
#include <future>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string> // string, std::to_string
#include "cfg.h"

//#define USE_OPENCV
using namespace std;

using namespace Halcon;
using namespace HDevEngineCpp;
using namespace cv;

#ifdef USE_OPENCV
bool matToHobject(cv::Mat &image, Hobject &hImage);
bool hobjectToMat(Hobject &Hobj, cv::Mat &rImage);
#else
    #include "qt_halcon.h"
#endif // USE_OPENCV


struct ResultData_Dynamic
{
#ifdef USE_OPENCV
    cv::Mat        image;
#else
    QImage        image;
#endif // USE_OPENCV
    Hobject        region;
    HTuple        modelID;
    HTuple        modelWidth;
    HTuple        modelHeight;
    HTuple        modelArea;
    int train_count;
};

struct TrainData
{
    HTuple             WindowHandle;
    HTuple             ModelWidthAll;
    HTuple             ModelHeightAll;

    bool               bTraining;
    QRect              bgRoi;
    QRect              partRoi;
    QString            strError;
    QString            strProject;
    ResultData_Dynamic         resultData;
    QList<ResultData_Dynamic>  resultList;
};

class HPDetectionPartsTrainer_Dynamic
{
public:
    HPDetectionPartsTrainer_Dynamic();
    ~HPDetectionPartsTrainer_Dynamic();

    QString project();
    void setProject(QString strProject);

    bool start(QString strProject, QRect bgRoi);
    void stop();
    bool finish(const QString &strPath, bool bIsHPdetection);
    bool isTraining();
    int trainTimes();
    QString errorString();
#ifdef USE_OPENCV
    bool train(cv::Mat &image);
#else
    bool train(QImage &image, const QString &strPath, QImage &outImage);
#endif // USE_OPENCV
    void saveTrainData();


private:
    void init();
    void uninit();
    std::shared_ptr<HDevEngine> pDevEngine;
    std::shared_ptr<HDevProcedure> pDevProcedure;
    std::shared_ptr<HDevProcedureCall> pDevProcedureCall;

    struct TrainData *m_pData;
};

#endif // HPDETECTIONPARTSTRAINER_DYNAMIC_H
