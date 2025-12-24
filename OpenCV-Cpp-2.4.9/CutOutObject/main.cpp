#include "mainwindow.h"

#include <QApplication>
#include <QElapsedTimer>
#include <opencv2/opencv.hpp>

#include "cutoutobject.h"
#include <iostream>

void cutoutForBlue(cv::Mat &cvImage, int nColorThreshold, int nBThreshold) {
    for (int i = 0; i < cvImage.rows; ++i) {
        for (int j = 0; j < cvImage.cols; ++j) {
            cv::Vec3b &pixel = cvImage.at<cv::Vec3b>(i, j);
            if ((pixel[0] - pixel[1] > nColorThreshold) &&
                (pixel[0] - pixel[2] > nColorThreshold) && (pixel[0] > nBThreshold)) {
                // 蓝色区域：设为白色
                pixel[0] = 255;
                pixel[1] = 255;
                pixel[2] = 255;
            } else {
                // 非蓝色区域：设为黑色
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;
            }
        }
    }
}

void test() {
    auto imageWithBackground =
        cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/CutOutObject/"
                   "dataset/14-39-42-685.png");
    cv::imshow("imageWithBackground", imageWithBackground);

    // 复制图像用于处理
    cv::Mat result1 = imageWithBackground.clone();
    cv::Mat result2 = imageWithBackground.clone();
    cv::Mat result3 = imageWithBackground.clone();

    cutoutForBlue(result1, 30, 50);
    cv::imshow("宽松阈值 (30,50)", result1);

    // 中等阈值：平衡检测
    cutoutForBlue(result2, 50, 100);
    cv::imshow("中等阈值 (50,100)", result2);

    // 严格阈值：只检测明显的蓝色
    cutoutForBlue(result3, 80, 150);
    cv::imshow("严格阈值 (80,150)", result3);

    cv::waitKey(0);
    cv::destroyAllWindows();
}

int main(int argc, char *argv[]) {
    // QApplication a(argc, argv);
    // MainWindow w;
    // w.show();
    // return a.exec();

    CutOutObject cutout;
    // cutout.testExtractLargestContour(imageName.toStdString());

    // QString imageName = QString(PROJECT_DIR) + "/dataset/bg.png"; // 纯背景
    // QString imageName = QString(PROJECT_DIR) + "/dataset/1.png"; // 1个物体
    QString imageName = QString(PROJECT_DIR) + "/dataset/2.png"; // 2个物体
    qDebug() << "imageName:" << imageName;

    auto image = cv::imread(imageName.toStdString());
    if (image.empty()) {
        qDebug() << "无法读取图像文件:" << imageName;
        return -1;
    }
    cv::imshow("image", image);

    double minArea = 2000.0;   // 最小面积阈值
    double maxArea = 100000.0; // 最大面积阈值

    cutout.testExtractMultipleObjects(imageName, minArea, maxArea);

    QElapsedTimer timer;
    timer.start();

    std::vector<cv::Mat> boundings;
    for (int t = 0; t < 1; ++t) {
        boundings = cutout.getMultipleObjectsInBoundingRect(image, 30, 50, 3, minArea, maxArea);
    }
    qDebug() << "getMultipleObjectsInBoundingRect elapsed:" << timer.elapsed();

    timer.restart();
    cv::Mat mask;
    for (int t = 0; t < 1; ++t) {
        mask = cutout.getMultipleObjectsInOriginalSize(image, 30, 50, 3, minArea, maxArea);
    }
    qDebug() << "getMultipleObjectsInBoundingRect elapsed:" << timer.elapsed();

    qDebug() << "boundings.size:" << boundings.size();
    int i = 0;
    for (auto &mat : boundings) {
        cv::imshow(QString("bounding %1").arg(i).toStdString(), mat);
        ++i;
    }

    cv::imshow(QString("mask %1").arg(i).toStdString(), mask);

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}
