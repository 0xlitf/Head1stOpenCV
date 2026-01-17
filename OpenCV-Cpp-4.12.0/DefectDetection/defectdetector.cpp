#include "DefectDetector.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImageReader>
#include <QMap>
#include <QString>
#include <opencv2/imgproc.hpp>

DefectDetector::DefectDetector(QObject *parent) : QObject(parent) {}

std::tuple<int, cv::Mat>
DefectDetector::analyzeAndDrawContour(const cv::Mat &inputImage) {
    if (inputImage.empty()) {
        cv::Mat emptyResult(300, 400, CV_8UC3, cv::Scalar(0, 0, 0));
        cv::putText(emptyResult, "输入图像为空", cv::Point(50, 150), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        return std::make_tuple(0, emptyResult);
    }

    // 2. 创建输出图像
    cv::Mat outputImage;
    if (inputImage.channels() == 1) {
        cv::cvtColor(inputImage, outputImage, cv::COLOR_GRAY2BGR);
    } else {
        outputImage = inputImage.clone();
    }

    // 3. 转换为灰度图
    cv::Mat grayImage;
    if (inputImage.channels() == 3) {
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = inputImage.clone();
    }

    // 4. 二值化处理
    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 200, 255, cv::THRESH_BINARY_INV);

    // cv::imshow("binaryImage", binaryImage);

    // 5. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    qDebug() << "contours.size()" << contours.size();

    int contourCount = static_cast<int>(contours.size());

    // 6. 根据轮廓数量选择颜色
    cv::Scalar contourColor;
    if (contourCount > 1) {
        contourColor = cv::Scalar(0, 0, 255); // 红色 (BGR) - 多个轮廓
    } else if (contourCount == 1) {
        contourColor = cv::Scalar(0, 255, 0); // 绿色 (BGR) - 单个轮廓
    } else {
        contourColor = cv::Scalar(255, 255, 255); // 白色 - 无轮廓
    }

    // 7. 绘制轮廓线
    for (size_t i = 0; i < contours.size(); ++i) {
        if (contours[i].empty() || contours[i].size() < 3)
            continue;

        // 绘制轮廓线
        cv::drawContours(outputImage, contours, static_cast<int>(i),
                         contourColor, // 轮廓颜色
                         2,            // 线宽
                         cv::LINE_AA);       // 抗锯齿

        if (bool drawContourInfo = false) {
            // 可选：添加轮廓信息
            double area = cv::contourArea(contours[i]);

            // 计算轮廓中心点
            cv::Moments m = cv::moments(contours[i]);
            if (m.m00 != 0) {
                int centerX = static_cast<int>(m.m10 / m.m00);
                int centerY = static_cast<int>(m.m01 / m.m00);

                std::string info = "C" + std::to_string(i + 1) + " A:" + std::to_string(static_cast<int>(area));

                // 绘制中心点
                cv::circle(outputImage, cv::Point(centerX, centerY), 4, contourColor, -1);

                // 添加文本信息
                cv::Point textPos(centerX + 10, centerY);

                // 绘制文本背景
                int baseline = 0;
                cv::Size textSize = cv::getTextSize(info, cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseline);
                cv::rectangle(outputImage, textPos - cv::Point(2, textSize.height + 2), textPos + cv::Point(textSize.width + 2, 2), cv::Scalar(0, 0, 0), -1);

                cv::putText(outputImage, info, textPos, cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(125, 125, 125), 1);
            }
        }
    }

    std::string statusText;
    if (contourCount > 1) {
        statusText = "检测到 " + std::to_string(contourCount) + " 个轮廓 (红色)";
    } else if (contourCount == 1) {
        statusText = "检测到 1 个轮廓 (绿色)";
    } else {
        statusText = "未检测到轮廓";
    }

    // cv::putText(outputImage, statusText, cv::Point(10, 30),
    //             cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(125, 125, 125), 2);

    return std::make_tuple(contourCount, outputImage);
}

void DefectDetector::setTemplateFolder(const QStringList &descStrs, const QStringList &folderNames) {
    if (descStrs.size() != folderNames.size()) {
        qFatal("setTemplateFolder descStrs.size != folderNames.size");
        return;
    }

    m_huMomentsList.clear();

    for (int i = 0; i < descStrs.size(); ++i) {
        auto &desc = descStrs[i];
        auto &folderName = folderNames[i];
        QDir templateDir(folderName);
        if (!templateDir.exists()) {
            qWarning() << "警告：模板文件夹不存在: " << folderName;
            continue;
        }

        QDir dir(folderName);
        QStringList imageFiles;
        QStringList filters;
        foreach (const QString &format, QImageReader::supportedImageFormats()) {
            filters << "*." + format;
        }

        imageFiles.append(dir.entryList(filters, QDir::Files));
        for (int i = 0; i < imageFiles.size(); ++i) {
            imageFiles[i] = dir.absoluteFilePath(imageFiles[i]);
        }

        for (auto &filename : imageFiles) {
            this->addTemplate(desc, filename);
        }
    }
}
