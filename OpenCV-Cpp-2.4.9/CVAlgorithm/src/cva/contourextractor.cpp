#include "contourextractor.h"
#include <QDebug>

std::vector<std::vector<cv::Point>> ContourExtractor::extractWhiteBackgroundContour(const cv::Mat &inputImage) {
    std::vector<std::vector<cv::Point>> resultContours;

    if (inputImage.empty()) {
        qWarning() << "错误：输入图像为空！";
        return resultContours;
    }

    // 1. 预处理图像
    cv::Mat processed = preprocessForWhiteBackground(inputImage);

    // 2. 自动分割白色背景
    cv::Mat binary = segmentWhiteBackground(processed);

    // 3. 查找并过滤轮廓
    resultContours = findAndFilterContours(binary);

    // 4. 验证结果
    if (resultContours.empty()) {
        qWarning() << "警告：未找到符合条件的轮廓！";
    } else {
        qWarning() << "成功提取 " << resultContours.size() << " 个轮廓";
    }

    return resultContours;
}

std::tuple<std::vector<std::vector<cv::Point>>, cv::Mat> ContourExtractor::extractContourWithDebug(const cv::Mat &inputImage) {
    std::vector<std::vector<cv::Point>> contours;
    cv::Mat debugImage;

    if (inputImage.empty()) {
        return std::make_tuple(contours, debugImage);
    }

    // 1. 预处理
    cv::Mat processed = preprocessForWhiteBackground(inputImage);

    // 2. 分割
    cv::Mat binary = segmentWhiteBackground(processed);
    // cv::imshow("binary", binary);

    // 3. 查找轮廓
    contours = findAndFilterContours(binary);

    // 4. 创建调试图像
    debugImage = createDebugImage(inputImage, processed, binary, contours);

    return std::make_tuple(contours, debugImage);
}

cv::Mat ContourExtractor::preprocessForWhiteBackground(const cv::Mat &input) {
    cv::Mat result;

    // 转换为灰度图
    if (input.channels() == 3) {
        cv::cvtColor(input, result, cv::COLOR_BGR2GRAY);
    } else {
        result = input.clone();
    }

    // 增强对比度：CLAHE（限制对比度自适应直方图均衡化）
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
    clahe->apply(result, result);

    // 高斯模糊去噪
    cv::GaussianBlur(result, result, cv::Size(5, 5), 0);

    return result;
}

cv::Mat ContourExtractor::segmentWhiteBackground(const cv::Mat &grayImage) {
    cv::Mat binary;

    // 方法1：自适应阈值（适合光照不均匀的情况）
    // cv::adaptiveThreshold(grayImage, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, m_adaptiveBlockSize, 2);
    // cv::threshold(grayImage, binary, 240, 255, cv::THRESH_BINARY_INV);

    // 方法2：Otsu阈值（适合直方图双峰明显的情况）
    // double otsuThresh = cv::threshold(grayImage, binary, 0, 255,
    //                                  cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    // qDebug() << "Otsu阈值: " << otsuThresh;

    // cv::imshow("grayImage", grayImage);
    // 方法3：固定阈值（如果背景非常稳定）
    cv::threshold(grayImage, binary, 240, 255, cv::THRESH_BINARY_INV);

    // double minVal, maxVal;
    // cv::Point minLoc, maxLoc; // 位置信息如果不需要，可以忽略

    // cv::minMaxLoc(binary, &minVal, &maxVal, &minLoc, &maxLoc);

    // qDebug() << "Min value: " << minVal << ", Max value: " << maxVal;
    // qDebug() << "Binary image type: " << binary.type() << " (CV_8U is " << CV_8U << ")";

    // 去除小噪声
    // if (m_removeNoise && m_noiseKernelSize > 0) {
    //     cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(m_noiseKernelSize, m_noiseKernelSize));
    //     cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);
    // }

    return binary;
}

std::vector<std::vector<cv::Point>> ContourExtractor::findAndFilterContours(const cv::Mat &binary) {
    std::vector<std::vector<cv::Point>> allContours;
    std::vector<cv::Vec4i> hierarchy;

    auto binaryClone = binary.clone();

    cv::findContours(binaryClone, allContours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> filteredContours;

    for (const auto &contour : allContours) {
        if (contour.size() < 3)
            continue; // 跳过点数太少的轮廓

        double area = cv::contourArea(contour);

        if (area > m_minContourArea) {
            filteredContours.push_back(contour);

            // 可选：输出轮廓信息用于调试
            // cv::Rect bbox = cv::boundingRect(contour);
            // qDebug() << "轮廓面积: " << area
            //           << ", 边界框: " << bbox
            //           << ", 点数: " << contour.size();
        }
    }

    // 按面积从大到小排序（可选）
    std::sort(filteredContours.begin(), filteredContours.end(), [](const std::vector<cv::Point> &c1, const std::vector<cv::Point> &c2) { return cv::contourArea(c1) > cv::contourArea(c2); });

    return filteredContours;
}

cv::Mat ContourExtractor::createDebugImage(const cv::Mat &original, const cv::Mat &processed, const cv::Mat &binary, const std::vector<std::vector<cv::Point>> &contours) {
    cv::Mat debugImage;

    // 创建3行2列的调试图
    std::vector<cv::Mat> rows;

    // 第一行：原始图和灰度图
    cv::Mat row1;
    cv::hconcat(original, convertTo3Channel(processed), row1);
    cv::putText(row1, "original", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
    cv::putText(row1, "processed", cv::Point(original.cols + 10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

    // 第二行：二值化图和轮廓图

    // double minVal, maxVal;
    // cv::Point minLoc, maxLoc; // 位置信息如果不需要，可以忽略

    // cv::minMaxLoc(binary, &minVal, &maxVal, &minLoc, &maxLoc);

    // qDebug() << "Min value: " << minVal << ", Max value: " << maxVal;
    // qDebug() << "Binary image type: " << binary.type() << " (CV_8U is " << CV_8U << ")";

    cv::Mat binaryColor;
    cv::cvtColor(binary, binaryColor, cv::COLOR_GRAY2BGR);

    cv::Mat contourImage = original.clone();
    if (contourImage.channels() == 1) {
        cv::cvtColor(contourImage, contourImage, cv::COLOR_GRAY2BGR);
    }

    cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2);

    // 在轮廓图上标注面积
    for (size_t i = 0; i < contours.size(); ++i) {
        double area = cv::contourArea(contours[i]);
        cv::Moments m = cv::moments(contours[i]);
        if (m.m00 != 0) {
            cv::Point center(m.m10 / m.m00, m.m01 / m.m00);
            std::string text = "Area: " + std::to_string(static_cast<int>(area));
            cv::putText(contourImage, text, center, cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
        }
    }

    cv::Mat row2;
    cv::hconcat(binaryColor, contourImage, row2);
    cv::putText(row2, "binary", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);
    cv::putText(row2, "contours", cv::Point(binaryColor.cols + 10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

    // 垂直拼接
    cv::vconcat(row1, row2, debugImage);

    // 添加总结信息
    std::string summary = "detect " + std::to_string(contours.size()) + " contour (area > " + std::to_string(static_cast<int>(m_minContourArea)) + ")";
    cv::putText(debugImage, summary, cv::Point(10, debugImage.rows - 20), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2);

    return debugImage;
}

cv::Mat ContourExtractor::convertTo3Channel(const cv::Mat &gray) {
    cv::Mat result;
    if (gray.channels() == 1) {
        cv::cvtColor(gray, result, cv::COLOR_GRAY2BGR);
    } else {
        result = gray.clone();
    }
    return result;
}

// 核心：寻找最大轮廓
std::vector<cv::Point> ContourExtractor::findLargestContour(const cv::Mat &src,
                                                            bool isTemplate, int whiteThreshold) {
    cv::Mat thr;
    // 背光图片：物体黑(0)，背景白(255)。
    // 使用 THRESH_BINARY_INV 将物体变成白色(255)，背景变成黑色(0)
    // 这样 findContours 才能正确找到物体
    cv::threshold(src, thr, whiteThreshold, 255, cv::THRESH_BINARY_INV);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thr, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.empty())
        return {};

    // 找到面积最大的轮廓
    double maxArea = 0;
    int maxIdx = -1;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area > maxArea) {
            maxArea = area;
            maxIdx = i;
        }
    }

    if (maxIdx != -1)
        return contours[maxIdx];
    return {};
}
