#include "DefectDetector.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImageReader>
#include <QMap>
#include <QString>
#include <opencv2/imgproc.hpp>
#include "minimumbounding.h"
#include "bgr2hsvconverter.h"

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

void DefectDetector::addTemplate(const QString &desc,
                                   const QString &fileName) {

    // 读取灰度图
    auto templateImg = cv::imread(fileName.toStdString());
    if (templateImg.empty()) {
        qDebug() << "templateImg is empty: " << fileName;
        emit errorOccured(IMAGE_LOAD_FAILED,
                          QString("templateImg is empty: %1").arg(fileName));
        return;
    } else {
        // auto folderBaseName = FileUtils::getFolderBaseName(fileName);

        cv::threshold(templateImg, templateImg, m_whiteThreshold, 255,
                      cv::THRESH_BINARY);

        cv::Mat tInput = templateImg.clone();
        cv::pyrDown(tInput, tInput);

        this->addTemplateIntoMap(desc, fileName, tInput);
    }
}

void DefectDetector::addTemplateIntoMap(const QString &desc,
                                          const QString &fileName,
                                          cv::Mat tInput) {

    auto tuple = std::make_tuple(desc, fileName, tInput);
    m_huMomentsList.append(tuple);
}

double DefectDetector::fullMatchImage(const QString &fileName) {
    double defectScoreResult{-1};

    if (fileName.isEmpty()) {
        qDebug() << "matchImage fileName isEmpty";
        emit errorOccured(IMAGE_LOAD_FAILED,
                          QString("matchImage fileName isEmpty: %1").arg(fileName));
        return -1;
    }

    emit sendLog(QString("matchImage: %1").arg(fileName));

    cv::Mat imageMat = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
    if (imageMat.empty())
        return defectScoreResult;

    return this->fullMatchMat(imageMat);
}

double DefectDetector::fullMatchMat(cv::Mat sceneImg) {
    QElapsedTimer timer;
    timer.start();

    QList<double> results;
    for (int i = 0; i < m_huMomentsList.size(); ++i) {
        std::tuple<QString, QString, cv::Mat> templateTuple = m_huMomentsList[i];
        QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<1>(templateTuple);
        cv::Mat templateInput = std::get<2>(templateTuple);

        double defectScore = this->matchMat(templateInput, sceneImg);
        if (defectScore > 0) {
            results.append(defectScore);
        }
    }

    double defectScoreResult{9999};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < defectScoreResult) {
                defectScoreResult = value;
            }
        }
        qDebug() << results << "最小值:" << defectScoreResult;
    }

    return defectScoreResult;
}

double DefectDetector::matchMat(cv::Mat templateInput, cv::Mat defectInput) {
    if (templateInput.empty() || defectInput.empty()) {
        QMessageBox::warning(nullptr, "错误", "请先加载正常图像和缺陷图像!");
        return -1;
    }

    MinimumBounding mini;

    cv::Mat tInput = templateInput.clone();
    cv::Mat dInput = defectInput.clone();

    cv::pyrDown(tInput, tInput);
    cv::pyrDown(dInput, dInput);

    // blur GaussianBlur medianBlur bilateralFilter
    // cv::blur(tInput, tInput, cv::Size(3, 3));
    // cv::blur(dInput, dInput, cv::Size(3, 3));
    cv::GaussianBlur(tInput, tInput, cv::Size(5, 5), 0, 0);
    cv::GaussianBlur(dInput, dInput, cv::Size(5, 5), 0, 0);
    // cv::medianBlur(tInput, tInput, 5);
    // cv::medianBlur(dInput, dInput, 5);
    // cv::bilateralFilter(tInput, tInput, 9, 50, 10);
    // cv::bilateralFilter(dInput, dInput, 9, 50, 10);

    cv::imshow("m_normalImage origin", tInput);
    cv::imshow("m_defectImage origin", dInput);

    if (m_useHSV) {
        BGR2HSVConverter cvt;
        tInput = cvt.convertBGR2HSV(tInput);
        dInput = cvt.convertBGR2HSV(dInput);
    }

    cv::imshow("m_normalImage hsv", tInput);
    cv::imshow("m_defectImage hsv", dInput);

    tInput = mini.removeOuterBorder(tInput, m_removeOuterBorderThickness);
    dInput = mini.removeOuterBorder(dInput, m_removeOuterBorderThickness);

    cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0, cv::INTER_LINEAR);

    tInput = mini.fillCenterWithWhite(tInput, m_detectThickness);
    dInput = mini.fillCenterWithWhite(dInput, m_detectThickness);

    std::vector<cv::Mat> thsvChannels;
    cv::split(tInput, thsvChannels);
    cv::Mat thChannel = thsvChannels[0]; // H通道
    cv::Mat tsChannel = thsvChannels[1]; // S通道
    cv::Mat tvChannel = thsvChannels[2]; // V通道

    std::vector<cv::Mat> dhsvChannels;
    cv::split(dInput, dhsvChannels);
    cv::Mat dhChannel = dhsvChannels[0]; // H通道
    cv::Mat dsChannel = dhsvChannels[1]; // S通道
    cv::Mat dvChannel = dhsvChannels[2]; // V通道

    if (bool showConcat = false) {
        cv::Mat tH_BGR, tS_BGR, tV_BGR, dH_BGR, dS_BGR, dV_BGR;
        cv::cvtColor(thChannel, tH_BGR, cv::COLOR_GRAY2BGR);
        cv::cvtColor(tsChannel, tS_BGR, cv::COLOR_GRAY2BGR);
        cv::cvtColor(tvChannel, tV_BGR, cv::COLOR_GRAY2BGR);
        cv::cvtColor(dhChannel, dH_BGR, cv::COLOR_GRAY2BGR);
        cv::cvtColor(dsChannel, dS_BGR, cv::COLOR_GRAY2BGR);
        cv::cvtColor(dvChannel, dV_BGR, cv::COLOR_GRAY2BGR);
        cv::putText(tH_BGR, "T-H", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
        cv::putText(tS_BGR, "T-S", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        cv::putText(tV_BGR, "T-V", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
        cv::putText(dH_BGR, "D-H", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
        cv::putText(dS_BGR, "D-S", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        cv::putText(dV_BGR, "D-V", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

        // 3. 分别拼接左右两侧（每侧3个通道垂直堆叠）
        cv::Mat leftCol, rightCol;
        cv::vconcat(std::vector<cv::Mat>{tH_BGR, tS_BGR, tV_BGR}, leftCol);  // 左侧垂直拼接
        cv::vconcat(std::vector<cv::Mat>{dH_BGR, dS_BGR, dV_BGR}, rightCol); // 右侧垂直拼接

        // 4. 将左右两侧水平拼接为最终图像
        cv::Mat concatResult;
        cv::hconcat(leftCol, rightCol, concatResult);

        // 5. 显示最终拼接结果
        cv::imshow("HSV Channels Comparison (Left: Template, Right: Detection)", concatResult);
    }

    cv::Mat sdiff;
    cv::absdiff(tsChannel, dsChannel, sdiff);

    cv::Mat vdiff;
    cv::absdiff(tvChannel, dvChannel, vdiff);

    // cv::Mat combinedDiff;
    // cv::add(sdiff, vdiff, combinedDiff);
    // cv::imshow("combinedDiff", combinedDiff);

    cv::Mat grayDiff;
    if (vdiff.channels() == 3) {
        cv::cvtColor(vdiff, grayDiff, cv::COLOR_BGR2GRAY);
    } else {
        grayDiff = vdiff;
    }

    cv::Mat thresholdDiff;
    // 1. 阈值化处理，得到二值图像
    cv::threshold(grayDiff, thresholdDiff, m_whiteThreshold, 255, cv::THRESH_BINARY);

    // 2. 形态学开运算去除小噪点
    int kernalSize = 3;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernalSize, kernalSize)); // 创建3x3矩形结构元素
    cv::morphologyEx(thresholdDiff, thresholdDiff, cv::MORPH_OPEN,
                     kernel); // 执行开运算

    // 3. 统计过滤后的白色像素点
    int whitePixelCount = cv::countNonZero(thresholdDiff);
    qDebug() << "过滤后白色像素点的个数为: " << whitePixelCount;
    // cv::imshow("thresholdDiff", thresholdDiff);

    if (bool showDiff = true) {
        cv::Mat concatDiffResult;

        cv::vconcat(std::vector<cv::Mat>{vdiff, grayDiff, thresholdDiff}, concatDiffResult);
        cv::imshow("concatDiffResult", concatDiffResult);
    }

    return whitePixelCount;
}

double DefectDetector::scoreThreshold() const {
    return m_scoreThreshold;
}

void DefectDetector::setScoreThreshold(double newScoreThreshold) {
    m_scoreThreshold = newScoreThreshold;
}

int DefectDetector::whiteThreshold() const {
    return m_whiteThreshold;
}

void DefectDetector::setWhiteThreshold(int newWhiteThreshold) {
    m_whiteThreshold = newWhiteThreshold;
}
