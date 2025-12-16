#include "mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include "layoutbuilder.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->createComponents();
}

void MainWindow::createComponents() {
    // --- 1. 构建 UI 界面 ---
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 顶部按钮区
    btnLoadTemplate = new QPushButton("1. 加载模板图像", this);
    btnLoadScene = new QPushButton("2. 加载检测图像", this);
    btnMatch = new QPushButton("3. 开始识别与定位", this);

    auto btnLayout = Layouting::Row{btnLoadTemplate, btnLoadScene, btnMatch};

    // 中间图像显示区
    QHBoxLayout *imgLayout = new QHBoxLayout();
    lblTemplateDisplay = new QLabel("模板预览", this);
    lblTemplateDisplay->setAlignment(Qt::AlignCenter);
    lblTemplateDisplay->setStyleSheet(
        "border: 1px solid gray; background: #eee;");
    lblTemplateDisplay->setMinimumSize(300, 300);

    lblSceneDisplay = new QLabel("检测场景预览", this);
    lblSceneDisplay->setAlignment(Qt::AlignCenter);
    lblSceneDisplay->setStyleSheet("border: 1px solid gray; background: #eee;");
    lblSceneDisplay->setMinimumSize(500, 300);

    // 让场景图占据更多空间
    imgLayout->addWidget(lblTemplateDisplay, 1);
    imgLayout->addWidget(lblSceneDisplay, 2);

    // 底部日志区
    txtLog = new QTextEdit(this);
    txtLog->setMaximumHeight(150);
    txtLog->setReadOnly(true);

    Layouting::Column{btnLayout, imgLayout, txtLog}.attachTo(centralWidget);

    resize(1000, 700);
    setWindowTitle("OpenCV C++ 形状匹配与定位示例");

    // --- 2. 信号槽连接 ---
    connect(btnLoadTemplate, &QPushButton::clicked, this,
            &MainWindow::onLoadTemplate);
    connect(btnLoadScene, &QPushButton::clicked, this, &MainWindow::onLoadScene);
    connect(btnMatch, &QPushButton::clicked, this, &MainWindow::onRunMatching);
}

MainWindow::~MainWindow() {}

// 辅助：Mat 转 QPixmap
QPixmap MainWindow::cvMatToQPixmap(const cv::Mat &inMat) {
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

void MainWindow::onLoadTemplate() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择模板图片", "",
                                                    "Images (*.png *.jpg *.bmp)");
    if (fileName.isEmpty())
        return;

    // 读取灰度图
    m_templateImg = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (m_templateImg.empty())
        return;

    // 显示原图
    lblTemplateDisplay->setPixmap(
        cvMatToQPixmap(m_templateImg)
            .scaled(lblTemplateDisplay->size(), Qt::KeepAspectRatio));

    // 预处理并提取特征
    m_templateContour = findLargestContour(m_templateImg, true);

    if (!m_templateContour.empty()) {
        // 计算 Hu 矩
        cv::Moments moms = cv::moments(m_templateContour);
        double hu[7];
        cv::HuMoments(moms, hu);

        txtLog->append("<b>[模板加载成功]</b>");
        txtLog->append("Hu 矩特征 (Log Scale):");
        QString huStr;
        for (int i = 0; i < 7; i++) {
            // 使用 Log 变换方便查看数量级
            huStr += QString::number(-1 * copysign(1.0, hu[i]) * log10(abs(hu[i])),
                                     'f', 2) +
                     " ";
        }
        txtLog->append(huStr);
    }
}

void MainWindow::onLoadScene() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择场景图片", "",
                                                    "Images (*.png *.jpg *.bmp)");
    if (fileName.isEmpty())
        return;

    // 这里读取彩色图，方便最后画绿色的框
    m_sceneImg = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
    if (m_sceneImg.empty())
        return;

    lblSceneDisplay->setPixmap(
        cvMatToQPixmap(m_sceneImg)
            .scaled(lblSceneDisplay->size(), Qt::KeepAspectRatio));
    txtLog->append("<b>[场景加载成功]</b> 等待识别...");
}

// 核心：寻找最大轮廓
std::vector<cv::Point> MainWindow::findLargestContour(const cv::Mat &src,
                                                      bool isTemplate) {
    cv::Mat thr;
    // 背光图片：物体黑(0)，背景白(255)。
    // 使用 THRESH_BINARY_INV 将物体变成白色(255)，背景变成黑色(0)
    // 这样 findContours 才能正确找到物体
    cv::threshold(src, thr, 50, 255, cv::THRESH_BINARY_INV);

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

void MainWindow::onRunMatching() {
    if (m_sceneImg.empty() || m_templateContour.empty()) {
        QMessageBox::warning(this, "提示", "请先加载模板和场景图片！");
        return;
    }

    txtLog->append("--- 开始匹配 ---");

    // 1. 场景图像预处理
    cv::Mat grayScene, thrScene;
    cv::cvtColor(m_sceneImg, grayScene, cv::COLOR_BGR2GRAY);
    cv::threshold(grayScene, thrScene, 50, 255, cv::THRESH_BINARY_INV);

    // 2. 提取场景所有轮廓
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thrScene, contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    // 3. 复制一份场景图用于绘制结果
    cv::Mat resultImg = m_sceneImg.clone();

    int matchCount = 0;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);

        // A. 简单的面积过滤，排除极小的噪点
        if (area < 500)
            continue;

        // B. 形状匹配 (OpenCV matchShapes)
        // 返回值越小越相似。0 表示完全一样。
        double score = cv::matchShapes(m_templateContour, contours[i],
                                       cv::CONTOURS_MATCH_I1, 0.0);

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

            std::string text = "Ang: " + std::to_string((int)rotRect.angle);
            cv::putText(resultImg, text, rotRect.center + cv::Point2f(20, 20),
                        cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);

            txtLog->append(QString("发现目标 -> 相似度: %1, 角度: %2, 坐标: (%3, %4)")
                               .arg(score)
                               .arg(rotRect.angle)
                               .arg(rotRect.center.x)
                               .arg(rotRect.center.y));
        } else {
            // 可选：绘制不匹配的轮廓为红色，方便调试
            // cv::drawContours(resultImg, contours, (int)i, cv::Scalar(0, 0, 255),
            // 1);
        }
    }

    if (matchCount == 0) {
        txtLog->append("未在场景中找到匹配物体。");
    }

    // 更新界面显示
    lblSceneDisplay->setPixmap(cvMatToQPixmap(resultImg).scaled(
        lblSceneDisplay->size(), Qt::KeepAspectRatio));
}
