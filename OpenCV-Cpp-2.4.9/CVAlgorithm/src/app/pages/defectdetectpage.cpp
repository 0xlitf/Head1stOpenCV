#include "defectdetectpage.h"
#include <QApplication>
#include <QElapsedTimer>
#include "minimumbounding.h"
#include "opencv2/opencv.hpp"
#include "bgr2hsvconverter.h"
#include "defectdetector.h"
#include "areadifference.h"
#include "cornersplitter.h"

DefectDetectPage::DefectDetectPage() {
    qDebug() << "DefectDetectPage";

    // testContour();
    // testAreaDiff();
    // testCorner();
    testP0();
}

void DefectDetectPage::testAreaDiff() {
    // 黑色矩形
    cv::Mat tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                   "DefectDetection/template_black/1ok.png")
                               .toStdString());
    // 缺角
    cv::Mat dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                   "DefectDetection/2/NG/2026-01-15_16-09-20_925.png")
                               .toStdString());

    MinimumBounding mini;
    cv::Mat tInput = tInputMat;
    tInput = mini.findAndCropObject(tInput);

    cv::Mat dInput = dInputMat;
    dInput = mini.findAndCropObject(dInput);

    cv::imshow("tInput", tInput);
    cv::imshow("dInput", dInput);

    AreaDifference areaDiff;

}

void DefectDetectPage::testDefect() {
    MinimumBounding mini;
    BGR2HSVConverter converter;
    DefectDetector detector;

    // cv::Mat bgMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1/test.png");
    // cv::Mat bgMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1_20260122153531310_5_52.png").toStdString());

    // int count{0};
    // cv::Mat res;
    // std::tie(count, res) = detector.analyzeAndDrawContour(bgMat);
    // qDebug() << "count" << count;

    // ContourExtractor extractor;

    // 4. 或者使用调试版本（返回轮廓+可视化结果）
    // std::vector<std::vector<cv::Point>> contoursDebug;
    // cv::Mat debugImage;
    // std::tie(contoursDebug, debugImage) = extractor.extractContourWithDebug(res);

    // cv::imshow("res", res);

    // 5. 处理提取的轮廓
    // if (contoursDebug.size() == 1) {
    //     qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
    //     auto contour = contoursDebug[0];
    //     double area = cv::contourArea(contour);
    //     cv::Rect bbox = cv::boundingRect(contour);

    //     qDebug() << "轮廓 " << 0 << ": "
    //              << "面积 = " << area
    //              << ", 边界框 = " << bbox.width << bbox.height
    //              << ", 点数 = " << contour.size();

    //     // 显示调试图像
    //     cv::imshow("轮廓提取结果", debugImage);
    //     cv::waitKey(0);
    // } else {
    //     qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
    //     qDebug() << "contoursDebug.size() != 1";
    // }

    detector.setRemoveOuterBorderThickness(3);
    detector.setDetectThickness(6);
    detector.setScoreThreshold(15);

    // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/1-1.png");
    // cv::Mat objMat = cv::imread(QString(PROJECT_DIR).append("/1-2.png").toStdString());
    // cv::Mat objMat = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2.png");
    // cv::Mat objMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1_20260122153531310_5_52.png").toStdString());
    // objMat = mini.findAndCropObject(objMat);
    // auto r = detector.analyzeAndDrawContour(objMat);
    // qDebug() << "count " << std::get<0>(r);
    // // cv::imshow("std::get<1>(r)" , std::get<1>(r));

    // auto objContour = detector.findContours(objMat);
    // auto objMatClone = objMat.clone();

    // if (!objContour.empty()) {
    //     // detector.displayContourMask(objMatClone, objContour, 20);

    //     // 或者使用方法2
    //     detector.displayOuterEdge(objMatClone, objContour, 10);
    // }

    // QStringList descList;
    // descList << "ok";
    // QStringList folderList;
    // folderList << QString(PROJECT_DIR).append("/template_black"); // template_brown template_black
    // detector.setTemplateFolder(descList, folderList);

    cv::Mat tInputMat;
    cv::Mat dInputMat;
    int outterWidth = 4;
    int innerWidth = 10;

    switch (3) {
    case 0: {
        // 黑色异形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1ok (1).png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/NG/1ng (3).png")
                                   .toStdString());
        outterWidth = 4;
        innerWidth = 10;
    } break;
    case 1: {
        // 黑色矩形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                       "DefectDetection/template_black/1ok.png")
                                   .toStdString());
        // 缺角
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                       "DefectDetection/2/NG/2026-01-15_16-09-20_925.png")
                                   .toStdString());

        // dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
        //                                "DefectDetection/template_black/5ok.png")
        //                            .toStdString());
        outterWidth = 5;
        innerWidth = 11;
        detector.setWhiteThreshold(50);
    } break;
    case 2: {
        // 黑色异形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1ok (1).png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/NG/1ng (5).png")
                                   .toStdString());
        outterWidth = 4;
        innerWidth = 10;
    } break;
    case 3: {
        // 橙色矩形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/template_brown/1ok (2)_template.png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/1/NG/1ng (2).png")
                                   .toStdString());
        outterWidth = 3;
        innerWidth = 6;
    } break;
    }

    // cv::pyrDown(tInputMat, tInputMat);
    // cv::pyrDown(dInputMat, dInputMat);


    for (int i = 0; i < 2; ++i) {
        int blurCoreSize = 3;
        // blur GaussianBlur medianBlur bilateralFilter
        switch (0) {
        case 0: { // blur
            cv::blur(tInputMat, tInputMat, cv::Size(3, 3));
            cv::blur(dInputMat, dInputMat, cv::Size(3, 3));
        } break;
        case 1: { // GaussianBlur
            cv::GaussianBlur(tInputMat, tInputMat, cv::Size(blurCoreSize, blurCoreSize), 0, 0);
            cv::GaussianBlur(dInputMat, dInputMat, cv::Size(blurCoreSize, blurCoreSize), 0, 0);
        } break;
        case 2: { // medianBlur
            cv::medianBlur(tInputMat, tInputMat, 5);
            cv::medianBlur(dInputMat, dInputMat, 5);
        } break;
        case 3: { // bilateralFilter
            cv::bilateralFilter(tInputMat, tInputMat, 9, 50, 10);
            cv::bilateralFilter(dInputMat, dInputMat, 9, 50, 10);
        } break;
        }
    }

    cv::Mat tInput = tInputMat;
    tInput = mini.findAndCropObject(tInput);

    cv::Mat dInput = dInputMat;
    dInput = mini.findAndCropObject(dInput);

    cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0, cv::INTER_LINEAR);

    cv::imshow("tInput", tInput);
    cv::imshow("dInput", dInput);


    ContourExtractor extractor;
    auto tContour = extractor.findContours(tInput);
    auto dContour = extractor.findContours(dInput);

    // 使用新的像素级环形边缘缺陷检测函数
    QElapsedTimer timer;
    timer.start();

    cv::Mat tEdge = detector.processRingEdge(tInput, tContour, outterWidth, innerWidth);
    cv::Mat dEdge = detector.processRingEdge(dInput, tContour, outterWidth, innerWidth);
    double defectScore = detector.matchMat(tEdge, dEdge);
    qDebug() << "defectScore:" << defectScore << ", matchMat elapsed:" << timer.elapsed();

}

void DefectDetectPage::testContour() {

    cv::Mat tInputMat;
    cv::Mat dInputMat;

    switch (2) {
    case 0: {
        // 黑色异形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1ok (1).png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/NG/1ng (3).png")
                                   .toStdString());
    } break;
    case 1: {
        // 黑色矩形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                       "DefectDetection/template_black/1ok.png")
                                   .toStdString());
        // 缺角
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                       "DefectDetection/2/NG/2026-01-15_16-09-20_925.png")
                                   .toStdString());

        // dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
        //                                "DefectDetection/template_black/5ok.png")
        //                            .toStdString());
    } break;
    case 2: {
        // 黑色异形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1ok (1).png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/NG/1ng (5).png")
                                   .toStdString());
    } break;
    case 3: {
        // 橙色矩形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/template_brown/1ok (2)_template.png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/1/NG/1ng (2).png")
                                   .toStdString());
    } break;
    }

    cv::pyrDown(tInputMat, tInputMat);
    cv::pyrDown(dInputMat, dInputMat);

    MinimumBounding mini;
    cv::Mat tInput = tInputMat;
    tInput = mini.findAndCropObject(tInput);
    qDebug() << "tInput.channels()" << tInput.channels();

    cv::Mat dInput = dInputMat;
    dInput = mini.findAndCropObject(dInput);
    qDebug() << "dInput.channels()" << dInput.channels();

    // 1. 创建轮廓提取器
    ContourExtractor extractor;

    // 设置参数（面积阈值 > 1000）
    extractor.setParameters(1000.0, true, 3, 11);

    // 2. 读取白背景物料图像
    if (tInputMat.empty()) {
        qDebug() << "无法读取图像！";
        return;
    }

    // 3. 提取轮廓（基础版本）
    // std::vector<std::vector<cv::Point>> contours = extractor.extractWhiteBackgroundContour(tInputMat);

    std::vector<std::vector<cv::Point>> contours;
    cv::Mat debugImage1;
    std::tie(contours, debugImage1) = extractor.extractContourWithDebug(tInput);

    // 5. 处理提取的轮廓
    double area1;
    if (contours.size() == 1) {
        qDebug() << "提取到 " << contours.size() << " 个轮廓：";
        auto contour = contours[0];
        area1 = cv::contourArea(contour);
        cv::Rect bbox = cv::boundingRect(contour);

        qDebug() << "轮廓 " << 0 << ": "
                 << "面积 = " << area1 << ", 边界框 = " << bbox.width << bbox.height << ", 点数 = " << contour.size();

        // 显示调试图像
        // cv::imshow("轮廓提取结果", debugImage);
        cv::imshow("contour result1", debugImage1);
        // cv::waitKey(0);
    } else {
        qDebug() << "提取到 " << contours.size() << " 个轮廓：";
        qDebug() << "contoursDebug.size() != 1";
    }

    // 4. 或者使用调试版本（返回轮廓+可视化结果）
    std::vector<std::vector<cv::Point>> contoursDebug;
    cv::Mat debugImage2;
    std::tie(contoursDebug, debugImage2) = extractor.extractContourWithDebug(dInput);

    // 5. 处理提取的轮廓
    double area2;
    if (contoursDebug.size() == 1) {
        qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
        auto contour = contoursDebug[0];
        area2 = cv::contourArea(contour);
        cv::Rect bbox = cv::boundingRect(contour);

        qDebug() << "轮廓 " << 0 << ": "
                 << "面积 = " << area2 << ", 边界框 = " << bbox.width << bbox.height << ", 点数 = " << contour.size();

        // 显示调试图像
        cv::imshow("contour result2", debugImage2);
        // cv::waitKey(0);
    } else {
        qDebug() << "提取到 " << contoursDebug.size() << " 个轮廓：";
        qDebug() << "contoursDebug.size() != 1";
    }

    qDebug() << "area1-area2" << (area1-area2) / area1;


    auto matchScore = cv::matchShapes(contours[0], contoursDebug[0], CV_CONTOURS_MATCH_I1, 0.0);
    qDebug() << "matchScore" << matchScore;

    // cv::cvtColor(tInput, tInput, cv::COLOR_BGR2GRAY);
    // cv::cvtColor(dInput, dInput, cv::COLOR_BGR2GRAY);

    // cv::Mat tInputLargest, dInputLargest;
    // cv::threshold(tInput, tInputLargest, 240, 255,
    //               cv::THRESH_BINARY);

    // cv::threshold(dInput, dInputLargest, 240, 255,
    //               cv::THRESH_BINARY);

    // cv::imshow("tInputLargest", tInputLargest);
    // cv::imshow("dInputLargest", dInputLargest);

    std::vector<cv::Point> contour1 = extractor.findLargestContour(tInput);
    std::vector<cv::Point> contour2 = extractor.findLargestContour(dInput);
    auto matchScore2 = cv::matchShapes(contour1, contour2, CV_CONTOURS_MATCH_I1, 0.0);
    qDebug() << "matchScore2" << matchScore2;

    // 未经下采样时是 0.0129737
    // 经下采样时是 0.0344635

    qDebug() << "面积检测阈值0.05";
    qDebug() << "整体轮廓匹配阈值0.05, 对小的缺角和缺边检测不佳";
}

void DefectDetectPage::testCorner() {

    cv::Mat tInputMat;
    cv::Mat dInputMat;

    switch (1) {
    case 0: {
        // 黑色异形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1ok (1).png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/NG/1ng (3).png")
                                   .toStdString());
    } break;
    case 1: {
        // 黑色矩形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                       "DefectDetection/template_black/1ok.png")
                                   .toStdString());
        // 缺角
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                       "DefectDetection/2/NG/2026-01-15_16-09-20_925.png")
                                   .toStdString());

        // dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
        //                                "DefectDetection/template_black/5ok.png")
        //                            .toStdString());
    } break;
    case 2: {
        // 黑色异形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/OK/1ok (1).png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/Irregular/NG/1ng (5).png")
                                   .toStdString());
    } break;
    case 3: {
        // 橙色矩形
        tInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/template_brown/1ok (2)_template.png")
                                   .toStdString());
        dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/1/NG/1ng (2).png")
                                   .toStdString());
    } break;
    }

    // cv::pyrDown(tInputMat, tInputMat);
    // cv::pyrDown(dInputMat, dInputMat);

    MinimumBounding mini;
    cv::Mat tInput = tInputMat;
    tInput = mini.findAndCropObject(tInput);
    qDebug() << "tInput.channels()" << tInput.channels();

    cv::Mat dInput = dInputMat;
    dInput = mini.findAndCropObject(dInput);
    qDebug() << "dInput.channels()" << dInput.channels();

    cv::imshow("tInput", tInput);
    cv::imshow("dInput", dInput);

    // 1. 创建轮廓提取器
    ContourExtractor extractor;

    // 设置参数（面积阈值 > 100）
    extractor.setParameters(100.0, true, 3, 11);

    // 2. 读取白背景物料图像
    if (tInputMat.empty()) {
        qDebug() << "无法读取图像！";
        return;
    }

    CornerSplitter sp;
    auto tCorner = sp.splitCorners(tInput);
    auto dCorner = sp.splitCorners(dInput);

    cv::Mat tMat0 = std::get<0>(tCorner);
    cv::Mat tMat1 = std::get<1>(tCorner);
    cv::Mat tMat2 = std::get<2>(tCorner);
    cv::Mat tMat3 = std::get<3>(tCorner);
    cv::Mat dMat0 = std::get<0>(dCorner);
    cv::Mat dMat1 = std::get<1>(dCorner);
    cv::Mat dMat2 = std::get<2>(dCorner);
    cv::Mat dMat3 = std::get<3>(dCorner);
    cv::pyrUp(tMat0, tMat0);
    cv::pyrUp(tMat1, tMat1);
    cv::pyrUp(tMat2, tMat2);
    cv::pyrUp(tMat3, tMat3);
    cv::pyrUp(dMat0, dMat0);
    cv::pyrUp(dMat1, dMat1);
    cv::pyrUp(dMat2, dMat2);
    cv::pyrUp(dMat3, dMat3);

    cv::imshow("dMat0", dMat0);
    cv::imshow("dMat1", dMat1);
    cv::imshow("dMat2", dMat2);
    cv::imshow("dMat3", dMat3);

    // 0.0328143 0.00122315 0.00038688 0.000265578
    // 0.0309739 0.00137884 0.000594925 0.000656243

    std::vector<cv::Point> t0 = extractor.findLargestContour(tMat0);
    std::vector<cv::Point> t1 = extractor.findLargestContour(tMat1);
    std::vector<cv::Point> t2 = extractor.findLargestContour(tMat2);
    std::vector<cv::Point> t3 = extractor.findLargestContour(tMat3);
    std::vector<cv::Point> d0 = extractor.findLargestContour(dMat0);
    std::vector<cv::Point> d1 = extractor.findLargestContour(dMat1);
    std::vector<cv::Point> d2 = extractor.findLargestContour(dMat2);
    std::vector<cv::Point> d3 = extractor.findLargestContour(dMat3);
    auto matchScore0 = cv::matchShapes(t0, d0, CV_CONTOURS_MATCH_I1, 0.0);
    auto matchScore1 = cv::matchShapes(t1, d1, CV_CONTOURS_MATCH_I1, 0.0);
    auto matchScore2 = cv::matchShapes(t2, d2, CV_CONTOURS_MATCH_I1, 0.0);
    auto matchScore3 = cv::matchShapes(t3, d3, CV_CONTOURS_MATCH_I1, 0.0);
    qDebug() << "matchScore0" << matchScore0;
    qDebug() << "matchScore1" << matchScore1;
    qDebug() << "matchScore2" << matchScore2;
    qDebug() << "matchScore3" << matchScore3;

    // 未经下采样时是 0.0129737
    // 经下采样时是 0.0344635
}

void DefectDetectPage::testP0() {
    // 面积检测一般差异小于0.05

    // 将下面5个对象作为类成员变量
    MinimumBounding mini;
    BGR2HSVConverter converter;
    DefectDetector detector;
    ContourExtractor extractor;
    CornerSplitter cornerSplitter;

    QStringList descList;
    descList << "ok";
    QStringList folderList;
    folderList << QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection").append("/template_black"); // template_brown template_black
    detector.setTemplateFolder(descList, folderList);

    auto dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                   "DefectDetection/2/NG/2026-01-15_16-09-37_766.png")  // 细小角落缺陷 2026-01-15_16-09-37_766.png
                                    // 明显缺角 2026-01-15_16-09-20_925.png
                               .toStdString());

    cv::Mat dInput = dInputMat;
    dInput = mini.findAndCropObject(dInput);

    std::vector<cv::Point> dInputContour = extractor.findLargestContour(dInput);
    double dInputArea = cv::contourArea(dInputContour);

    std::tuple<cv::Mat, cv::Mat, cv::Mat, cv::Mat> corners = cornerSplitter.splitCorners(dInput);
    std::vector<cv::Point> ct0 = extractor.findLargestContour(std::get<0>(corners));
    std::vector<cv::Point> ct1 = extractor.findLargestContour(std::get<1>(corners));
    std::vector<cv::Point> ct2 = extractor.findLargestContour(std::get<2>(corners));
    std::vector<cv::Point> ct3 = extractor.findLargestContour(std::get<3>(corners));
    std::tuple<std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>, std::vector<cv::Point>> subContours = std::make_tuple(ct0, ct1, ct2, ct3);

    double area0 = cv::contourArea(ct0);
    double area1 = cv::contourArea(ct1);
    double area2 = cv::contourArea(ct2);
    double area3 = cv::contourArea(ct3);
    std::tuple<double, double, double, double> subContourAreas = std::make_tuple(area0, area1, area2, area3);

    double areaDiff = detector.p0_matchArea(dInputArea); // 整体轮廓面积，小于0.01合格，对于比较厚的物料，适当增大本阈值
    qDebug() << "areaDiff" << areaDiff;

    double shapeDiff = detector.p1_matchShapes(dInputContour); // 整体轮廓形状分数，小于0.01~0.05合格
    qDebug() << "shapeDiff" << shapeDiff;

    double subAreaDiff = detector.p2_matchSubAreas(subContourAreas); // 子区域轮廓面积，小于0.02合格
    qDebug() << "subAreaDiff" << subAreaDiff;

    double subShapeDiff = detector.p3_matchSubShapes(subContours); // 子区域轮廓形状分数，小于0.002合格
    qDebug() << "subShapeDiff" << subShapeDiff;

    double defectScore = detector.p4_fullMatchMatPixel(dInput); // 缺陷像素点数，小于15合格
    qDebug() << "defectScore" << defectScore;

}
