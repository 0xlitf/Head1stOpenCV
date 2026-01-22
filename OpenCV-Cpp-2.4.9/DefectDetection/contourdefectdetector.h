#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

#pragma execution_character_set("utf-8")

class ContourDefectDetector {
public:
    ContourDefectDetector() {
        m_contourMatchThreshold = 0.3;
        m_pixelDiffThreshold = 30;
        m_contourInspectWidth = 5;
        m_debugMode = true;
        m_minContourArea = 1000.0;
    }

    // 设置检测参数
    void setParameters(double contourThreshold, double pixelThreshold, int contourWidth, double minArea = 1000.0);

    // 主检测函数
    double matchMat(cv::Mat templateInput, cv::Mat defectInput);

private:
    // 图像预处理
    cv::Mat preprocessImage(const cv::Mat& input);

    // 提取主要轮廓
    std::vector<cv::Point> extractMainContour(const cv::Mat& image);

    // 计算轮廓相似度[5](@ref)
    double calculateContourSimilarity(const cv::Mat& templateImg, const cv::Mat& testImg);

    // 创建轮廓检测带掩膜
    cv::Mat createContourInspectionMask(const std::vector<cv::Point>& contour, cv::Size imageSize);

    // 计算轮廓区域像素差异
    double calculateContourZoneDifference(const cv::Mat& templateImg, const cv::Mat& testImg);

private:
    double m_contourMatchThreshold;  // 轮廓匹配阈值
    double m_pixelDiffThreshold;      // 像素差异阈值
    int m_contourInspectWidth;       // 轮廓检测带宽度
    bool m_debugMode;
    double m_minContourArea;         // 最小轮廓面积

};
