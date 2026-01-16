#ifndef DEFECTDETECTOR_H
#define DEFECTDETECTOR_H

#include <QObject>
#include <opencv2/opencv.hpp>

class DefectDetector : public QObject
{
    Q_OBJECT

public:
    explicit DefectDetector(QObject *parent = nullptr);

    // 缺陷检测主接口
    struct DefectResult {
        double totalLoss;           // 总损失值 (0-1, 越大缺陷越严重)
        double correlationLoss;     // 相关性损失
        double chiSquareLoss;       // 卡方损失
        double bhattacharyyaLoss;   // 巴氏距离损失
        cv::Mat defectMap;          // 缺陷热力图
        bool hasDefect;             // 是否有缺陷
    };

    DefectResult detectDefect(const cv::Mat& normalImage, const cv::Mat& defectImage);

    // 设置检测参数
    void setDetectionThreshold(double threshold) { m_threshold = threshold; }
    void setHistogramBins(int hBins, int sBins) { m_hBins = hBins; m_sBins = sBins; }

private:
    // HSV直方图比较
    double compareHSVHistograms(const cv::Mat& hsv1, const cv::Mat& hsv2, int method);

    // 生成缺陷可视化图
    cv::Mat generateDefectMap(const cv::Mat& normalImage, const cv::Mat& defectImage);

    // 参数
    double m_threshold = 0.3;  // 缺陷阈值
    int m_hBins = 50;          // H通道bins数量
    int m_sBins = 60;          // S通道bins数量
};

#endif // DEFECTDETECTOR_H
