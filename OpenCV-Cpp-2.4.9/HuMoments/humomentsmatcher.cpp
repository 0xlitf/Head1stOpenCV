#include "humomentsmatcher.h"

HuMomentsMatcher::HuMomentsMatcher(QObject *parent) : QObject(parent) {}

void HuMomentsMatcher::addTemplate(const QString &fileName) {

    // 读取灰度图
    auto templateImg = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (templateImg.empty()) {
        qDebug() << "templateImg is empty: " << fileName;
        emit errorOccured(IMAGE_LOAD_FAILED, QString("templateImg is empty: %1").arg(fileName));
        return;
    } else {
        auto folderName = FileUtils::getFolderBaseName(fileName);

        cv::threshold(templateImg, templateImg, m_whiteThreshold, 255, cv::THRESH_BINARY);

        auto templateContour = this->findLargestContour(templateImg, true);

        QString huStr;
        if (!templateContour.empty()) {
            huStr = this->calcHuMoments(templateContour);
        } else {
            qDebug() << "calcHuMoments failed: templateContour is empty";
            emit errorOccured(NO_CONTOURS_FOUND, QString("calcHuMoments failed: templateContour is empty: %1").arg(fileName));
        }

        this->addTemplateIntoMap(folderName, huStr, templateContour);
    }
}

void HuMomentsMatcher::addTemplateIntoMap(const QString &name, const QString& huStr, std::vector<cv::Point> contour) {

    auto tuple = std::make_tuple(name, huStr, contour);
    m_huMomentsList.append(tuple);
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
    cv::threshold(src, thr, m_whiteThreshold, 255, cv::THRESH_BINARY_INV);

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

void HuMomentsMatcher::setTemplateFolder(const QString &folderName) {
    auto imageFilenames = FileUtils::findAllImageFiles(folderName);
    for (auto& filename: imageFilenames) {
        this->addTemplate(filename);
    }

    auto folderNames = FileUtils::findDepth1Folder(folderName);

    emit sendLog(QString("setTemplateFolder: %1\ntemplate folders count: %2\ntemplate images count: %3").arg(folderName).arg(folderNames.size()).arg(m_huMomentsList.size()));

    qDebug() << "m_humomentsList:";
    for (int i = 0; i < m_huMomentsList.size(); ++i) {
        auto tuple = m_huMomentsList[i];
        qDebug() << i << ", key, value.size: " << std::get<0>(tuple)
                 << std::get<1>(tuple);
    }
}

void HuMomentsMatcher::matchImage(const QString &fileName) {
    if (fileName.isEmpty()) {
        qDebug() << "matchImage fileName isEmpty";
        emit errorOccured(IMAGE_LOAD_FAILED, QString("matchImage fileName isEmpty: %1").arg(fileName));
        return;
    }

    emit sendLog(QString("matchImage: %1").arg(fileName));

    auto imageMat = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
    if (imageMat.empty())
        return;

    this->matchMat(imageMat);
}

void HuMomentsMatcher::matchMat(cv::Mat sceneImg) {

    // 1. 场景图像预处理
    cv::Mat grayScene, thrScene;
    cv::cvtColor(sceneImg, grayScene, cv::COLOR_BGR2GRAY);
    cv::threshold(grayScene, thrScene, m_whiteThreshold, 255, cv::THRESH_BINARY_INV);

    // 2. 提取场景所有轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thrScene, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    qDebug() << "findContours contours.size: " << contours.size();

    // 3. 复制一份场景图用于绘制结果
    cv::Mat resultImg = sceneImg.clone();
    if (!contours.empty()) {
        // 青色在BGR中是 (255, 255, 0)
        cv::Scalar cyanColor(255, 255, 0); // B=255, G=255, R=0

        // 绘制所有轮廓
        cv::drawContours(resultImg, contours,
                         -1,        // 绘制所有轮廓
                         cyanColor, // 青色
                         2,         // 线宽
                         CV_AA);    // 抗锯齿

        qDebug() << "已绘制" << contours.size() << "个轮廓";
    } else {
        qDebug() << "未找到任何轮廓";
    }

    int matchCount = 0;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);

        // A. 简单的面积过滤，排除极小的噪点
        if (area < 300) {
            // qDebug() << "counters index:" << i << ", area:" << area << " < 500";
            continue;
        }

        // B. 形状匹配 (OpenCV matchShapes)
        // 返回值越小越相似。0 表示完全一样。

        for (int j = 0; j < m_huMomentsList.size(); ++j) {
            auto contour = std::get<2>(m_huMomentsList[j]);

            double score = cv::matchShapes(contour, contours[i],
                                           CV_CONTOURS_MATCH_I1, 0.0);

            qDebug() << "counters index:" << j << ", area:" << area
                     << ", score:" << score;

            emit sendLog(QString("counters index: %1, area: %2, score: %3").arg(j).arg(area).arg(score));

            // 阈值判定：根据实际情况调整，通常 0.1 - 0.2 是很严格的，0.5 较宽松
            if (score < m_scoreThreshold) {
                auto objName = std::get<0>(m_huMomentsList[j]);
                matchCount++;

                // C. 获取旋转矩形 (RotatedRect)
                cv::RotatedRect rotRect = cv::minAreaRect(contours[j]);

                // D. 绘制旋转矩形
                cv::Point2f vertices[4];
                rotRect.points(vertices);
                for (int k = 0; k < 4; k++) {
                    cv::line(resultImg, vertices[k], vertices[(k + 1) % 4],
                             cv::Scalar(0, 255, 0), 3);
                }

                // E. 绘制中心点和角度文字
                cv::circle(resultImg, rotRect.center, 5, cv::Scalar(0, 0, 255), -1);

                // std::string text = "Ang: " + std::to_string((int)rotRect.angle);
                // cv::putText(resultImg, text, rotRect.center, // rotRect.center +
                // cv::Point2f(40, 40)
                //             cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

                auto str = QString("发现目标 %1 -> 相似度(越小越好): %2, 角度: %3, "
                                   "坐标: (%4, %5)")
                               .arg(objName)
                               .arg(score)
                               .arg(rotRect.angle)
                               .arg(rotRect.center.x)
                               .arg(rotRect.center.y);
                qDebug() << str;

                emit sendLog(str);

                break;
            } else {
                // 可选：绘制不匹配的轮廓为红色，方便调试
                // cv::drawContours(resultImg, contours, (int)i, cv::Scalar(0, 0, 255),
                // 1);
            }
        }
    }

    if (matchCount == 0) {
        qDebug() << "未在场景中找到匹配物体。";
    }
}
