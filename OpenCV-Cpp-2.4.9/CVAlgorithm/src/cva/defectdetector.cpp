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
    // cv::imshow("edgeMask", edgeMask);

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
    if (m_debugImageFlag) {
        cv::imshow("edgeMask", edgeMask);
    }

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
    if (m_debugImageFlag) {
        cv::imshow("Result", result);
    }
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

    if (m_debugImageFlag) {
        cv::imshow("Outer Edge Detection Zone", comparison);
    }
    return result;
}

void DefectDetector::setTemplateFolder(const QString &folderName) {
    m_templateList.clear();

    QDir dir(folderName);
    if (!dir.exists()) {
        qWarning() << "警告：模板文件夹不存在: " << folderName;
        return;
    }

    QStringList imageFiles;
    QStringList filters;
    foreach (const QString &format, QImageReader::supportedImageFormats()) {
        filters << "*." + format;
    }

    imageFiles.append(dir.entryList(filters, QDir::Files));
    for (int i = 0; i < imageFiles.size(); ++i) {
        imageFiles[i] = dir.absoluteFilePath(imageFiles[i]);
    }

    for (auto &fileName : imageFiles) {
        auto templateImg = cv::imread(fileName.toLocal8Bit().toStdString());
        if (templateImg.empty()) {
            qDebug() << "templateImg is empty: " << fileName;
            emit errorOccured(IMAGE_LOAD_FAILED, QString("templateImg is empty: %1").arg(fileName));
            continue;
        }
        this->addTemplate(fileName, templateImg);

        auto rotateImg = MinimumBounding::rotate180degree(templateImg);
        this->addTemplate(fileName, rotateImg);
    }
}

bool DefectDetector::setInputMat(cv::Mat inputMat) {
    m_thresholdDiff.clear();

    if (inputMat.empty()) {
        qDebug() << "DefectDetector::setInputMat, inputMat is empty: ";
        emit errorOccured(IMAGE_LOAD_FAILED, QString("inputMat is empty"));

        m_inputMat = cv::Mat();

        return false;
    }

    cv::Mat dInput = inputMat.clone();
    m_inputMat = m_mini.findAndCropObject(dInput);

    m_inputMatContour = m_extractor.findLargestContour(m_inputMat);
    if (m_inputMatContour.empty()) {
        qDebug() << "findLargestContour(m_inputMat) is empty: ";
        return false;
    }

    m_inputMatArea = cv::contourArea(m_inputMatContour);

    m_cornerSplitter.setCornerSideLength(m_cornerSideLength);
    m_corners = m_cornerSplitter.splitCorners(m_inputMat);
    std::vector<cv::Point> ct0 = m_extractor.findLargestContour(std::get<0>(m_corners));
    std::vector<cv::Point> ct1 = m_extractor.findLargestContour(std::get<1>(m_corners));
    std::vector<cv::Point> ct2 = m_extractor.findLargestContour(std::get<2>(m_corners));
    std::vector<cv::Point> ct3 = m_extractor.findLargestContour(std::get<3>(m_corners));
    m_subContours = std::make_tuple(ct0, ct1, ct2, ct3);

    // cv::imshow("0", std::get<0>(m_corners));
    // cv::imshow("1", std::get<1>(m_corners));
    // cv::imshow("2", std::get<2>(m_corners));
    // cv::imshow("3", std::get<3>(m_corners));

	double area0{ 0. };
	double area1{ 0. };
	double area2{ 0. };
	double area3{ 0. };
	if (ct0.size() > 3) {
		area0 = cv::contourArea(ct0);
	}
	if (ct1.size() > 3) {
		area1 = cv::contourArea(ct1);
	}
	if (ct2.size() > 3) {
		area2 = cv::contourArea(ct2);
	}
	if (ct3.size() > 3) {
		area3 = cv::contourArea(ct3);
	}

    m_subContourAreas = std::make_tuple(area0, area1, area2, area3);

    return true;
}

void DefectDetector::addTemplate(const QString &fileName, cv::Mat templateImg) {
    cv::Mat tInput = templateImg.clone();
    tInput = m_mini.findAndCropObject(tInput);
    std::vector<cv::Point> tInputContour = m_extractor.findLargestContour(tInput);

    double tInputArea{0.};
    if (tInputContour.size() > 3) {
        tInputArea = cv::contourArea(tInputContour);
    }

    m_cornerSplitter.setCornerSideLength(m_cornerSideLength);
    std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = m_cornerSplitter.splitCorners(tInput);

    std::vector<cv::Point> ct0 = m_extractor.findLargestContour(std::get<0>(corners));
    std::vector<cv::Point> ct1 = m_extractor.findLargestContour(std::get<1>(corners));
    std::vector<cv::Point> ct2 = m_extractor.findLargestContour(std::get<2>(corners));
    std::vector<cv::Point> ct3 = m_extractor.findLargestContour(std::get<3>(corners));
    std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::make_tuple(ct0, ct1, ct2, ct3);

    double area0{ 0. };
    double area1{ 0. };
    double area2{ 0. };
    double area3{ 0. };
    if (ct0.size() > 3) {
        area0 = cv::contourArea(ct0);
    }
    if (ct1.size() > 3) {
        area1 = cv::contourArea(ct1);
    }
    if (ct2.size() > 3) {
        area2 = cv::contourArea(ct2);
    }
    if (ct3.size() > 3) {
        area3 = cv::contourArea(ct3);
    }

    std::tuple<double, double, double, double> subContourAreas = std::make_tuple(area0, area1, area2, area3);


    this->addTemplateIntoList(fileName, tInput, tInputContour, tInputArea, corners, subContours, subContourAreas);
}

void DefectDetector::addTemplateIntoList(const QString &fileName,
                                        cv::Mat tInput,
                                        std::vector<cv::Point> tInputContour,
                                        double tInputArea,
                                        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners,
                                        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours,
                                        std::tuple<double, double, double, double> subContourAreas) {
    auto tuple = std::make_tuple(fileName, tInput, tInputContour, tInputArea, corners, subContours, subContourAreas);
    m_templateList.append(tuple);
}

int DefectDetector::cornerSideLength() const {
    return m_cornerSideLength;
}

void DefectDetector::setCornerSideLength(int newCornerSideLength) {
    m_cornerSideLength = newCornerSideLength;
}

double DefectDetector::overallAreaThreshold() const {
    return m_overallAreaThreshold;
}

void DefectDetector::setOverallAreaThreshold(double newOverallAreaThreshold) {
    m_overallAreaThreshold = newOverallAreaThreshold;
}

double DefectDetector::overallShapeThreshold() const {
    return m_overallShapeThreshold;
}

void DefectDetector::setOverallShapeThreshold(double newOverallShapeThreshold) {
    m_overallShapeThreshold = newOverallShapeThreshold;
}

double DefectDetector::subAreaThreshold() const {
    return m_subAreaThreshold;
}

void DefectDetector::setSubAreaThreshold(double newSubAreaThreshold) {
    m_subAreaThreshold = newSubAreaThreshold;
}

double DefectDetector::subShapeThreshold() const {
    return m_subShapeThreshold;
}

void DefectDetector::setSubShapeThreshold(double newSubShapeThreshold) {
    m_subShapeThreshold = newSubShapeThreshold;
}

int DefectDetector::innerWidth() const {
    return m_innerWidth;
}

void DefectDetector::setInnerWidth(int newInnerWidth) {
    m_innerWidth = newInnerWidth;
}

int DefectDetector::outterWidth() const {
    return m_outterWidth;
}

void DefectDetector::setOutterWidth(int newOutterWidth) {
    m_outterWidth = newOutterWidth;
}

double DefectDetector::colorDiffCountThreshold() const {
    return m_colorDiffCountThreshold;
}

void DefectDetector::setColorDiffCountThreshold(double newColorDiffCountThreshold) {
    m_colorDiffCountThreshold = newColorDiffCountThreshold;
}

double DefectDetector::missingPixelCountThreshold() const {
    return m_missingPixelCountThreshold;
}

void DefectDetector::setMissingPixelCountThreshold(double newMissingPixelCountThreshold) {
    m_missingPixelCountThreshold = newMissingPixelCountThreshold;
}

QList<cv::Mat> DefectDetector::thresholdDiff() const {
    return m_thresholdDiff;
}

std::tuple<bool, double> DefectDetector::p0_matchArea() {
    double inputMatArea = m_inputMatArea;

    QElapsedTimer timer;
    timer.start();

    QList<double> results;
    for (int i = 0; i < m_templateList.size(); ++i) {
        auto templateTuple = m_templateList[i];

        // QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<0>(templateTuple);
        cv::Mat templateInput = std::get<1>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<2>(templateTuple);
        double tInputArea = std::get<3>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<4>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<5>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<6>(templateTuple);

        double areaDiff = std::abs(tInputArea - inputMatArea) / tInputArea;
        if (areaDiff >= 0) {
            results.append(areaDiff);
        }

        if (m_debugImageFlag) {
            qDebug() << "p0_matchArea tInputArea" << tInputArea << ", inputMatArea" << inputMatArea << ", areaDiff" << areaDiff;
        }

    }

    double minResult{9999};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }

        if (m_debugImageFlag) {
            qDebug() << "p0_matchArea" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
        }
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

        // QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<0>(templateTuple);
        cv::Mat templateInput = std::get<1>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<2>(templateTuple);
        double tInputArea = std::get<3>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<4>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<5>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<6>(templateTuple);

        if (tInputContour.size() > 3 && inputMatContour.size()) {
            double shapeDiff = cv::matchShapes(tInputContour, inputMatContour, CV_CONTOURS_MATCH_I1, 0.0);
            if (shapeDiff >= 0) {
                results.append(shapeDiff);
            }

            if (m_debugImageFlag) {
                qDebug() << "p1_matchShapes shapeDiff" << shapeDiff;
            }
        }
    }

    double minResult{1.};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }

        if (m_debugImageFlag) {
            qDebug() << "p1_matchShapes" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
        }
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

        // QString templateName = std::get<0>(templateTuple);
        QString templateFileName = std::get<0>(templateTuple);
        cv::Mat templateInput = std::get<1>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<2>(templateTuple);
        double tInputArea = std::get<3>(templateTuple);
        std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<4>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<5>(templateTuple);
        std::tuple<double, double, double, double> subContourAreas = std::get<6>(templateTuple);


        if (m_debugImageFlag) {
            qDebug() << "subContourAreas inputContourAreas" << std::get<0>(subContourAreas) << std::get<0>(inputContourAreas);
            qDebug() << "subContourAreas inputContourAreas" << std::get<1>(subContourAreas) << std::get<1>(inputContourAreas);
            qDebug() << "subContourAreas inputContourAreas" << std::get<2>(subContourAreas) << std::get<2>(inputContourAreas);
            qDebug() << "subContourAreas inputContourAreas" << std::get<3>(subContourAreas) << std::get<3>(inputContourAreas);
        }

        double areaDiff0{-1.}, areaDiff1{-1.}, areaDiff2{-1.}, areaDiff3{-1.};
        if (std::get<0>(subContourAreas) > 1e-9 && std::get<0>(inputContourAreas) > 1e-9) {
            areaDiff0 = std::abs(std::get<0>(subContourAreas) - std::get<0>(inputContourAreas)) / std::get<0>(subContourAreas);
        }
        if (std::get<1>(subContourAreas) > 1e-9 && std::get<1>(inputContourAreas) > 1e-9) {
            areaDiff1 = std::abs(std::get<1>(subContourAreas) - std::get<1>(inputContourAreas)) / std::get<1>(subContourAreas);
        }
        if (std::get<2>(subContourAreas) > 1e-9 && std::get<2>(inputContourAreas) > 1e-9) {
            areaDiff2 = std::abs(std::get<2>(subContourAreas) - std::get<2>(inputContourAreas)) / std::get<2>(subContourAreas);
        }
        if (std::get<3>(subContourAreas) > 1e-9 && std::get<3>(inputContourAreas) > 1e-9) {
            areaDiff3 = std::abs(std::get<3>(subContourAreas) - std::get<3>(inputContourAreas)) / std::get<3>(subContourAreas);
        }

        QList<double> areaDiffs;
        areaDiffs << areaDiff0 << areaDiff1 << areaDiff2 << areaDiff3;

        double areaDiffMax = std::max({areaDiff0, areaDiff1, areaDiff2, areaDiff3});
        if (areaDiffMax >= 0) {
            results.append(areaDiffMax);
        }
        if (m_debugImageFlag) {
            qDebug() << "p2_matchSubAreas areaDiffs max:" << areaDiffMax << ", in" << areaDiff0 << areaDiff1 << areaDiff2 << areaDiff3;
        }
    }

    double minResult{9999.};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }
        // qDebug() << "p2_matchSubAreas" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
    } else {
        // 如果为空，则证明没有大于0的面积比值，返回-1，true代表跳过此项检测
        minResult = -1;
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

        // QString templateFileName = std::get<0>(templateTuple);
        // cv::Mat templateInput = std::get<1>(templateTuple);
        // std::vector<cv::Point> tInputContour = std::get<2>(templateTuple);
        // double tInputArea = std::get<3>(templateTuple);
        // std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<4>(templateTuple);
        std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<5>(templateTuple);
        // std::tuple<double, double, double, double> subContourAreas = std::get<6>(templateTuple);

        // qDebug() << "std::get<0>(subContours).size()" << std::get<0>(subContours).size();
        // qDebug() << "std::get<1>(subContours).size()" << std::get<1>(subContours).size();
        // qDebug() << "std::get<2>(subContours).size()" << std::get<2>(subContours).size();
        // qDebug() << "std::get<3>(subContours).size()" << std::get<3>(subContours).size();


        double shapeDiff0{999.}, shapeDiff1{999.}, shapeDiff2{999.}, shapeDiff3{999.};
        if (std::get<0>(subContours).size() > 3 && std::get<0>(inputCornerContours).size() > 3) {
            shapeDiff0 = cv::matchShapes(std::get<0>(subContours), std::get<0>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);
        } else {
            // 如果其中一个面积为0，就不对比
            shapeDiff0 = -1;
        }
        if (std::get<1>(subContours).size() > 3 && std::get<1>(inputCornerContours).size() > 3) {
            shapeDiff1 = cv::matchShapes(std::get<1>(subContours), std::get<1>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);
        } else {
            // 如果其中一个面积为0，就不对比
            shapeDiff1 = -1;
        }
        if (std::get<2>(subContours).size() > 3 && std::get<2>(inputCornerContours).size() > 3) {
            shapeDiff2 = cv::matchShapes(std::get<2>(subContours), std::get<2>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);
        } else {
            // 如果其中一个面积为0，就不对比
            shapeDiff2 = -1;
        }
        if (std::get<3>(subContours).size() > 3 && std::get<3>(inputCornerContours).size() > 3) {
            shapeDiff3 = cv::matchShapes(std::get<3>(subContours), std::get<3>(inputCornerContours), CV_CONTOURS_MATCH_I1, 0.0);
        } else {
            // 如果其中一个面积为0，就不对比
            shapeDiff3 = -1;
        }

        QList<double> shapeDiffs;
        shapeDiffs << shapeDiff0 << shapeDiff1 << shapeDiff2 << shapeDiff3;
        // qDebug() << "p3_matchSubShapes matchShapes" << shapeDiffs;
        double shapeDiffMax = std::max({shapeDiff0, shapeDiff1, shapeDiff2, shapeDiff3});
        if (shapeDiffMax >= 0) {
            results.append(shapeDiffMax);
        }
    }

    double minResult{9999.};
    if (!results.isEmpty()) {
        for (const double &value : results) {
            if (value < minResult) {
                minResult = value;
            }
        }
        qDebug() << "p3_matchSubShapes" << results << "最小值:" << minResult << ", elapsed" << timer.nsecsElapsed();
    } else {
        minResult = -1;
    }

    return std::make_tuple((minResult < m_subShapeThreshold), minResult);
}

std::tuple<bool, int, int> DefectDetector::p4_fullMatchMatPixel() {
    QElapsedTimer timer;
    timer.start();

    cv::Mat dInput = m_inputMat.clone();
    // cv::pyrDown(dInput, dInput);

    QElapsedTimer innerTimer;
    QList<int> missingPixelCountResults;
    QList<int> colorDiffPixelCountResults;
    for (int i = 0; i < m_templateList.size(); ++i) {
        innerTimer.restart();

        auto templateTuple = m_templateList[i];

        // QString templateName = std::get<0>(templateTuple);
        // QString templateFileName = std::get<0>(templateTuple);
        cv::Mat templateInput = std::get<1>(templateTuple);
        std::vector<cv::Point> tInputContour = std::get<2>(templateTuple);
        // double tInputArea = std::get<3>(templateTuple);
        // std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = std::get<4>(templateTuple);
        // std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::get<5>(templateTuple);
        // std::tuple<double, double, double, double> subContourAreas = std::get<6>(templateTuple);

        // qDebug() << "matchMat elapsed:" << double(innerTimer.nsecsElapsed()) / 1e6 << "ms";
        // innerTimer.restart();

        cv::Mat tInput = templateInput.clone();

        // qDebug() << "clone elapsed:" << double(innerTimer.nsecsElapsed()) / 1e6 << "ms";
        // innerTimer.restart();

        // cv::pyrDown(tInput, tInput);

        // 略微耗时
        // for (int i = 0; i < 0; ++i) {
        //     int blurCoreSize = 3;
        //     // blur GaussianBlur medianBlur bilateralFilter
        //     switch (0) {
        //     case 0: { // blur
        //         cv::blur(tInput, tInput, cv::Size(3, 3));
        //         cv::blur(dInput, dInput, cv::Size(3, 3));
        //     } break;
        //     case 1: { // GaussianBlur
        //         cv::GaussianBlur(tInput, tInput, cv::Size(blurCoreSize, blurCoreSize), 0, 0);
        //         cv::GaussianBlur(dInput, dInput, cv::Size(blurCoreSize, blurCoreSize), 0, 0);
        //     } break;
        //     case 2: { // medianBlur
        //         cv::medianBlur(tInput, tInput, 5);
        //         cv::medianBlur(dInput, dInput, 5);
        //     } break;
        //     case 3: { // bilateralFilter
        //         cv::bilateralFilter(tInput, tInput, 9, 50, 10);
        //         cv::bilateralFilter(dInput, dInput, 9, 50, 10);
        //     } break;
        //     }
        // }

        cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0, cv::INTER_LINEAR);

        // qDebug() << "resize elapsed:" << double(innerTimer.nsecsElapsed()) / 1e6 << "ms";
        // innerTimer.restart();

        auto tContour = tInputContour; // m_extractor.findContours(tInput);
        auto dContour = m_extractor.findContours(dInput);

        // qDebug() << "findContours elapsed:" << double(innerTimer.nsecsElapsed()) / 1e6 << "ms";
        // innerTimer.restart();

        cv::Mat tEdge = this->processRingEdge(tInput, std::vector<std::vector<cv::Point>>({tInputContour}), m_outterWidth, m_innerWidth);
        cv::Mat dEdge = this->processRingEdge(dInput, std::vector<std::vector<cv::Point>>({tInputContour}), m_outterWidth, m_innerWidth);

        // qDebug() << "processRingEdge elapsed:" << double(innerTimer.nsecsElapsed()) / 1e6 << "ms";
        // innerTimer.restart();

        // cv::imshow("tEdge", tEdge);
        // cv::imshow("dEdge", dEdge);

        int missingPixelCount, colorDiffPixelCount;
        std::tie(missingPixelCount, colorDiffPixelCount)  = this->matchMatPixel(tEdge, dEdge);
        // qDebug() << "matchMatPixel missingPixelCount" << missingPixelCount << ", colorDiffPixelCount" << colorDiffPixelCount << ", matchMat elapsed:" << double(innerTimer.nsecsElapsed()) / 1e6 << "ms";

        if (missingPixelCount >= 0) {
            missingPixelCountResults.append(missingPixelCount);
        }
        if (missingPixelCount >= 0) {
            colorDiffPixelCountResults.append(colorDiffPixelCount);
        }
        // break;
    }

    auto findMin = [](QList<int> list){
        double minResult{1000000};
        if (!list.isEmpty()) {
            for (const double &value : list) {
                if (value < minResult) {
                    minResult = value;
                }
            }
        }
        return minResult;
    };

    double minResult1 = findMin(missingPixelCountResults);
    // if (!missingPixelCountResults.isEmpty()) {
    //     for (const double &value : missingPixelCountResults) {
    //         if (value < minResult1) {
    //             minResult1 = value;
    //         }
    //     }
    //     qDebug() << "p4_fullMatchMatPixel" << missingPixelCountResults << "最小值:" << minResult1 << ", elapsed" << double(timer.nsecsElapsed()) / 1e6 << "ms";
    // }

    double minResult2 = findMin(colorDiffPixelCountResults);
    // if (!colorDiffPixelCountResults.isEmpty()) {
    //     for (const double &value : colorDiffPixelCountResults) {
    //         if (value < minResult2) {
    //             minResult2 = value;
    //         }
    //     }
    //     qDebug() << "p4_fullMatchMatPixel" << missingPixelCountResults << "最小值:" << minResult2 << ", elapsed" << double(timer.nsecsElapsed()) / 1e6 << "ms";
    // }

    return std::make_tuple((minResult1 <= m_missingPixelCountThreshold) && (minResult2 <= m_colorDiffCountThreshold), minResult1, minResult2);
}

// 输入的是环形边缘
std::tuple<int, int> DefectDetector::matchMatPixel(cv::Mat tEdge, cv::Mat dEdge) {
    QElapsedTimer timer;
    timer.start();

    if (tEdge.empty() || dEdge.empty()) {
        QMessageBox::warning(nullptr, "错误", "请先加载正常图像和缺陷图像!");
        return std::make_tuple(9999, 9999);
    }

    cv::Mat tInput = tEdge.clone();
    cv::Mat dInput = dEdge.clone();

    // cv::pyrDown(tInput, tInput);
    // cv::pyrDown(dInput, dInput);




    cv::Mat dEdgeGray, tEdgeGray;

    if (tInput.channels() == 3) {
        cv::cvtColor(tInput, tEdgeGray, cv::COLOR_BGR2GRAY);
        cv::threshold(tEdgeGray, tEdgeGray, m_binaryThreshold, 255, cv::THRESH_BINARY);
    }

    if (dInput.channels() == 3) {
        cv::cvtColor(dInput, dEdgeGray, cv::COLOR_BGR2GRAY);
        cv::threshold(dEdgeGray, dEdgeGray, m_binaryThreshold, 255, cv::THRESH_BINARY);
    }

    // cv::imshow("tEdgeGray1", tEdgeGray);
    // cv::imshow("dEdgeGray1", dEdgeGray);

    cv::Mat dEdgeBinary, tEdgeBinary;
    cv::threshold(tEdgeGray, tEdgeBinary, m_binaryThreshold, 255, cv::THRESH_BINARY);
    cv::threshold(dEdgeGray, dEdgeBinary, m_binaryThreshold, 255, cv::THRESH_BINARY);

    // cv::imshow("tEdgeBinary", tEdgeBinary);
    // cv::imshow("dEdgeBinary", dEdgeBinary);

    cv::Mat missingMask;
    cv::Mat tEdgeNotWhite = (tEdgeBinary != 255);

    cv::bitwise_and(dEdgeBinary, tEdgeNotWhite, missingMask);
    int missingPixelCount = cv::countNonZero(missingMask);
    // qDebug() << "tEdgeBinary.channels" << tEdgeBinary.channels();
    // qDebug() << "dEdgeBinary.channels" << dEdgeBinary.channels();
    // qDebug() << "resultMask.channels" << missingMask.channels();
    // qDebug() << "missingDefectPixelCount" << missingDefectPixelCount;

    // cv::imshow("missingMask", missingMask);





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

    cv::Mat diffResult;
    cv::threshold(grayDiff, diffResult, m_whiteThreshold, 255, cv::THRESH_BINARY);

    cv::Mat colorDiffDefect;

    // cv::pyrDown(missingMask, missingMask);

    cv::Mat nonMissingMask;
    cv::bitwise_not(missingMask, nonMissingMask);

    // cv::imshow("diffResult", diffResult);
    cv::Mat thresholdDiff = diffResult;

    // m_thresholdDiff.append(thresholdDiff);


    if ((diffResult.size() != nonMissingMask.size()) || (diffResult.channels() != nonMissingMask.channels())) {
        qDebug() << "size:" << diffResult.size().width << diffResult.size().height << nonMissingMask.size().width << nonMissingMask.size().height;
        qDebug() << "channels:" << diffResult.channels() << nonMissingMask.channels();
        qFatal("(diffResult.size() != nonMissingMask.size()) || (diffResult.size() != nonMissingMask.size())");
    }

    cv::bitwise_and(diffResult, nonMissingMask, colorDiffDefect);

    // cv::imshow("colorDiffDefect", colorDiffDefect);

    // thresholdDiff = colorDiffDefect;

    // int kernalSize = 3; // 从3改变到5，可以去掉矩形物料diff边缘的噪声
    // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
    // cv::Size(kernalSize, kernalSize)); cv::morphologyEx(thresholdDiff,
    // thresholdDiff, cv::MORPH_OPEN,
    //                  kernel);

    int colorDiffPixelCount = cv::countNonZero(colorDiffDefect);
    // qDebug() << "colorDiffPixelCount" << colorDiffPixelCount;

    cv::Mat resultImage = cv::Mat::zeros(missingMask.size(), CV_8UC3);
    resultImage.setTo(cv::Scalar(0, 0, 255), missingMask);
    resultImage.setTo(cv::Scalar(0, 255, 0), colorDiffDefect);

    m_thresholdDiff.append(resultImage);


    if (m_debugImageFlag) {
        cv::Mat concatDiffResult;

        cv::vconcat(std::vector<cv::Mat>{vdiff, grayDiff, thresholdDiff}, concatDiffResult);
        cv::imshow("concatDiffResult", concatDiffResult);
    }

    // qDebug() << "matchMatPixel , elapsed" << double(timer.nsecsElapsed()) / 1e6 << "ms";

    return std::make_tuple(missingPixelCount, colorDiffPixelCount);
}

bool DefectDetector::debugImageFlag() const { return m_debugImageFlag; }

void DefectDetector::setDebugImageFlag(bool newDebugImageFlag) {
    m_debugImageFlag = newDebugImageFlag;
}

int DefectDetector::precision() const { return m_precision; }

void DefectDetector::setPrecision(int newPrecision) {
    m_precision = newPrecision;
}

int DefectDetector::whiteThreshold() const { return m_whiteThreshold; }

void DefectDetector::setWhiteThreshold(int newWhiteThreshold) {
    m_whiteThreshold = newWhiteThreshold;
}
