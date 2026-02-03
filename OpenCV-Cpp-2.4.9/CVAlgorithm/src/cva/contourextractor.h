#ifndef ContourExtractor_H
#define ContourExtractor_H

#pragma execution_character_set("utf-8")

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

class ContourExtractor {
private:
    double m_minContourArea;     // 最小轮廓面积阈值
    bool m_removeNoise;          // 是否去除噪声
    int m_noiseKernelSize;       // 去噪核大小
    int m_adaptiveBlockSize;     // 自适应阈值块大小

public:
    ContourExtractor() {
        m_minContourArea = 1000.0;   // 默认面积阈值 > 1000
        m_removeNoise = true;
        m_noiseKernelSize = 3;
        m_adaptiveBlockSize = 11;
    }

    // 设置参数
    void setParameters(double minArea = 1000.0, bool removeNoise = true,
                       int noiseKernelSize = 3, int adaptiveBlockSize = 11) {
        m_minContourArea = minArea;
        m_removeNoise = removeNoise;
        m_noiseKernelSize = noiseKernelSize;
        m_adaptiveBlockSize = adaptiveBlockSize;
    }

    // 主接口：输入cv::Mat，返回提取的轮廓
    std::vector<std::vector<cv::Point>> extractWhiteBackgroundContour(const cv::Mat& inputImage);

    // 增强版本：返回轮廓+调试图像
    std::tuple<std::vector<std::vector<cv::Point>>, cv::Mat> extractContourWithDebug(const cv::Mat& inputImage);

    std::vector<cv::Point> findLargestContour(const cv::Mat& src, int whiteThreshold = 240);

    static std::vector<std::vector<cv::Point>> findContours(const cv::Mat &inputImage, int whiteThreshold = 240, int areaThreshold = 2000);

private:
    // 预处理图像：优化对比度和清晰度
    cv::Mat preprocessForWhiteBackground(const cv::Mat& input);

    // 自动分割白色背景
    cv::Mat segmentWhiteBackground(const cv::Mat& grayImage);

    // 查找并过滤轮廓
    std::vector<std::vector<cv::Point>> findAndFilterContours(const cv::Mat& binary);

    // 创建调试图像
    cv::Mat createDebugImage(const cv::Mat& original, const cv::Mat& processed, const cv::Mat& binary, const std::vector<std::vector<cv::Point>>& contours);

    // 将单通道图像转换为三通道用于显示
    cv::Mat convertTo3Channel(const cv::Mat& gray);
};

#endif // ContourExtractor_H
