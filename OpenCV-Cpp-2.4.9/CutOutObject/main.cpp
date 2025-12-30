
#pragma execution_character_set("utf-8")

#include "mainwindow.h"

#include <QApplication>
#include <QFile>
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


QString getImageFormatBySignature(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return "无法打开文件";
    }

    QByteArray header = file.read(8); // 读取前8个字节通常足够识别常见格式
    file.close();

    if (header.startsWith("\x89PNG\x0D\x0A\x1A\x0A")) {
        return "PNG";
    } else if (header.startsWith("\xFF\xD8\xFF")) {
        return "JPEG";
    } else if (header.startsWith("BM")) {
        return "BMP";
    } else if (header.startsWith("GIF8")) {
        return "GIF";
    } else if (header.startsWith("II") || header.startsWith("MM")) {
        return "TIFF";
    } else {
        return "未知格式";
    }
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
    // QString imageName = QString(PROJECT_DIR) + "/cannotdetectobject.png";
    // QString imageName = QString(PROJECT_DIR) + "/CutOutObjectd_Z6YHwZteAM.png";
    // QString imageName = QString(PROJECT_DIR) + "/Cam1_18-32-25-052.png";

    // QString imageName = "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/CVAlgorithm/dataset_blueconveyor/14-30-00-395.png";

    qDebug() << "imageName:" << imageName;

    QString trueFormat = getImageFormatBySignature(imageName);
    qDebug() << "图像的真实格式是：" << trueFormat;

    auto image = cv::imread(imageName.toStdString());
    if (image.empty()) {
        qDebug() << "无法读取图像文件:" << imageName;
        return -1;
    } else {
        int width = image.cols;  // 图像宽度（像素）
        int height = image.rows; // 图像高度（像素）
        int channels = image.channels(); // 通道数[6](@ref)

        qDebug() << "=== 图像信息 ===";
        qDebug() << "图像路径:" << imageName;
        qDebug() << "宽度:" << width << "像素";
        qDebug() << "高度:" << height << "像素";
        qDebug() << "通道数:" << channels;

        int depth = image.depth();
        QString depthStr;
        switch (depth) {
        case CV_8U: depthStr = "8位无符号整数"; break;
        case CV_32F: depthStr = "32位浮点数"; break;
        // ... 其他深度类型
        default: depthStr = "其他";
        }
        qDebug() << "位图深度:" << depthStr.toUtf8().constData();
    }
    cv::imshow("image", image);

    cv::Mat eraseBlueBackground;
    cv::Mat singleChannelZeroImage;
    std::tie(eraseBlueBackground, singleChannelZeroImage) = cutout.eraseBlueBackground(image, 30, 50);
    cv::imshow("eraseBlueBackground", eraseBlueBackground);

    double minArea = 1000.0;   // 最小面积阈值
    double maxArea = 1000000.0; // 最大面积阈值


    std::vector<ObjectDetectionResult> results = cutout.extractMultipleObjects(singleChannelZeroImage, minArea, maxArea);


    QElapsedTimer timer;
    timer.start();

    std::vector<cv::Mat> boundings;
    for (int t = 0; t < 1; ++t) {
        boundings = cutout.getMultipleObjectsInBoundingRect(results);
    }

    qDebug() << "boundings.size:" << boundings.size();
    int i = 0;
    for (auto &mat : boundings) {
        cv::imshow(QString("bounding %1").arg(i).toStdString(), mat);
        ++i;
    }
    qDebug() << "getMultipleObjectsInBoundingRect elapsed:" << timer.elapsed();

    timer.restart();
    cv::Mat resultImg(eraseBlueBackground.size(), CV_8UC3, cv::Scalar(255, 255, 255));
    // cv::Mat mask = cutout.getMultipleObjectsInOriginalSize(results, resultImg);
    cv::Mat mask = cutout.getMultipleObjectsInOriginalSize(results, resultImg);
    cv::imshow(QString("getMultipleObjectsInOriginalSize").toStdString(), mask);
    qDebug() << "getMultipleObjectsInBoundingRect elapsed:" << timer.elapsed();


    cv::Mat objsInfo = cutout.drawObjectsInfo(results, singleChannelZeroImage);

    cv::imshow("objsInfo", objsInfo);

    cv::waitKey(0);
    cv::destroyAllWindows();
    return 0;
}
