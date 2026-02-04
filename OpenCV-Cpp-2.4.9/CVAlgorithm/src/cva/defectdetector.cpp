#include "DefectDetector.h"
#include "bgr2hsvconverter.h"
#include "contourextractor.h"
#include "minimumbounding.h"
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImageReader>
#include <QMap>
#include <QString>

DefectDetector::DefectDetector(QObject *parent) : QObject(parent) {}

cv::Mat DefectDetector::createRingEdgeMask(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeStartWidth, int edgeEndWidth) {
    if (contours.empty()) {
        return cv::Mat::zeros(inputImage.size(), CV_8UC1);
    }

    cv::Mat mask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
    cv::Mat outerMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
    cv::Mat innerMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
    cv::Mat edgeMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);

    // 1. 绘制原始轮廓（填充）
    cv::drawContours(mask, contours, -1, cv::Scalar(255), CV_FILLED);

    // 2. 创建外层边界（向外扩展，可选）
    cv::Mat kernelOuter = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2 * edgeStartWidth + 1, 2 * edgeStartWidth + 1));
    cv::erode(mask, outerMask, kernelOuter);

    // 3. 创建内层边界（向内腐蚀）
    cv::Mat kernelInner = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2 * edgeEndWidth + 1, 2 * edgeEndWidth + 1));
    cv::erode(mask, innerMask, kernelInner);

    // 4. 环形区域 = 外层边界 - 内层边界
    cv::subtract(outerMask, innerMask, edgeMask);

    // 5. 优化：只保留与原始轮廓相交的环形区域
    cv::Mat finalMask;
    cv::bitwise_and(edgeMask, mask, finalMask);

    return finalMask;
}

cv::Mat DefectDetector::processRingEdge(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeStartWidth, int edgeEndWidth) {
    if (inputImage.empty() || contours.empty()) {
        return cv::Mat();
    }

    // 创建环形边缘掩膜
    cv::Mat edgeMask = createRingEdgeMask(inputImage, contours, edgeStartWidth, edgeEndWidth);

    // 可视化边缘掩膜
    if (m_debugImageFlag) {
        cv::imshow("edgeMask", edgeMask);
    }

    // 创建结果图像：只保留环形边缘区域，其他区域设为白色
    cv::Mat result = cv::Mat(inputImage.size(), inputImage.type(), cv::Scalar(255, 255, 255));
    inputImage.copyTo(result, edgeMask);

    return result;
}

cv::Mat DefectDetector::createOuterEdgeMask(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeWidth) {
    if (contours.empty()) {
        return cv::Mat::zeros(inputImage.size(), CV_8UC1);
    }

    cv::Mat mask = cv::Mat::zeros(inputImage.size(), CV_8UC1);
    cv::Mat edgeMask = cv::Mat::zeros(inputImage.size(), CV_8UC1);

    // 1. 绘制原始轮廓（填充）
    cv::drawContours(mask, contours, -1, cv::Scalar(255), CV_FILLED);

    // 2. 创建腐蚀后的内部区域
    cv::Mat innerMask = mask.clone();
    if (edgeWidth > 0) {
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2 * edgeWidth + 1, 2 * edgeWidth + 1));
        cv::erode(mask, innerMask, kernel);
    }

    // 3. 外边缘掩膜 = 原始掩膜 - 内部掩膜（环形区域）
    cv::subtract(mask, innerMask, edgeMask);

    return edgeMask;
}

cv::Mat DefectDetector::processOuterEdge(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeWidth) {
    if (inputImage.empty() || contours.empty()) {
        return cv::Mat();
    }

    // 克隆原图用于显示
    cv::Mat displayMat = inputImage.clone();
    if (displayMat.channels() == 1) {
        cv::cvtColor(displayMat, displayMat, cv::COLOR_GRAY2BGR);
    }

    // 创建外边缘掩膜
    cv::Mat edgeMask = createOuterEdgeMask(inputImage, contours, edgeWidth);
    cv::imshow("edgeMask", edgeMask);

    // 创建彩色掩膜（红色边缘）
    cv::Mat colorEdge;
    cv::cvtColor(edgeMask, colorEdge, cv::COLOR_GRAY2BGR);
    colorEdge.setTo(cv::Scalar(0, 0, 0), edgeMask); // BGR: 红色

    // 可选：创建绿色原始轮廓线
    cv::Mat contourImage = inputImage.clone();
    if (contourImage.channels() == 1) {
        cv::cvtColor(contourImage, contourImage, cv::COLOR_GRAY2BGR);
    }
    cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2);

    // 将边缘掩膜叠加到原图
    // cv::addWeighted(displayMat, 0.5, colorEdge, 0.5, 0, displayMat);

    cv::Mat edgeMask_8u;
    edgeMask.convertTo(edgeMask_8u, CV_8UC1);

    // cv::Mat whiteBackground = cv::Mat::ones(displayMat.size(),
    // displayMat.type()) * 255;
    cv::Mat whiteBackground = cv::Mat(displayMat.size(), displayMat.type(), cv::Scalar(255, 255, 255));
    cv::Mat result;
    displayMat.copyTo(result,
                      edgeMask);               // 将displayMat中边缘掩膜对应的区域复制到result
    whiteBackground.copyTo(result, ~edgeMask); // 将非边缘区域设置为白色

    return result;
}

cv::Mat DefectDetector::displayOuterEdge(const cv::Mat &inputImage, const std::vector<std::vector<cv::Point>> &contours, int edgeWidth) {
    if (inputImage.empty() || contours.empty()) {
        return cv::Mat();
    }

    // 克隆原图用于显示
    cv::Mat displayMat = inputImage.clone();
    if (displayMat.channels() == 1) {
        cv::cvtColor(displayMat, displayMat, cv::COLOR_GRAY2BGR);
    }

    // 创建外边缘掩膜
    cv::Mat edgeMask = createOuterEdgeMask(inputImage, contours, edgeWidth);
    cv::imshow("edgeMask", edgeMask);

    // 创建彩色掩膜（红色边缘）
    cv::Mat colorEdge;
    cv::cvtColor(edgeMask, colorEdge, cv::COLOR_GRAY2BGR);
    colorEdge.setTo(cv::Scalar(0, 0, 0), edgeMask); // BGR: 红色

    // 可选：创建绿色原始轮廓线
    cv::Mat contourImage = inputImage.clone();
    if (contourImage.channels() == 1) {
        cv::cvtColor(contourImage, contourImage, cv::COLOR_GRAY2BGR);
    }
    cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2);

    // 将边缘掩膜叠加到原图
    // cv::addWeighted(displayMat, 0.5, colorEdge, 0.5, 0, displayMat);

    cv::Mat edgeMask_8u;
    edgeMask.convertTo(edgeMask_8u, CV_8UC1);

    // cv::Mat whiteBackground = cv::Mat::ones(displayMat.size(),
    // displayMat.type()) * 255;
    cv::Mat whiteBackground = cv::Mat(displayMat.size(), displayMat.type(), cv::Scalar(255, 255, 255));
    cv::Mat result;
    displayMat.copyTo(result,
                      edgeMask);               // 将displayMat中边缘掩膜对应的区域复制到result
    whiteBackground.copyTo(result, ~edgeMask); // 将非边缘区域设置为白色

    cv::imshow("Result", result);

    // 显示对比图
    cv::Mat comparison;
    qDebug() << "contourImage" << contourImage.cols << contourImage.rows;
    qDebug() << "displayMat" << displayMat.cols << displayMat.rows;
    cv::hconcat(contourImage, result, comparison);

    // 添加文字说明
    std::string edgeText = "Edge Width: " + std::to_string(edgeWidth) + " pixels";
    cv::putText(comparison, "Original Contour (Green)", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
    cv::putText(comparison, "Edge Zone (Red)", cv::Point(contourImage.cols + 10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
    cv::putText(comparison, edgeText, cv::Point(10, comparison.rows - 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 2);

    cv::imshow("Outer Edge Detection Zone", comparison);

    return result;
}

void DefectDetector::setTemplateFolder(const QStringList &descStrs, const QStringList &folderNames) {
    if (descStrs.size() != folderNames.size()) {
        qFatal("setTemplateFolder descStrs.size != folderNames.size");
        return;
    }

    m_templateList.clear();

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

void DefectDetector::setInputMat(cv::Mat inputMat) {
    if (inputMat.empty()) {
        qDebug() << "DefectDetector::setInputMat, inputMat is empty: ";
        emit errorOccured(IMAGE_LOAD_FAILED, QString("inputMat is empty"));
        return;
    }

    cv::Mat dInput = inputMat.clone();
    m_inputMat = m_mini.findAndCropObject(dInput);

    m_inputMatContour = m_extractor.findLargestContour(m_inputMat);
    m_inputMatArea = cv::contourArea(m_inputMatContour);

    m_corners = m_cornerSplitter.splitCorners(m_inputMat);
    std::vector<cv::Point> ct0 = m_extractor.findLargestContour(std::get<0>(m_corners));
    std::vector<cv::Point> ct1 = m_extractor.findLargestContour(std::get<1>(m_corners));
    std::vector<cv::Point> ct2 = m_extractor.findLargestContour(std::get<2>(m_corners));
    std::vector<cv::Point> ct3 = m_extractor.findLargestContour(std::get<3>(m_corners));
    m_subContours = std::make_tuple(ct0, ct1, ct2, ct3);

    cv::imshow("0", std::get<0>(m_corners));
    cv::imshow("1", std::get<1>(m_corners));
    cv::imshow("2", std::get<2>(m_corners));
    cv::imshow("3", std::get<3>(m_corners));

    double area0 = cv::contourArea(ct0);
    double area1 = cv::contourArea(ct1);
    double area2 = cv::contourArea(ct2);
    double area3 = cv::contourArea(ct3);
    m_subContourAreas = std::make_tuple(area0, area1, area2, area3);
}

void DefectDetector::addTemplate(const QString &desc, const QString &fileName) {
    // 读取灰度图
    auto templateImg = cv::imread(fileName.toStdString());
    if (templateImg.empty()) {
        qDebug() << "templateImg is empty: " << fileName;
        emit errorOccured(IMAGE_LOAD_FAILED, QString("templateImg is empty: %1").arg(fileName));
        return;
    } else {
        cv::Mat tInput = templateImg.clone();
        std::vector<cv::Point> tInputContour = m_extractor.findLargestContour(tInput);
        double tInputArea = cv::contourArea(tInputContour);

        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = m_cornerSplitter.splitCorners(tInput);
        std::vector<cv::Point> ct0 = m_extractor.findLargestContour(std::get<0>(corners));
        std::vector<cv::Point> ct1 = m_extractor.findLargestContour(std::get<1>(corners));
        std::vector<cv::Point> ct2 = m_extractor.findLargestContour(std::get<2>(corners));
        std::vector<cv::Point> ct3 = m_extractor.findLargestContour(std::get<3>(corners));
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::make_tuple(ct0, ct1, ct2, ct3);

        double area0 = cv::contourArea(ct0);
        double area1 = cv::contourArea(ct1);
        double area2 = cv::contourArea(ct2);
        double area3 = cv::contourArea(ct3);
        std::tuple<double, double, double, double> subContourAreas = std::make_tuple(area0, area1, area2, area3);


        this->addTemplateIntoMap(desc, fileName, tInput, tInputContour, tInputArea, corners, subContours, subContourAreas);
    }
}

void DefectDetector::addTemplateIntoMap(const QString &desc,
                                        const QString &fileName,
                                        cv::Mat tInput,
                                        std::vector<cv::Point> tInputContour,
                                        double tInputArea,
                                        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners,
                                        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours,
                                        std::tuple<double, double, double, double> subContourAreas) {
    auto tuple = std::make_tuple(desc, fileName, tInput, tInputContour, tInputArea, corners, subContours, subContourAreas);
    m_templateList.append(tuple);
}

std::tuple<bool, double> DefectDetector::p0_matchArea() {
    double inputMatArea = m_inputMatArea;

    QElapsedTimer timer;
    timer.start();

    QList<double> results;
    for (int i = 0; i < m_templateList.size(); ++i) {
        auto templateTuple = m_templateList[i];

        QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<1>(templateTuple);
        cv::Mat templateInput = std::get<2>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<3>(templateTuple);
        double tInputArea = std::get<4>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<5>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<6>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<7>(templateTuple);

        double areaDiff = std::abs(tInputArea - inputMatArea) / tInputArea;
        if (areaDiff >= 0) {
            results.append(areaDiff);
        }
    }

    double minResult{1};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }
        qDebug() << "p0_matchArea" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
    }

    return std::make_tuple((minResult < m_overallAreaThreshold), minResult);
}

std::tuple<bool, double> DefectDetector::p1_matchShapes() {
    std::vector<cv::Point> inputMatContour = m_inputMatContour;
    QElapsedTimer timer;
    timer.start();

    QList<double> results;
    for (int i = 0; i < m_templateList.size(); ++i) {
        auto templateTuple = m_templateList[i];

        QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<1>(templateTuple);
        cv::Mat templateInput = std::get<2>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<3>(templateTuple);
        double tInputArea = std::get<4>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<5>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<6>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<7>(templateTuple);

        double defectScore = cv::matchShapes(tInputContour, inputMatContour, CV_CONTOURS_MATCH_I1, 0.0);
        if (defectScore >= 0) {
            results.append(defectScore);
        }
    }

    double minResult{1.};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }
        qDebug() << "p1_matchShapes" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
    }

    return std::make_tuple((minResult < m_overallShapeThreshold), minResult);
}

std::tuple<bool, double> DefectDetector::p2_matchSubAreas() {
    std::tuple<double, double, double, double> inputContourAreas = m_subContourAreas;
    QElapsedTimer timer;
    timer.start();

    QList<double> results;
    for (int i = 0; i < m_templateList.size(); ++i) {
        auto templateTuple = m_templateList[i];

        QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<1>(templateTuple);
        cv::Mat templateInput = std::get<2>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<3>(templateTuple);
        double tInputArea = std::get<4>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<5>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<6>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<7>(templateTuple);

        double areaDiff0 = std::abs(std::get<0>(subContourAreas) - std::get<0>(inputContourAreas)) / std::get<0>(subContourAreas);
        double areaDiff1 = std::abs(std::get<1>(subContourAreas) - std::get<1>(inputContourAreas)) / std::get<1>(subContourAreas);
        double areaDiff2 = std::abs(std::get<2>(subContourAreas) - std::get<2>(inputContourAreas)) / std::get<2>(subContourAreas);
        double areaDiff3 = std::abs(std::get<3>(subContourAreas) - std::get<3>(inputContourAreas)) / std::get<3>(subContourAreas);

        QList<double> areaDiffs;
        areaDiffs << areaDiff0 << areaDiff1 << areaDiff2 << areaDiff3;
        qDebug() << "p2_matchSubAreas areaDiffs" << areaDiffs;
        double areaDiffMax = std::max({areaDiff0, areaDiff1, areaDiff2, areaDiff3});
        if (areaDiffMax >= 0) {
            results.append(areaDiffMax);
        }
    }

    double minResult{1};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }
        qDebug() << "p2_matchSubAreas" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
    }

    return std::make_tuple((minResult < m_subAreaThreshold), minResult);
}

std::tuple<bool, double> DefectDetector::p3_matchSubShapes() {
    std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> inputCornerContours = m_subContours;
    QElapsedTimer timer;
    timer.start();

    QList<double> results;
    for (int i = 0; i < m_templateList.size(); ++i) {
        auto templateTuple = m_templateList[i];

        QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<1>(templateTuple);
        cv::Mat templateInput = std::get<2>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<3>(templateTuple);
        double tInputArea = std::get<4>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<5>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<6>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<7>(templateTuple);

        double defectScore0 = cv::matchShapes(std::get<0>(subContours), std::get<0>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);
        double defectScore1 = cv::matchShapes(std::get<1>(subContours), std::get<1>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);
        double defectScore2 = cv::matchShapes(std::get<2>(subContours), std::get<2>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);
        double defectScore3 = cv::matchShapes(std::get<3>(subContours), std::get<3>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);

        QList<double> defectScores;
        defectScores << defectScore0 << defectScore1 << defectScore2 << defectScore3;
        qDebug() << "p3_matchSubShapes matchShapes" << defectScores;
        double defectScoreMax = std::max({defectScore0, defectScore1, defectScore2, defectScore3});
        if (defectScoreMax >= 0) {
            results.append(defectScoreMax);
        }
    }

    double minResult{1.};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }
        qDebug() << "p3_matchSubShapes" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
    }

    return std::make_tuple((minResult < m_subShapeThreshold), minResult);
}

std::tuple<bool, double> DefectDetector::p4_fullMatchMatPixel() {
    cv::Mat inputImg = m_inputMat;
    QElapsedTimer timer;
    timer.start();

    cv::Mat dInput = inputImg.clone();
    // cv::pyrDown(dInput, dInput);

    QList<double> results;
    for (int i = 0; i < m_templateList.size(); ++i) {
        auto templateTuple = m_templateList[i];

        QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<1>(templateTuple);
        cv::Mat templateInput = std::get<2>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<3>(templateTuple);
        double tInputArea = std::get<4>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<5>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<6>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<7>(templateTuple);


        cv::Mat tInput = templateInput.clone();
        // cv::pyrDown(tInput, tInput);

        for (int i = 0; i < 2; ++i) {
            int blurCoreSize = 3;
            // blur GaussianBlur medianBlur bilateralFilter
            switch (0) {
            case 0: { // blur
                cv::blur(tInput, tInput, cv::Size(3, 3));
                cv::blur(dInput, dInput, cv::Size(3, 3));
            } break;
            case 1: { // GaussianBlur
                cv::GaussianBlur(tInput, tInput, cv::Size(blurCoreSize, blurCoreSize), 0, 0);
                cv::GaussianBlur(dInput, dInput, cv::Size(blurCoreSize, blurCoreSize), 0, 0);
            } break;
            case 2: { // medianBlur
                cv::medianBlur(tInput, tInput, 5);
                cv::medianBlur(dInput, dInput, 5);
            } break;
            case 3: { // bilateralFilter
                cv::bilateralFilter(tInput, tInput, 9, 50, 10);
                cv::bilateralFilter(dInput, dInput, 9, 50, 10);
            } break;
            }
        }

        cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0, cv::INTER_LINEAR);

        auto tContour = m_extractor.findContours(tInput);
        auto dContour = m_extractor.findContours(dInput);

        cv::Mat tEdge = this->processRingEdge(tInput, tContour, m_outterWidth, m_innerWidth);
        cv::Mat dEdge = this->processRingEdge(dInput, tContour, m_outterWidth, m_innerWidth);

        cv::imshow("tEdge", tEdge);
        cv::imshow("dEdge", dEdge);

        double defectScore = this->matchMatPixel(tEdge, dEdge);
        qDebug() << "matchMatPixel defectScore" << defectScore << ", matchMat elapsed:" << timer.nsecsElapsed();

        if (defectScore >= 0) {
            results.append(defectScore);
        }
        break;
    }

    double minResult{9999.};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }
        qDebug() << "p4_fullMatchMatPixel" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
    }

    return std::make_tuple((minResult < m_scoreThreshold), minResult);
}

double DefectDetector::matchMatPixel(cv::Mat templateInput, cv::Mat defectInput) {
    if (templateInput.empty() || defectInput.empty()) {
        QMessageBox::warning(nullptr, "错误", "请先加载正常图像和缺陷图像!");
        return -1;
    }

    cv::Mat tInput = templateInput.clone();
    cv::Mat dInput = defectInput.clone();

    cv::pyrDown(tInput, tInput);
    cv::pyrDown(dInput, dInput);

    if (m_useHSV) {
        BGR2HSVConverter cvt;
        tInput = cvt.convertBGR2HSV(tInput);
        dInput = cvt.convertBGR2HSV(dInput);
    }

    if (m_debugImageFlag) {
        cv::imshow("tEdge hsv", tInput);
        cv::imshow("dEdge hsv", dInput);
    }

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

        cv::Mat leftCol, rightCol;
        cv::vconcat(std::vector<cv::Mat>{tH_BGR, tS_BGR, tV_BGR},
                    leftCol); // 左侧垂直拼接
        cv::vconcat(std::vector<cv::Mat>{dH_BGR, dS_BGR, dV_BGR},
                    rightCol); // 右侧垂直拼接

        cv::Mat concatResult;
        cv::hconcat(leftCol, rightCol, concatResult);

        if (m_debugImageFlag) {
            cv::imshow("HSV Channels Comparison (Left: Template, Right: Detection)", concatResult);
        }
    }

    cv::Mat sdiff;
    cv::absdiff(tsChannel, dsChannel, sdiff);

    cv::Mat vdiff;
    cv::absdiff(tvChannel, dvChannel, vdiff);

    cv::Mat grayDiff;

    if (bool useHSVDiff = true) {
        if (vdiff.channels() == 3) {
            cv::cvtColor(vdiff, grayDiff, cv::COLOR_BGR2GRAY);
        } else {
            grayDiff = vdiff;
        }
    } else {
        cv::Mat grayTemplate, grayTest;
        cv::cvtColor(tInput, grayTemplate, cv::COLOR_BGR2GRAY);
        cv::cvtColor(dInput, grayTest, cv::COLOR_BGR2GRAY);

        cv::imshow("grayTemplate", grayTemplate);
        cv::imshow("grayTest", grayTest);

        // 2. 计算绝对差异
        cv::absdiff(grayTemplate, grayTest, grayDiff);
        cv::imshow("grayDiff", grayDiff);
    }

    cv::Mat thresholdDiff;
    cv::threshold(grayDiff, thresholdDiff, m_whiteThreshold, 255, cv::THRESH_BINARY);

    int kernalSize = 3; // 从3改变到5，可以去掉矩形物料diff边缘的噪声
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
    cv::Size(kernalSize, kernalSize)); cv::morphologyEx(thresholdDiff,
    thresholdDiff, cv::MORPH_OPEN,
                     kernel);

    int whitePixelCount = cv::countNonZero(thresholdDiff);

    if (m_debugImageFlag) {
        cv::Mat concatDiffResult;

        cv::vconcat(std::vector<cv::Mat>{vdiff, grayDiff, thresholdDiff}, concatDiffResult);
        cv::imshow("concatDiffResult", concatDiffResult);
    }

    return whitePixelCount;
}

double DefectDetector::fullMatchImagePixel(const QString &fileName) {
    double defectScoreResult{-1};

    if (fileName.isEmpty()) {
        qDebug() << "matchImage fileName isEmpty";
        emit errorOccured(IMAGE_LOAD_FAILED, QString("matchImage fileName isEmpty: %1").arg(fileName));
        return -1;
    }

    emit sendLog(QString("matchImage: %1").arg(fileName));

    cv::Mat imageMat = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
    if (imageMat.empty())
        return defectScoreResult;

    return this->fullMatchMatPixel(imageMat);
}

double DefectDetector::fullMatchMatPixel(cv::Mat inputImg) {
    QElapsedTimer timer;
    timer.start();

    QList<double> results;
    for (int i = 0; i < m_templateList.size(); ++i) {
        auto templateTuple = m_templateList[i];
        QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<1>(templateTuple);
        cv::Mat templateInput = std::get<2>(templateTuple);

        double defectScore = this->matchMat(templateInput, inputImg);
        if (defectScore >= 0) {
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

    for (int i = 0; i < 2; ++i) {
        int blurCoreSize = 3;
        // blur GaussianBlur medianBlur bilateralFilter
        switch (1) {
        case 0: { // blur
            cv::blur(tInput, tInput, cv::Size(3, 3));
            cv::blur(dInput, dInput, cv::Size(3, 3));
        } break;
        case 1: { // GaussianBlur
            cv::GaussianBlur(tInput, tInput, cv::Size(blurCoreSize,
            blurCoreSize), 0, 0); cv::GaussianBlur(dInput, dInput,
            cv::Size(blurCoreSize, blurCoreSize), 0, 0);
        } break;
        case 2: { // medianBlur
            cv::medianBlur(tInput, tInput, 5);
            cv::medianBlur(dInput, dInput, 5);
        } break;
        case 3: { // bilateralFilter
            cv::bilateralFilter(tInput, tInput, 9, 50, 10);
            cv::bilateralFilter(dInput, dInput, 9, 50, 10);
        } break;
        }
    }

    // if (m_debugImageFlag) {
    //     cv::imshow("m_normalImage origin", tInput);
    //     cv::imshow("m_defectImage origin", dInput);
    // }

    if (m_useHSV) {
        BGR2HSVConverter cvt;
        tInput = cvt.convertBGR2HSV(tInput);
        dInput = cvt.convertBGR2HSV(dInput);
    }

    // resize转移到函数外部
    cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0,
    cv::INTER_LINEAR);

    if (m_debugImageFlag) {
        cv::imshow("tEdge hsv", tInput);
        cv::imshow("dEdge hsv", dInput);
    }

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

        cv::Mat leftCol, rightCol;
        cv::vconcat(std::vector<cv::Mat>{tH_BGR, tS_BGR, tV_BGR},
                    leftCol); // 左侧垂直拼接
        cv::vconcat(std::vector<cv::Mat>{dH_BGR, dS_BGR, dV_BGR},
                    rightCol); // 右侧垂直拼接

        cv::Mat concatResult;
        cv::hconcat(leftCol, rightCol, concatResult);

        if (m_debugImageFlag) {
            cv::imshow("HSV Channels Comparison (Left: Template, Right: Detection)", concatResult);
        }
    }

    cv::Mat sdiff;
    cv::absdiff(tsChannel, dsChannel, sdiff);

    cv::Mat vdiff;
    cv::absdiff(tvChannel, dvChannel, vdiff);

    cv::Mat grayDiff;

    if (bool useHSVDiff = true) {
        if (vdiff.channels() == 3) {
            cv::cvtColor(vdiff, grayDiff, cv::COLOR_BGR2GRAY);
        } else {
            grayDiff = vdiff;
        }
    } else {
        cv::Mat grayTemplate, grayTest;
        cv::cvtColor(tInput, grayTemplate, cv::COLOR_BGR2GRAY);
        cv::cvtColor(dInput, grayTest, cv::COLOR_BGR2GRAY);

        cv::imshow("grayTemplate", grayTemplate);
        cv::imshow("grayTest", grayTest);

        // 2. 计算绝对差异
        cv::absdiff(grayTemplate, grayTest, grayDiff);
        cv::imshow("grayDiff", grayDiff);
    }

    cv::Mat thresholdDiff;
    cv::threshold(grayDiff, thresholdDiff, m_whiteThreshold, 255, cv::THRESH_BINARY);

    // int kernalSize = 5; // 从3改变到5，可以去掉矩形物料diff边缘的噪声
    // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
    // cv::Size(kernalSize, kernalSize)); cv::morphologyEx(thresholdDiff,
    // thresholdDiff, cv::MORPH_OPEN,
    //                  kernel);

    int whitePixelCount = cv::countNonZero(thresholdDiff);

    if (m_debugImageFlag) {
        cv::Mat concatDiffResult;

        cv::vconcat(std::vector<cv::Mat>{vdiff, grayDiff, thresholdDiff}, concatDiffResult);
        cv::imshow("concatDiffResult", concatDiffResult);
    }

    return whitePixelCount;
}

bool DefectDetector::debugImageFlag() const { return m_debugImageFlag; }

void DefectDetector::setDebugImageFlag(bool newDebugImageFlag) {
    m_debugImageFlag = newDebugImageFlag;
}

int DefectDetector::precision() const { return m_precision; }

void DefectDetector::setPrecision(int newPrecision) {
    m_precision = newPrecision;
}

double DefectDetector::scoreThreshold() const { return m_scoreThreshold; }

void DefectDetector::setScoreThreshold(double newScoreThreshold) {
    m_scoreThreshold = newScoreThreshold;
}

int DefectDetector::whiteThreshold() const { return m_whiteThreshold; }

void DefectDetector::setWhiteThreshold(int newWhiteThreshold) {
    m_whiteThreshold = newWhiteThreshold;
}
