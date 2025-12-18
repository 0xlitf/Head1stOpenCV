#include "humomentsmatcher.h"

HuMomentsMatcher::HuMomentsMatcher() {}

cv::Mat HuMomentsMatcher::addTemplate(const QString &fileName) {

    // 读取灰度图
    auto templateImg = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (templateImg.empty()) {
        qDebug() << "templateImg is empty: " << fileName;
        return cv::Mat();
    } else {
        auto folderName = FileUtils::getFolderBaseName(fileName);
        this->addTemplateIntoMap(folderName, templateImg);
    }

    return templateImg;
}

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

QString HuMomentsMatcher::calcHuMoments(std::vector<cv::Point> contour) {
    cv::Moments moms = cv::moments(contour);
    double hu[7];
    cv::HuMoments(moms, hu);

    QString huStr;
    for (int i = 0; i < 7; i++) {
        double value = hu[i];
        double result;
        // 使用 Log 变换方便查看数量级
        // 取Hu矩的绝对值
        // 以10为底的对数
        // 保存原始hu[i]的符号
        if (fabs(value) < 1e-20) {  // 接近0的处理
            result = 0.0;
        } else {
            result = -1 * copysign(1.0, value) * log10(fabs(value));
        }
        // huStr += QString::number(-1 * copysign(1.0, hu[i]) * log10(abs(hu[i])),
        //                          'f', 2) +
        //          " ";
        huStr += QString::number(result, 'f', 2) + " ";
    }
    return huStr;
}

cv::Mat HuMomentsMatcher::croppedCanvas(cv::Mat templateImg, std::vector<cv::Point> contour) {
    cv::Mat canvas;
    if (templateImg.channels() == 1) {
        cv::cvtColor(templateImg, canvas, cv::COLOR_GRAY2BGR);
    } else {
        canvas = templateImg.clone();  // 深拷贝
    }

    std::vector<std::vector<cv::Point>> contoursToDraw = { contour };
    cv::drawContours(canvas, contoursToDraw, 0, cv::Scalar(0, 255, 0), 2); // 绿色线条

    // 3. 计算轮廓的包围盒 (Bounding Rect)
    cv::Rect boundRect = cv::boundingRect(contour);

    // 4. 增加一点 padding (边距)，防止轮廓紧贴着边缘不好看
    int padding = 10;
    boundRect.x = std::max(0, boundRect.x - padding);
    boundRect.y = std::max(0, boundRect.y - padding);
    boundRect.width = std::min(canvas.cols - boundRect.x, boundRect.width + 2 * padding);
    boundRect.height = std::min(canvas.rows - boundRect.y, boundRect.height + 2 * padding);

    // 5. 裁剪图像 (ROI - Region of Interest)
    cv::Mat croppedCanvas = canvas(boundRect);

    return croppedCanvas;
}
