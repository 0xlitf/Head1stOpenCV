#ifndef HUMOMENTSMATCHER_H
#define HUMOMENTSMATCHER_H

#pragma execution_character_set("utf-8")

#include "fileutils.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QPixmap>
#include <QString>
#include <opencv2/opencv.hpp>

class HuMomentsMatcher {
public:
    HuMomentsMatcher();
    void addTemplateIntoMap(const QString &name, const QString &huStr);

    // 辅助函数：将 cv::Mat 转换为 QPixmap 用于显示
    QPixmap cvMatToQPixmap(const cv::Mat &inMat);

    // 辅助函数：获取最大的轮廓
    std::vector<cv::Point> findLargestContour(const cv::Mat &srcInfo,
                                              bool isTemplate);

    cv::Mat addTemplate(const QString &fileName);

    QString calcHuMoments(std::vector<cv::Point> contour);

    cv::Mat croppedCanvas(cv::Mat templateImg, std::vector<cv::Point> contour);

    // C:\GitHub\Head1stOpenCV\OpenCV-Cpp-2.4.9\HuMoments\dataset_foler
    void setTemplateFolder(const QString &folderName);

    void matchImage(const QString &fileName) {
        if (fileName.isEmpty()) {
            qDebug() << "matchImage fileName isEmpty";
            return;
        }

        auto imageMat = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
        if (imageMat.empty())
            return;

        this->matchMat(imageMat);
    }

    void matchMat(cv::Mat sceneImg) {

        // 1. 场景图像预处理
        cv::Mat grayScene, thrScene;
        cv::cvtColor(sceneImg, grayScene, cv::COLOR_BGR2GRAY);
        cv::threshold(grayScene, thrScene, 240, 255, cv::THRESH_BINARY_INV);

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
            double score = cv::matchShapes(m_templateContour, contours[i],
                                           CV_CONTOURS_MATCH_I1, 0.0);

            qDebug() << "counters index:" << i << ", area:" << area
                     << ", score:" << score;

            // 阈值判定：根据实际情况调整，通常 0.1 - 0.2 是很严格的，0.5 较宽松
            if (score < 0.2) {
                matchCount++;

                // C. 获取旋转矩形 (RotatedRect)
                cv::RotatedRect rotRect = cv::minAreaRect(contours[i]);

                // D. 绘制旋转矩形
                cv::Point2f vertices[4];
                rotRect.points(vertices);
                for (int j = 0; j < 4; j++) {
                    cv::line(resultImg, vertices[j], vertices[(j + 1) % 4],
                             cv::Scalar(0, 255, 0), 3);
                }

                // E. 绘制中心点和角度文字
                cv::circle(resultImg, rotRect.center, 5, cv::Scalar(0, 0, 255), -1);

                // std::string text = "Ang: " + std::to_string((int)rotRect.angle);
                // cv::putText(resultImg, text, rotRect.center, // rotRect.center +
                // cv::Point2f(40, 40)
                //             cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

                qDebug() <<
                    QString("发现目标 -> 相似度: %1, 角度: %2, 坐标: (%3, %4)")
                        .arg(score)
                        .arg(rotRect.angle)
                        .arg(rotRect.center.x)
                        .arg(rotRect.center.y);
            } else {
                // 可选：绘制不匹配的轮廓为红色，方便调试
                // cv::drawContours(resultImg, contours, (int)i, cv::Scalar(0, 0, 255),
                // 1);
            }
        }

        if (matchCount == 0) {
            qDebug() << "未在场景中找到匹配物体。";
        }
    }

private:
    std::vector<cv::Point> m_templateContour; // 存储提取出的模板轮廓

    // QMap<QString, QList<cv::Mat>> m_humomentsList;
    QList<std::tuple<QString, QString>> m_humomentsList;
};

#endif // HUMOMENTSMATCHER_H
