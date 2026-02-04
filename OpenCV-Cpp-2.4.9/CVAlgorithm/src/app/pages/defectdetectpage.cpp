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

    test();
}

void DefectDetectPage::test() {
    DefectDetector detector;

    QStringList descList;
    descList << "ok";
    QStringList folderList;
    folderList << QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection").append("/template_black"); // template_brown template_black
    detector.setTemplateFolder(descList, folderList);

    auto dInputMat = cv::imread(QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/"
                                   "DefectDetection/2/NG/2026-01-15_16-09-37_766.png")  // 细小角落缺陷 2026-01-15_16-09-37_766.png
                                    // 明显缺角 2026-01-15_16-09-20_925.png
                               .toStdString());

    std::tuple<int, cv::Mat> analyzeResult = ContourExtractor::analyzeAndDrawContour(dInputMat);
    if (std::get<0>(analyzeResult) != 1) {
        qDebug() << "analyzeResult 物体数量 != 1";
        cv::imshow("analyzeResult", std::get<1>(analyzeResult));
    }

    detector.setInputMat(dInputMat);

    std::tuple<bool, double> areaDiff = detector.p0_matchArea(); // 整体轮廓面积，小于0.01合格，对于比较厚的物料，适当增大本阈值
    std::tuple<bool, double> shapeDiff = detector.p1_matchShapes(); // 整体轮廓形状分数，小于0.01~0.05合格
    std::tuple<bool, double> subAreaDiff = detector.p2_matchSubAreas(); // 子区域轮廓面积，对于细微的角落缺陷，小于0.02合格
    std::tuple<bool, double> subShapeDiff = detector.p3_matchSubShapes(); // 子区域轮廓形状分数，小于0.002合格
    std::tuple<bool, double> defectScore = detector.p4_fullMatchMatPixel(); // 缺陷像素点数，小于15合格

    qDebug() << "p0_matchArea areaDiff" << std::get<0>(areaDiff) << std::get<1>(areaDiff);
    qDebug() << "p1_matchShapes shapeDiff" << std::get<0>(shapeDiff) << std::get<1>(shapeDiff);
    qDebug() << "p2_matchSubAreas subAreaDiff" << std::get<0>(subAreaDiff) << std::get<1>(subAreaDiff);
    qDebug() << "p3_matchSubShapes subShapeDiff" << std::get<0>(subShapeDiff) << std::get<1>(subShapeDiff);
    qDebug() << "p4_fullMatchMatPixel defectScore" << std::get<0>(defectScore) << std::get<1>(defectScore);

}
