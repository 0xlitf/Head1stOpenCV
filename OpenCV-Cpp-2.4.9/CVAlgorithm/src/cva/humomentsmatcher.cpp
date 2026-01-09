#include "humomentsmatcher.h"

#include <QElapsedTimer>
#include <QImageReader>

HuMomentsMatcher::HuMomentsMatcher(QObject *parent) : QObject(parent) {}

std::tuple<int, cv::Mat>
HuMomentsMatcher::analyzeAndDrawContour(const cv::Mat &inputImage) {
    if (inputImage.empty()) {
        cv::Mat emptyResult(300, 400, CV_8UC3, cv::Scalar(0, 0, 0));
        cv::putText(emptyResult, "输入图像为空", cv::Point(50, 150),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        return std::make_tuple(0, emptyResult);
    }

    // 2. 创建输出图像
    cv::Mat outputImage;
    if (inputImage.channels() == 1) {
        cv::cvtColor(inputImage, outputImage, cv::COLOR_GRAY2BGR);
    } else {
        outputImage = inputImage.clone();
    }

    // 3. 转换为灰度图
    cv::Mat grayImage;
    if (inputImage.channels() == 3) {
        cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);
    } else {
        grayImage = inputImage.clone();
    }

    // 4. 二值化处理
    cv::Mat binaryImage;
    cv::threshold(grayImage, binaryImage, 200, 255, cv::THRESH_BINARY_INV);

    // cv::imshow("binaryImage", binaryImage);

    // 5. 查找轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binaryImage, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    qDebug() << "contours.size()" << contours.size();

    int contourCount = static_cast<int>(contours.size());

    // 6. 根据轮廓数量选择颜色
    cv::Scalar contourColor;
    if (contourCount > 1) {
        contourColor = cv::Scalar(0, 0, 255); // 红色 (BGR) - 多个轮廓
    } else if (contourCount == 1) {
        contourColor = cv::Scalar(0, 255, 0); // 绿色 (BGR) - 单个轮廓
    } else {
        contourColor = cv::Scalar(255, 255, 255); // 白色 - 无轮廓
    }

    // 7. 绘制轮廓线
    for (size_t i = 0; i < contours.size(); ++i) {
        if (contours[i].empty() || contours[i].size() < 3)
            continue;

        // 绘制轮廓线
        cv::drawContours(outputImage, contours, static_cast<int>(i),
                         contourColor, // 轮廓颜色
                         2,            // 线宽
                         CV_AA);       // 抗锯齿

        if (bool drawContourInfo = false) {
            // 可选：添加轮廓信息
            double area = cv::contourArea(contours[i]);

            // 计算轮廓中心点
            cv::Moments m = cv::moments(contours[i]);
            if (m.m00 != 0) {
                int centerX = static_cast<int>(m.m10 / m.m00);
                int centerY = static_cast<int>(m.m01 / m.m00);

                std::string info = "C" + std::to_string(i + 1) +
                                   " A:" + std::to_string(static_cast<int>(area));

                // 绘制中心点
                cv::circle(outputImage, cv::Point(centerX, centerY), 4, contourColor,
                           -1);

                // 添加文本信息
                cv::Point textPos(centerX + 10, centerY);

                // 绘制文本背景
                int baseline = 0;
                cv::Size textSize =
                    cv::getTextSize(info, cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseline);
                cv::rectangle(outputImage, textPos - cv::Point(2, textSize.height + 2),
                              textPos + cv::Point(textSize.width + 2, 2),
                              cv::Scalar(0, 0, 0), -1);

                cv::putText(outputImage, info, textPos, cv::FONT_HERSHEY_SIMPLEX, 0.4,
                            cv::Scalar(125, 125, 125), 1);
            }
        }
    }

    std::string statusText;
    if (contourCount > 1) {
        statusText = "检测到 " + std::to_string(contourCount) + " 个轮廓 (红色)";
    } else if (contourCount == 1) {
        statusText = "检测到 1 个轮廓 (绿色)";
    } else {
        statusText = "未检测到轮廓";
    }

    // cv::putText(outputImage, statusText, cv::Point(10, 30),
    //             cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(125, 125, 125), 2);

    return std::make_tuple(contourCount, outputImage);
}

void HuMomentsMatcher::setWhiteThreshold(int thres) {
    m_whiteThreshold = thres;
}

int HuMomentsMatcher::whiteThreshold() const { return m_whiteThreshold; }

double HuMomentsMatcher::scoreThreshold() const { return m_scoreThreshold; }

void HuMomentsMatcher::setScoreThreshold(double newScoreThreshold) {
    m_scoreThreshold = newScoreThreshold;
}

void HuMomentsMatcher::addTemplate(const QString &desc,
                                   const QString &fileName) {

    // 读取灰度图
    auto templateImg = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (templateImg.empty()) {
        qDebug() << "templateImg is empty: " << fileName;
        emit errorOccured(IMAGE_LOAD_FAILED,
                          QString("templateImg is empty: %1").arg(fileName));
        return;
    } else {
        // auto folderBaseName = FileUtils::getFolderBaseName(fileName);

        cv::threshold(templateImg, templateImg, m_whiteThreshold, 255,
                      cv::THRESH_BINARY);

        auto templateContour = this->findLargestContour(templateImg, true);

        QString huStr;
        if (!templateContour.empty()) {
            huStr = this->calcHuMoments(templateContour);
        } else {
            qDebug() << "calcHuMoments failed: templateContour is empty";
            emit errorOccured(
                NO_CONTOURS_FOUND,
                QString("calcHuMoments failed: templateContour is empty: %1")
                    .arg(fileName));
        }

        this->addTemplateIntoMap(desc, fileName, huStr, templateContour);
    }
}

double HuMomentsMatcher::areaThreshold() const
{
    return m_areaThreshold;
}

void HuMomentsMatcher::setAreaThreshold(double newAreaThreshold)
{
    m_areaThreshold = newAreaThreshold;
}

void HuMomentsMatcher::addTemplateIntoMap(const QString &desc,
                                          const QString &fileName,
                                          const QString &huStr,
                                          std::vector<cv::Point> contour) {

    auto tuple = std::make_tuple(desc, fileName, huStr, contour);
    m_huMomentsList.append(tuple);
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
        if (fabs(value) < 1e-20) { // 接近0的处理
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

cv::Mat HuMomentsMatcher::croppedCanvas(cv::Mat templateImg,
                                        std::vector<cv::Point> contour) {
    cv::Mat canvas;
    if (templateImg.channels() == 1) {
        cv::cvtColor(templateImg, canvas, cv::COLOR_GRAY2BGR);
    } else {
        canvas = templateImg.clone(); // 深拷贝
    }

    std::vector<std::vector<cv::Point>> contoursToDraw = {contour};
    cv::drawContours(canvas, contoursToDraw, 0, cv::Scalar(0, 255, 0),
                     2); // 绿色线条

    // 3. 计算轮廓的包围盒 (Bounding Rect)
    cv::Rect boundRect = cv::boundingRect(contour);

    // 4. 增加一点 padding (边距)，防止轮廓紧贴着边缘不好看
    int padding = 10;
    boundRect.x = std::max(0, boundRect.x - padding);
    boundRect.y = std::max(0, boundRect.y - padding);
    boundRect.width =
        std::min(canvas.cols - boundRect.x, boundRect.width + 2 * padding);
    boundRect.height =
        std::min(canvas.rows - boundRect.y, boundRect.height + 2 * padding);

    // 5. 裁剪图像 (ROI - Region of Interest)
    cv::Mat croppedCanvas = canvas(boundRect);

    return croppedCanvas;
}

void HuMomentsMatcher::setTemplateFolder(const QStringList &descStrs,
                                         const QStringList &folderNames) {
    if (descStrs.size() != folderNames.size()) {
        qFatal("setTemplateFolder descStrs.size != folderNames.size");
        return;
    }

    m_huMomentsList.clear();

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

QList<MatchResult> HuMomentsMatcher::quickMatchImage(const QString &fileName) {
    QList<MatchResult> tuple;

    if (fileName.isEmpty()) {
        qDebug() << "matchImage fileName isEmpty";
        emit errorOccured(IMAGE_LOAD_FAILED,
                          QString("matchImage fileName isEmpty: %1").arg(fileName));
        return tuple;
    }

    emit sendLog(QString("matchImage: %1").arg(fileName));

    cv::Mat imageMat = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
    if (imageMat.empty())
        return tuple;

    return this->quickMatchMat(imageMat);
}

QList<MatchResult> HuMomentsMatcher::quickMatchMat(cv::Mat sceneImg) {
    // 输入1通道黑底白物
    // cv::imshow("sceneImg", sceneImg);

    QElapsedTimer timer;
    timer.start();

    // 2. 提取场景所有轮廓
    std::vector<std::vector<cv::Point>> contoursInScene;
    cv::findContours(sceneImg, contoursInScene, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);
    qDebug() << "findContours contours.size: " << contoursInScene.size();

    cv::Mat resultImg = sceneImg.clone();
    if (auto showContoursImage = false) {

        // 3. 复制一份场景图用于绘制结果
        if (!contoursInScene.empty()) {
            // 青色在BGR中是 (255, 255, 0)
            cv::Scalar cyanColor(255, 255, 0); // B=255, G=255, R=0

            // 绘制所有轮廓
            cv::drawContours(resultImg, contoursInScene,
                             -1,        // 绘制所有轮廓
                             cyanColor, // 青色
                             2,         // 线宽
                             CV_AA);    // 抗锯齿

            qDebug() << "已绘制" << contoursInScene.size() << "个轮廓";
        } else {
            qDebug() << "未找到任何轮廓";
        }
        cv::imshow("drawContours", resultImg);
    }

    QList<MatchResult> resultList;

    int matchCount = 0;
    for (size_t i = 0; i < contoursInScene.size(); i++) {
        auto objContourInScene = contoursInScene[i];
        double objArea = cv::contourArea(objContourInScene);

        // A. 简单的面积过滤，排除极小的噪点
        if (objArea < 300) {
            // qDebug() << "counters index:" << i << ", area:" << area << " < 500";
            continue;
        }

        // 计算轮廓中心点
        cv::Moments m = cv::moments(objContourInScene);
        cv::Point2f center(m.m10 / m.m00, m.m01 / m.m00);

        // B. 形状匹配 (OpenCV matchShapes)
        // 返回值越小越相似。0 表示完全一样。

        bool finded = false;

        for (int j = 0; j < m_huMomentsList.size(); ++j) {
            auto templateTuple = m_huMomentsList[j];
            auto templateName = std::get<0>(templateTuple);
            auto templateContour = std::get<3>(templateTuple);
            double templateArea = cv::contourArea(templateContour);

            double areaDifferencePercent = (templateArea - objArea) / objArea;
            if (qAbs(areaDifferencePercent) > m_areaThreshold) {
                continue;
            } else {
                qDebug() << "templateName:" << templateName << "areaDifferencePercent:" << areaDifferencePercent;
            }

            double score = cv::matchShapes(templateContour, objContourInScene,
                                           CV_CONTOURS_MATCH_I1, 0.0);

            qDebug() << "counters index:" << j << ", templateArea:" << templateArea
                     << ", matchShapes score:" << score;

            // emit sendLog(QString("counters index: %1, area: %2, score:
            // %3").arg(j).arg(area).arg(score));

            // 阈值判定：根据实际情况调整，通常 0.1 - 0.2 是很严格的，0.5 较宽松
            if (score < m_scoreThreshold) {
                auto objName = std::get<0>(templateTuple);
                matchCount++;

                // C. 获取旋转矩形 (RotatedRect)
                cv::RotatedRect rotRect = cv::minAreaRect(objContourInScene);

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
                                   "坐标: (%4, %5), 面积差值百分比: %6")
                               .arg(objName)
                               .arg(score)
                               .arg(rotRect.angle)
                               .arg(rotRect.center.x)
                               .arg(rotRect.center.y)
                               .arg(areaDifferencePercent);
                qDebug() << str.toUtf8().constData();

                emit sendLog(str);

                finded = true;

                resultList.append(std::make_tuple(objName, objContourInScene, center,
                                                  score, areaDifferencePercent));

                break;
            } else {
                // cv::drawContours(resultImg, contours, (int)i, cv::Scalar(0, 0, 255), 1);
            }
        }

        if (!finded) {
            resultList.append(
                std::make_tuple(QString(""), objContourInScene, center, 100, -100.));
        }
    }

    if (matchCount == 0) {
        qDebug() << "未在场景中找到匹配物体。";
    }

    qDebug() << "matchMat elapsed:" << timer.nsecsElapsed() << ", matchCount:" << matchCount << ", resultList.count:" << resultList.count();

    return resultList;
}

cv::Mat HuMomentsMatcher::drawResultOnImage(const cv::Mat &inputImage,
                                             const MatchResult &result) {
    // 创建输出图像（复制原始图像）
    cv::Mat outputImage = inputImage.clone();

    // 如果原始图像是灰度图，转换为彩色以便绘制
    if (outputImage.channels() == 1) {
        cv::cvtColor(outputImage, outputImage, cv::COLOR_GRAY2BGR);
    }

    // 定义颜色数组
    std::vector<cv::Scalar> colors = {
        cv::Scalar(0, 255, 0),   // 绿色
        cv::Scalar(255, 0, 0),   // 蓝色
        cv::Scalar(0, 0, 255),   // 红色
        cv::Scalar(255, 255, 0), // 青色
        cv::Scalar(255, 0, 255), // 洋红
        cv::Scalar(0, 255, 255)  // 黄色
    };

    int colorIndex = 0;

    QString label = std::get<0>(result);
    std::vector<cv::Point> contour = std::get<1>(result);

    if (contour.empty()) {
        qDebug() << "contour is empty";
        return outputImage;
    }

    // 选择颜色
    cv::Scalar color = colors[colorIndex % colors.size()];
    // cv::Scalar color = cv::Scalar(255, 255, 255);
    colorIndex++;

    // 绘制轮廓
    cv::drawContours(outputImage, std::vector<std::vector<cv::Point>>{contour}, -1, color, 2, CV_AA);

    // 计算轮廓中心点用于放置标签
    cv::Moments m = cv::moments(contour);
    if (m.m00 != 0) {
        int centerX = static_cast<int>(m.m10 / m.m00);
        int centerY = static_cast<int>(m.m01 / m.m00);

        // 绘制标签
        drawLabel(outputImage, label, cv::Point(centerX, centerY), color);
    }

    return outputImage;
}

cv::Mat HuMomentsMatcher::drawResultsOnImage(const cv::Mat &inputImage,
                                     const QList<MatchResult> &resultList) {
    // 创建输出图像（复制原始图像）
    cv::Mat outputImage = inputImage.clone();

    // 如果原始图像是灰度图，转换为彩色以便绘制
    if (outputImage.channels() == 1) {
        cv::cvtColor(outputImage, outputImage, cv::COLOR_GRAY2BGR);
    }

    // 定义颜色数组
    std::vector<cv::Scalar> colors = {
        cv::Scalar(0, 255, 0),   // 绿色
        cv::Scalar(255, 0, 0),   // 蓝色
        cv::Scalar(0, 0, 255),   // 红色
        cv::Scalar(255, 255, 0), // 青色
        cv::Scalar(255, 0, 255), // 洋红
        cv::Scalar(0, 255, 255)  // 黄色
    };

    int colorIndex = 0;

    // 遍历所有结果
    for (const auto &result : resultList) {
        QString label = std::get<0>(result);
        std::vector<cv::Point> contour = std::get<1>(result);

        if (contour.empty()) {
            qDebug() << "contour is empty";
            continue;
        }

        // 选择颜色
        cv::Scalar color = colors[colorIndex % colors.size()];
        // cv::Scalar color = cv::Scalar(255, 255, 255);
        colorIndex++;

        // 绘制轮廓
        cv::drawContours(outputImage, std::vector<std::vector<cv::Point>>{contour}, -1, color, 2, CV_AA);

        // 计算轮廓中心点用于放置标签
        cv::Moments m = cv::moments(contour);
        if (m.m00 != 0) {
            int centerX = static_cast<int>(m.m10 / m.m00);
            int centerY = static_cast<int>(m.m01 / m.m00);

            // 绘制标签
            drawLabel(outputImage, label, cv::Point(centerX, centerY), color);
        }
    }

    return outputImage;
}

void HuMomentsMatcher::drawLabel(cv::Mat &image, const QString &label,
                                 const cv::Point &position,
                                 const cv::Scalar &color) {
    std::string text = label.toStdString();

    // 计算文本尺寸
    int baseline = 0;
    cv::Size textSize =
        cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);

    // 绘制背景矩形
    cv::rectangle(image,
                  cv::Point(position.x - 5, position.y - textSize.height - 5),
                  cv::Point(position.x + textSize.width + 5, position.y + 5),
                  cv::Scalar(0, 0, 0), -1); // 黑色背景

    // 绘制文本
    cv::putText(image, text, cv::Point(position.x, position.y),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1);
}

cv::Mat HuMomentsMatcher::binaryProcess(cv::Mat inputImage) {
    // 1. 检查输入图像是否有效
    if (inputImage.empty()) {
        qDebug() << "错误：binaryProcess 接收到空图像！";
        return cv::Mat(); // 返回空矩阵表示错误
    }

    cv::Mat grayScene, thrScene;

    // 2. 根据输入图像的通道数进行不同处理
    int channels = inputImage.channels();
    qDebug() << "输入图像通道数：" << channels;

    if (channels == 1) {
        // 情况1：输入已经是单通道图像（例如灰度图或二值图）
        grayScene = inputImage; // 直接使用，无需转换
        qDebug() << "输入为单通道图像，直接进行二值化。";
    }
    else if (channels == 3 || channels == 4) {
        // 情况2：输入是3通道（BGR）或4通道（如BGRA）图像
        // 将彩色图像转换为灰度图[2,5](@ref)
        cv::cvtColor(inputImage, grayScene, cv::COLOR_BGR2GRAY);
        qDebug() << "已将" << channels << "通道图像转换为灰度图。";
    }
    else {
        // 情况3：不支持的通道数（如2通道）
        qDebug() << "错误：不支持的图像通道数：" << channels;
        return cv::Mat(); // 返回空矩阵表示错误
    }

    // 3. 对灰度图进行二值化[4,7](@ref)
    cv::threshold(grayScene, thrScene, m_whiteThreshold, 255, cv::THRESH_BINARY_INV);

    // 4. 返回二值化结果（根据您的函数名和逻辑，应该返回thrScene）
    return thrScene;
}
