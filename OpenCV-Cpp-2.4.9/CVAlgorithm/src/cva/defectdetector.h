#ifndef DEFECTDETECTOR_H
#define DEFECTDETECTOR_H

#pragma execution_character_set("utf-8")

#include "bgr2hsvconverter.h"
#include "contourextractor.h"
#include "cornersplitter.h"
#include "minimumbounding.h"
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

    void setTemplateFolder(const QStringList &descStrs, const QStringList &folderName);

    void setInputMat(cv::Mat inputMat);

    std::tuple<bool, double> p0_matchArea();

    std::tuple<bool, double> p1_matchShapes();

    std::tuple<bool, double> p2_matchSubAreas();

    std::tuple<bool, double> p3_matchSubShapes();

    std::tuple<bool, double> p4_fullMatchMatPixel();

    double scoreThreshold() const;
    void setScoreThreshold(double newScoreThreshold);

    int whiteThreshold() const;
    void setWhiteThreshold(int newWhiteThreshold);

    bool hasDefect(double scoreThreshold) { return scoreThreshold > m_scoreThreshold; }

    int precision() const;
    void setPrecision(int newPrecision);

    bool debugImageFlag() const;
    void setDebugImageFlag(bool newDebugImageFlag);

    double matchMatPixel(cv::Mat templateInput, cv::Mat defectInput);

    cv::Mat thresholdDiff() const;

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

private: // 工具类
    MinimumBounding m_mini;
    BGR2HSVConverter m_converter;
    CornerSplitter m_cornerSplitter;
    ContourExtractor m_extractor;

    // 输入图片的临时变量
    cv::Mat m_inputMat;
    std::vector<cv::Point> m_inputMatContour;
    double m_inputMatArea;
    std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> m_corners;
    std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> m_subContours;
    std::tuple<double, double, double, double> m_subContourAreas;

    cv::Mat m_thresholdDiff;

private:
    QList<std::tuple<QString,
                     QString,
                     cv::Mat,
                     std::vector<cv::Point>,
                     double,
                     std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat>,
                     std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>>,
                     std::tuple<double, double, double, double>>> m_templateList;

    int m_whiteThreshold{35};            // 差值结果阈值，大于这个值被认为是缺陷点，一般设置为30-40

    // 以下参数暂时不改
    int m_precision{2};   // 取决于进行几次下采样，暂时不可更改
    bool m_useHSV{false}; // false true
    bool m_debugImageFlag{false}; // false true 是否输出调试结果图片

    // 下列参数值暂时只适用于矩形
    double m_overallAreaThreshold{0.02}; // 整体轮廓面积，小于0.02合格，对于比较厚的物料，适当增大本阈值
    double m_overallShapeThreshold{0.01}; // 整体轮廓形状分数，小于0.01~0.05合格
    double m_subAreaThreshold{0.02}; // 子区域轮廓面积，对于细微的角落缺陷，小于0.02合格
    double m_subShapeThreshold{0.002}; // 子区域轮廓形状分数，小于0.002合格

    // 非矩形物料暂时不要用像素检测
    int m_outterWidth{4};
    int m_innerWidth{10};
    double m_scoreThreshold{15}; // 缺陷点的个数，根据下采样的次数决定，m_precision为2时，此数值一般为10-20
};

#endif // DEFECTDETECTOR_H
