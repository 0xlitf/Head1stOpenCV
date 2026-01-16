#include "DefectDetector.h"
#include <QDebug>
#include <opencv2/imgproc.hpp>

DefectDetector::DefectDetector(QObject *parent) : QObject(parent)
{
}

DefectDetector::DefectResult DefectDetector::detectDefect(
    const cv::Mat& normalImage, const cv::Mat& defectImage)
{
    DefectResult result;

    // 输入验证
    if (normalImage.empty() || defectImage.empty()) {
        qWarning() << "输入图像为空!";
        result.totalLoss = 1.0;
        result.hasDefect = true;
        return result;
    }

    // 统一图像尺寸
    cv::Mat resizedDefect;
    cv::resize(defectImage, resizedDefect, normalImage.size());

    // 转换为HSV颜色空间[1,6](@ref)
    cv::Mat hsvNormal, hsvDefect;
    cv::cvtColor(normalImage, hsvNormal, cv::COLOR_BGR2HSV);
    cv::cvtColor(resizedDefect, hsvDefect, cv::COLOR_BGR2HSV);

    // 计算不同方法的损失值[7](@ref)
    double correlation = compareHSVHistograms(hsvNormal, hsvDefect, cv::HISTCMP_CORREL);
    double chiSquare = compareHSVHistograms(hsvNormal, hsvDefect, cv::HISTCMP_CHISQR);
    double bhattacharyya = compareHSVHistograms(hsvNormal, hsvDefect, cv::HISTCMP_BHATTACHARYYA);

    // 归一化损失值 (0-1范围，越大表示缺陷越严重)
    result.correlationLoss = 1.0 - correlation;  // 相关性：1最好，0最差
    result.chiSquareLoss = std::min(chiSquare / 10.0, 1.0);  // 卡方：0最好，归一化
    result.bhattacharyyaLoss = bhattacharyya;  // 巴氏距离：0最好，1最差

    // 计算综合损失值 (加权平均)
    result.totalLoss = (result.correlationLoss * 0.4 +
                        result.chiSquareLoss * 0.3 +
                        result.bhattacharyyaLoss * 0.3);

    // 生成缺陷热力图
    result.defectMap = generateDefectMap(normalImage, resizedDefect);

    // 判断是否有缺陷
    result.hasDefect = result.totalLoss > m_threshold;

    qDebug() << "缺陷检测结果:";
    qDebug() << "相关性损失:" << result.correlationLoss;
    qDebug() << "卡方损失:" << result.chiSquareLoss;
    qDebug() << "巴氏距离损失:" << result.bhattacharyyaLoss;
    qDebug() << "综合损失:" << result.totalLoss;
    qDebug() << "是否有缺陷:" << result.hasDefect;

    return result;
}

double DefectDetector::compareHSVHistograms(const cv::Mat& hsv1, const cv::Mat& hsv2, int method)
{
    // 直方图参数设置[1,6](@ref)
    int histSize[] = { m_hBins, m_sBins };
    float hRanges[] = { 0, 180 };  // H通道范围: 0-180[2](@ref)
    float sRanges[] = { 0, 256 };  // S通道范围: 0-256
    const float* ranges[] = { hRanges, sRanges };
    int channels[] = { 0, 1 };     // 使用H和S通道，忽略V通道减少光照影响[1](@ref)

    // 计算直方图
    cv::Mat hist1, hist2;
    cv::calcHist(&hsv1, 1, channels, cv::Mat(), hist1, 2, histSize, ranges, true, false);
    cv::calcHist(&hsv2, 1, channels, cv::Mat(), hist2, 2, histSize, ranges, true, false);

    // 归一化直方图[6](@ref)
    cv::normalize(hist1, hist1, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    cv::normalize(hist2, hist2, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

    // 比较直方图[7](@ref)
    return cv::compareHist(hist1, hist2, method);
}

cv::Mat DefectDetector::generateDefectMap(const cv::Mat& normalImage, const cv::Mat& defectImage)
{
    // 转换为灰度图
    cv::Mat grayNormal, grayDefect;
    cv::cvtColor(normalImage, grayNormal, cv::COLOR_BGR2GRAY);
    cv::cvtColor(defectImage, grayDefect, cv::COLOR_BGR2GRAY);

    // 计算绝对差异
    cv::Mat diff;
    cv::absdiff(grayNormal, grayDefect, diff);

    // 应用阈值突出差异区域
    cv::Mat defectMap;
    cv::threshold(diff, defectMap, 30, 255, cv::THRESH_BINARY);

    // 形态学操作去除噪声
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(defectMap, defectMap, cv::MORPH_OPEN, kernel);

    return defectMap;
}
