#ifndef DEFECTDETECTOR_H
#define DEFECTDETECTOR_H

#pragma execution_character_set("utf-8")

#include "contourextractor.h"
#include "cornersplitter.h"
#include <QDebug>
#include <QMessageBox>
#include <QObject>
#include <opencv2/opencv.hpp>

class DefectDetector : public QObject {
    Q_OBJECT

public:
    enum ErrorCode {
        SUCCESS = 0,           // 成功
        FILE_NOT_FOUND = 1,    // 文件不存在
        IMAGE_LOAD_FAILED = 2, // 图像加载失败
        INVALID_IMAGE = 3,     // 无效图像
        NO_CONTOURS_FOUND = 4, // 未找到轮廓
        TEMPLATE_NOT_SET = 5,  // 模板未设置
        MATCH_FAILED = 6       // 匹配失败
    };
    Q_ENUM(ErrorCode) // 启用Qt元对象系统

signals:
    void sendLog(const QString &logStr);
    void errorOccured(ErrorCode errorCode, const QString &errorStr);

public:
    explicit DefectDetector(QObject *parent = nullptr);

    // 创建环形掩膜（精确的环形边缘区域）
    cv::Mat createRingEdgeMask(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeStartWidth = 4, int edgeEndWidth = 16);

    // 处理环形边缘，提取边缘区域
    cv::Mat processRingEdge(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeStartWidth = 4, int edgeEndWidth = 16);

    // 创建外边缘掩膜（向内6个像素的环形区域）
    cv::Mat createOuterEdgeMask(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeWidth = 6);

    // 显示外边缘掩膜
    cv::Mat processOuterEdge(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeWidth = 6);

    // 显示外边缘掩膜
    cv::Mat displayOuterEdge(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeWidth = 6);

    // 首先调用这个接口，分析图片中有几个物料，多的直接吹掉
    static std::tuple<int, cv::Mat> analyzeAndDrawContour(const cv::Mat &inputImage, int whiteThreshold = 240, int areaThreshold = 2000);

    void setTemplateFolder(const QStringList &descStrs, const QStringList &folderName);

    double p0_matchArea(double inputMatArea);

    double p1_matchShapes(std::vector<cv::Point> inputMatContour);

    double p2_matchSubAreas(std::tuple<double, double, double, double>);

    double p3_matchSubShapes(std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> inputCornerContours);

    double p3_matchMatPixels(cv::Mat inputImg);

    double p4_fullMatchMatPixel(cv::Mat inputImg);

    double fullMatchImagePixel(const QString &fileName);

    double fullMatchMatPixel(cv::Mat inputImg);

    double scoreThreshold() const;
    void setScoreThreshold(double newScoreThreshold);

    int whiteThreshold() const;
    void setWhiteThreshold(int newWhiteThreshold);

    int detectThickness() const;
    void setDetectThickness(int newDetectThickness);

    int removeOuterBorderThickness() const;
    void setRemoveOuterBorderThickness(int newRemoveOuterBorderThickness);

    bool hasDefect(double scoreThreshold) { return scoreThreshold > m_scoreThreshold; }

    int precision() const;
    void setPrecision(int newPrecision);

    bool debugImageFlag() const;
    void setDebugImageFlag(bool newDebugImageFlag);

    double matchMat(cv::Mat templateInput, cv::Mat defectInput);

    double matchMatPixel(cv::Mat templateInput, cv::Mat defectInput);

private:
    void addTemplate(const QString &desc, const QString &fileName);

    void addTemplateIntoMap(const QString &desc,
                            const QString &fileName,
                            cv::Mat tInput,
                            std::vector<cv::Point>,
                            double,
                            std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat>,
                            std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>>,
                            std::tuple<double, double, double, double>);

private:
    QList<std::tuple<QString,
                     QString,
                     cv::Mat,
                     std::vector<cv::Point>,
                     double,
                     std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat>,
                     std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>>,
                     std::tuple<double, double, double, double>>> m_templateList;
    int m_removeOuterBorderThickness{3}; // 比对时忽略的边缘厚度
    int m_detectThickness{6};            // 比对时检测的边缘厚度
    int m_whiteThreshold{35};            // 差值结果阈值，大于这个值被认为是缺陷点，一般设置为30-40
    double m_scoreThreshold{15};         // 缺陷点的个数，根据下采样的次数决定，m_precision为2时，此数值一般为10-20


    int m_outterWidth = 4;
    int m_innerWidth = 10;

    // 以下参数不改
    int m_precision{2};   // 取决于进行几次下采样，暂时不可更改
    bool m_useHSV{false}; // false true

    bool m_debugImageFlag{true}; // false true 是否输出调试结果图片

    CornerSplitter m_cornerSplitter;

    ContourExtractor m_extractor;
};

#endif // DEFECTDETECTOR_H
