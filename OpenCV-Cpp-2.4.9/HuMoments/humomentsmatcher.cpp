#include "humomentsmatcher.h"

HuMomentsMatcher::HuMomentsMatcher() {}

void HuMomentsMatcher::addTemplateIntoMap(const QString &name, cv::Mat mat) {
    QList<cv::Mat> &matList = m_matMap[name];
    matList.append(mat.clone());

    qDebug() << "m_matMap:";
    for (int i = 0; i < m_matMap.size(); ++i) {
        qDebug() << i << ", key, value.size: " << m_matMap.keys()[i]
                 << m_matMap.values()[i].size();
    }
}

QPixmap HuMomentsMatcher::cvMatToQPixmap(const cv::Mat &inMat) {
    if (inMat.empty())
        return QPixmap();

    // 转换颜色空间 BGR -> RGB
    cv::Mat temp;
    if (inMat.channels() == 3) {
        cv::cvtColor(inMat, temp, cv::COLOR_BGR2RGB);
    } else if (inMat.channels() == 1) {
        cv::cvtColor(inMat, temp, cv::COLOR_GRAY2RGB);
    } else {
        return QPixmap();
    }

    QImage img((const uchar *)temp.data, temp.cols, temp.rows, temp.step,
               QImage::Format_RGB888);
    // bits() 只是浅拷贝，必须 deep copy 才能让 QPixmap 在 cv::Mat 释放后继续存在
    img.bits();
    return QPixmap::fromImage(img.copy());
}

// 核心：寻找最大轮廓
std::vector<cv::Point> HuMomentsMatcher::findLargestContour(const cv::Mat &src,
                                                            bool isTemplate) {
    cv::Mat thr;
    // 背光图片：物体黑(0)，背景白(255)。
    // 使用 THRESH_BINARY_INV 将物体变成白色(255)，背景变成黑色(0)
    // 这样 findContours 才能正确找到物体
    cv::threshold(src, thr, 240, 255, cv::THRESH_BINARY_INV);

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

cv::Mat HuMomentsMatcher::addTemplate(const QString &fileName) {

    // 读取灰度图
    auto templateImg = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (templateImg.empty()) {
        qDebug() << "templateImg is empty: " << fileName;
        return cv::Mat();
    } else {
        auto getFolderName = [](const QString& fileName){
            QFileInfo fileInfo(fileName);

            if (!fileInfo.exists()) {
                qDebug() << "文件不存在:" << fileName;
                return QString();
            }

            // 获取文件所在目录的绝对路径
            QString dirPath = fileInfo.absolutePath();

            // 获取目录名
            QDir dir(dirPath);
            return dir.dirName();
        };
        auto folderName = getFolderName(fileName);
        this->addTemplateIntoMap(folderName, templateImg);
    }

    return templateImg;
}
