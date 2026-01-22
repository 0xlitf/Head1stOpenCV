#include "contourdefectdetector.h"

void ContourDefectDetector::setParameters(double contourThreshold, double pixelThreshold, int contourWidth, double minArea) {
    m_contourMatchThreshold = contourThreshold;
    m_pixelDiffThreshold = pixelThreshold;
    m_contourInspectWidth = contourWidth;
    m_minContourArea = minArea;
}

double ContourDefectDetector::matchMat(cv::Mat templateInput, cv::Mat defectInput) {
    if (templateInput.empty() || defectInput.empty()) {
        return -1;
    }

    // 1. 图像预处理
    cv::Mat templateProcessed = preprocessImage(templateInput);
    cv::Mat testProcessed = preprocessImage(defectInput);

    // 2. 轮廓匹配度计算
    double contourMatchScore = calculateContourSimilarity(templateProcessed, testProcessed);

    // 3. 轮廓区域像素差异计算
    double pixelDiffScore = calculateContourZoneDifference(templateProcessed, testProcessed);

    // 4. 综合评分（可以根据需求调整权重）
    double finalScore = 0.7 * contourMatchScore + 0.3 * pixelDiffScore;

    if (m_debugMode) {
        std::cout << "轮廓匹配分数: " << contourMatchScore << ", 像素差异分数: " << pixelDiffScore << ", 最终分数: " << finalScore << std::endl;
    }

    return finalScore;
}

cv::Mat ContourDefectDetector::preprocessImage(const cv::Mat &input) {
    cv::Mat result;

    // 转换为灰度图
    if (input.channels() == 3) {
        cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
    } else {
        result = input.clone();
    }

    // 高斯模糊去噪
    cv::GaussianBlur(result, result, cv::Size(5, 5), 0);

    return result;
}

std::vector<cv::Point> ContourDefectDetector::extractMainContour(const cv::Mat& image) {
    cv::Mat binary;
    // 自适应阈值处理，适合不同光照条件[2](@ref)
    cv::adaptiveThreshold(image, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 找到面积最大的轮廓[1](@ref)
    if (contours.empty()) {
        return std::vector<cv::Point>();
    }

    auto maxContour = *std::max_element(contours.begin(), contours.end(), [](const std::vector<cv::Point>& c1, const std::vector<cv::Point>& c2) { return cv::contourArea(c1) < cv::contourArea(c2); });

    // 过滤太小的轮廓
    if (cv::contourArea(maxContour) < m_minContourArea) {
        return std::vector<cv::Point>();
    }

    return maxContour;
}

double ContourDefectDetector::calculateContourSimilarity(const cv::Mat& templateImg, const cv::Mat& testImg) {
    std::vector<cv::Point> templateContour = extractMainContour(templateImg);
    std::vector<cv::Point> testContour = extractMainContour(testImg);

    if (templateContour.empty() || testContour.empty()) {
        return 1.0; // 返回最大差异
    }

    // 使用Hu矩进行形状匹配[5](@ref)
    cv::Moments templateMoments = cv::moments(templateContour);
    cv::Moments testMoments = cv::moments(testContour);

    cv::Mat templateHu, testHu;
    cv::HuMoments(templateMoments, templateHu);
    cv::HuMoments(testMoments, testHu);

    // 计算形状匹配距离
    double distance = cv::matchShapes(templateContour, testContour, CV_CONTOURS_MATCH_I2, 0);

    return distance;
}

cv::Mat ContourDefectDetector::createContourInspectionMask(const std::vector<cv::Point>& contour, cv::Size imageSize) {
    cv::Mat mask = cv::Mat::zeros(imageSize, CV_8UC1);

    if (contour.empty()) {
        return mask;
    }

    // 绘制轮廓线
    std::vector<std::vector<cv::Point>> contours = {contour};
    cv::drawContours(mask, contours, -1, 255, 1);

    // 形态学膨胀创建检测带[1](@ref)
    if (m_contourInspectWidth > 1) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(m_contourInspectWidth, m_contourInspectWidth));
        cv::dilate(mask, mask, kernel);
    }

    return mask;
}

double ContourDefectDetector::calculateContourZoneDifference(const cv::Mat& templateImg, const cv::Mat& testImg) {
    std::vector<cv::Point> templateContour = extractMainContour(templateImg);
    std::vector<cv::Point> testContour = extractMainContour(testImg);

    if (templateContour.empty() || testContour.empty()) {
        return 1.0;
    }

    // 创建轮廓检测带掩膜
    cv::Mat templateMask = createContourInspectionMask(templateContour, templateImg.size());
    cv::Mat testMask = createContourInspectionMask(testContour, testImg.size());

    // 应用掩膜，只检测轮廓区域
    cv::Mat templateMasked, testMasked;
    templateImg.copyTo(templateMasked, templateMask);
    testImg.copyTo(testMasked, testMask);

    // 确保图像尺寸一致
    if (templateMasked.size() != testMasked.size()) {
        cv::resize(testMasked, testMasked, templateMasked.size());
    }

    // 计算差异[2](@ref)
    cv::Mat diff;
    cv::absdiff(templateMasked, testMasked, diff);

    // 二值化差异图
    cv::Mat thresholdDiff;
    cv::threshold(diff, thresholdDiff, m_pixelDiffThreshold, 255, cv::THRESH_BINARY);

    // 形态学操作去除噪声[1](@ref)
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::morphologyEx(thresholdDiff, thresholdDiff, cv::MORPH_OPEN, kernel);

    // 计算缺陷像素比例
    int totalPixels = cv::countNonZero(templateMask);
    int defectPixels = cv::countNonZero(thresholdDiff);

    double defectRatio = (totalPixels > 0) ? static_cast<double>(defectPixels) / totalPixels : 0.0;

    // 调试显示
    if (m_debugMode) {
        cv::Mat colorDisplay;
        cv::cvtColor(templateImg, colorDisplay, cv::COLOR_GRAY2BGR);

        // 绘制轮廓区域
        std::vector<std::vector<cv::Point>> contours = {templateContour};
        cv::drawContours(colorDisplay, contours, -1, cv::Scalar(0, 255, 0), 2);

        // 绘制差异点
        cv::Mat defectColor;
        cv::cvtColor(thresholdDiff, defectColor, cv::COLOR_GRAY2BGR);
        defectColor.setTo(cv::Scalar(0, 0, 255), thresholdDiff);

        cv::addWeighted(colorDisplay, 0.7, defectColor, 0.3, 0, colorDisplay);

        cv::imshow("Contour Inspection Result", colorDisplay);
        cv::waitKey(1);
    }

    return defectRatio;
}
