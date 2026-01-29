#include "MainWindow.h"
#include "bgr2hsvconverter.h"
#include "minimumbounding.h"

#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    // 设置窗口属性
    setWindowTitle("HSV缺陷检测系统");
    setMinimumSize(1400, 800);

    // 创建中心部件和布局
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_imagesLayout = new QHBoxLayout();

    // 创建图像显示标签
    m_normalImageLabel = new QLabel("正常图像（未加载）");
    m_defectImageLabel = new QLabel("缺陷图像（未加载）");
    m_resultLabel = new QLabel("");

    // 设置标签样式和属性
    m_normalImageLabel->setAlignment(Qt::AlignCenter);
    m_defectImageLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setAlignment(Qt::AlignCenter);

    m_normalImageLabel->setStyleSheet("border: 2px solid gray; background-color: #f0f0f0;");
    m_defectImageLabel->setStyleSheet("border: 2px solid gray; background-color: #f0f0f0;");
    m_resultLabel->setStyleSheet("border: 1px solid black; background-color: white; padding: 10px;");

    m_normalImageLabel->setMinimumSize(400, 400);
    m_defectImageLabel->setMinimumSize(400, 400);

    // 创建按钮
    m_loadNormalButton = new QPushButton("加载正常图像");
    m_loadDefectButton = new QPushButton("加载缺陷图像");
    m_detectButton = new QPushButton("开始缺陷检测");
    m_detectButton->setEnabled(false); // 初始时禁用检测按钮

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_loadNormalButton);
    buttonLayout->addWidget(m_loadDefectButton);
    buttonLayout->addWidget(m_detectButton);

    // 图像显示布局
    m_imagesLayout->addWidget(m_normalImageLabel);
    m_imagesLayout->addWidget(m_defectImageLabel);

    // 主布局组装
    m_mainLayout->addLayout(buttonLayout);
    m_mainLayout->addLayout(m_imagesLayout);
    m_mainLayout->addWidget(m_resultLabel);

    // 初始化检测器
    m_detector = new DefectDetector(this);

    // 连接信号槽
    connect(m_loadNormalButton, &QPushButton::clicked, this, [=]() {
        this->onLoadNormalImage();

        // QList<QString> imageList;
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (1).png";
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (2).png";
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (3).png";
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (4).png";
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (5).png";
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (6).png";
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (7).png";
        // imageList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/1ok (8).png";

        // MinimumBounding mini;
        // for (auto& imageDir: imageList) {
        //     cv::Mat tInput = cv::imread(imageDir.toStdString());
        //     tInput = mini.findAndCropObject(tInput);
        //     cv::imwrite(imageDir.replace(".png", "_template.png").toStdString(), tInput);
        // }
    });
    connect(m_loadDefectButton, &QPushButton::clicked, this, [=]() { this->onLoadDefectImage(); });
    connect(m_detectButton, &QPushButton::clicked, this, [=]() {
        if (bool single = true) {
            QElapsedTimer timer;
            DefectDetector detector;
            MinimumBounding mini;

            cv::Mat tInput = m_normalImage;
            tInput = mini.findAndCropObject(tInput);

            cv::Mat dInput = m_defectImage;
            dInput = mini.findAndCropObject(dInput);

            cv::pyrDown(tInput, tInput);
            cv::pyrDown(dInput, dInput);

            cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0, cv::INTER_LINEAR);

            cv::imshow("tInput", tInput);
            cv::imshow("dInput", dInput);


            auto tContour = detector.findContours(tInput);
            auto dContour = detector.findContours(dInput);

            timer.start();

            int outterWidth = 1;
            int innerWidth = 4;

            cv::Mat tEdge = detector.processRingEdge(tInput, tContour, outterWidth, innerWidth);
            cv::Mat dEdge = detector.processRingEdge(dInput, tContour, outterWidth, innerWidth);
            double defectScore = detector.matchMat(tEdge, dEdge);
            qDebug() << "defectScore:" << defectScore << ", matchMat elapsed:" << timer.elapsed();

        } else {
            QElapsedTimer timer;

            DefectDetector detector;
            QStringList descList;
            descList << "ok";
            QStringList folderList;
            folderList << "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/template_black"; // template_brown template_black
            detector.setTemplateFolder(descList, folderList);

            cv::Mat dInput = m_defectImage; // cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/2ok (5).png");

            timer.start();
            double defectScore = detector.fullMatchMat(dInput);

            qDebug() << "defectScore:" << defectScore << ", fullMatchMat elapsed:" << timer.elapsed();
        }
    });

    this->loadDefaultImages();
}

MainWindow::~MainWindow() {}

void MainWindow::loadDefaultImages() {
    // 加载默认图像（可选）

    // cv::Mat tInput =
    // cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ok/2026-01-15_16-07-45_741.png");
    // cv::Mat dInput =
    // cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ng/2026-01-15_16-09-20_925.png");

    cv::Mat tInput = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/1/ok/1ok (2).png");
    cv::Mat dInput = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/1/ng/1ng (2).png");

    MinimumBounding mini;
    tInput = mini.findAndCropObject(tInput);
    dInput = mini.findAndCropObject(dInput);

    // cv::imwrite("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/tInput.png", tInput);
    // cv::imwrite("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/build/Desktop_Qt_6_10_1_MSVC2022_64bit-Release/DefectDetection_bin/Windows/Release/5ok.png", dInput);

    // tInput = mini.removeOuterBorder(tInput, 2);
    // dInput = mini.removeOuterBorder(dInput, 2);

    // qDebug() << "tInput.cols rows" << tInput.cols << tInput.rows;
    // qDebug() << "dInput.cols rows" << dInput.cols << dInput.rows;

    // cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0, cv::INTER_LINEAR);

    // qDebug() << "tInput.cols rows" << tInput.cols << tInput.rows;
    // qDebug() << "dInput.cols rows" << dInput.cols << dInput.rows;

    // tInput = mini.fillCenterWithWhite(tInput, m_detectThickness);
    // dInput = mini.fillCenterWithWhite(dInput, m_detectThickness);

    // if (bool showDiff = true) {
    //     cv::Mat diff;
    //     cv::absdiff(tInput, dInput, diff);

    //     cv::imshow("diff", diff);

    //     cv::Mat grayDiff;
    //     if (diff.channels() == 3) {
    //         cv::cvtColor(diff, grayDiff, cv::COLOR_BGR2GRAY);
    //     } else {
    //         grayDiff = diff;
    //     }

    //     cv::imshow("grayDiff", grayDiff);

    //     cv::Mat thresholdDiff;
    //     // 将差异明显的像素设为255（白色），无差异或差异小的设为0（黑色）
    //     cv::threshold(grayDiff, thresholdDiff, 60, 255, cv::THRESH_BINARY);

    //     int whitePixelCount = cv::countNonZero(thresholdDiff);
    //     qDebug() << "白色像素点的个数为: " << whitePixelCount;

    //     cv::imshow("thresholdDiff", thresholdDiff);
    // }

    m_normalImage = tInput;
    m_defectImage = dInput;

    if (!m_normalImage.empty()) {
        displayImageOnLabel(m_normalImageLabel, m_normalImage);
    }
    if (!m_defectImage.empty()) {
        displayImageOnLabel(m_defectImageLabel, m_defectImage);
        m_detectButton->setEnabled(true);
    }
}

void MainWindow::onLoadNormalImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择正常图像", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");
    if (!fileName.isEmpty()) {
        cv::Mat tInput = cv::imread(fileName.toStdString());

        MinimumBounding mini;
        cv::Mat objMat = mini.findAndCropObject(tInput);

        BGR2HSVConverter cvt;

        m_normalImage = objMat;
        if (!m_normalImage.empty()) {
            displayImageOnLabel(m_normalImageLabel, m_normalImage);
            m_normalImageLabel->setText("");

            // 如果两张图都已加载，启用检测按钮
            if (!m_defectImage.empty()) {
                m_detectButton->setEnabled(true);
            }
        } else {
            QMessageBox::warning(this, "错误", "无法加载正常图像！");
        }
    }
}

void MainWindow::onLoadDefectImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择缺陷图像", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");
    if (!fileName.isEmpty()) {
        cv::Mat dInput = cv::imread(fileName.toStdString());

        MinimumBounding mini;
        cv::Mat objMat = mini.findAndCropObject(dInput);

        m_defectImage = objMat;
        if (!m_defectImage.empty()) {
            displayImageOnLabel(m_defectImageLabel, m_defectImage);
            m_defectImageLabel->setText("");

            // 如果两张图都已加载，启用检测按钮
            if (!m_normalImage.empty()) {
                m_detectButton->setEnabled(true);
            }
        } else {
            QMessageBox::warning(this, "错误", "无法加载缺陷图像！");
        }
    }
}

void MainWindow::displayImageOnLabel(QLabel *label, const cv::Mat &image) {
    if (image.empty()) {
        label->setText("图像为空");
        qDebug() << "图像为空";
        return;
    }

    // 1. 将OpenCV Mat安全地转换为QImage，并进行BGR到RGB的转换[1,3](@ref)
    QImage qImage;
    cv::Mat rgbImage;
    if (image.channels() == 3) {
        cv::cvtColor(image, rgbImage, cv::COLOR_BGR2RGB);
        // 使用rgbImage.data创建QImage，此时qImage与rgbImage共享数据[1](@ref)
        qImage = QImage(rgbImage.data, rgbImage.cols, rgbImage.rows, rgbImage.step, QImage::Format_RGB888);
    } else if (image.channels() == 1) {
        qImage = QImage(image.data, image.cols, image.rows, image.step, QImage::Format_Grayscale8);
    } else {
        label->setText("不支持的图像格式");
        return;
    }

    // 2. 将QImage转换为QPixmap
    QPixmap originalPixmap = QPixmap::fromImage(qImage);

    // 3. 关键步骤：缩放Pixmap以适应Label大小，同时保持长宽比[5,8](@ref)
    QPixmap scaledPixmap = originalPixmap.scaled(label->size(), Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation); // 使用平滑变换以获得更好质量

    // 4. 显示图像
    label->setPixmap(scaledPixmap);
    // 非常重要：禁用自动拉伸，由我们自己控制缩放[7,8](@ref)
    label->setScaledContents(false);
}

cv::Mat MainWindow::resizeImageForDisplay(const cv::Mat &image, QLabel *label) {
    if (image.empty())
        return cv::Mat();

    // 获取标签的可用大小
    QSize labelSize = label->size();
    int maxWidth = labelSize.width() - 20; // 留出边距
    int maxHeight = labelSize.height() - 20;

    // 如果图像尺寸小于标签尺寸，直接返回原图
    if (image.cols <= maxWidth && image.rows <= maxHeight) {
        return image.clone();
    }

    // 计算缩放比例，保持宽高比 [3](@ref)
    double scale = std::min(static_cast<double>(maxWidth) / image.cols, static_cast<double>(maxHeight) / image.rows);

    int newWidth = static_cast<int>(image.cols * scale);
    int newHeight = static_cast<int>(image.rows * scale);

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight));
    return resizedImage;
}

void MainWindow::onDetectDefect() {
    if (m_normalImage.empty() || m_defectImage.empty()) {
        QMessageBox::warning(this, "错误", "请先加载正常图像和缺陷图像!");
        return;
    }

    MinimumBounding mini;

    cv::Mat tInput = m_normalImage.clone();
    cv::Mat dInput = m_defectImage.clone();

    cv::pyrDown(tInput, tInput);
    cv::pyrDown(dInput, dInput);

    // blur GaussianBlur medianBlur bilateralFilter
    // cv::blur(tInput, tInput, cv::Size(3, 3));
    // cv::blur(dInput, dInput, cv::Size(3, 3));
    cv::GaussianBlur(tInput, tInput, cv::Size(5, 5), 0, 0);
    cv::GaussianBlur(dInput, dInput, cv::Size(5, 5), 0, 0);
    // cv::medianBlur(tInput, tInput, 5);
    // cv::medianBlur(dInput, dInput, 5);
    // cv::bilateralFilter(tInput, tInput, 9, 50, 10);
    // cv::bilateralFilter(dInput, dInput, 9, 50, 10);


    cv::imshow("m_normalImage origin", m_normalImage);
    cv::imshow("m_defectImage origin", m_defectImage);

    if (!m_normalImage.empty()) {
        displayImageOnLabel(m_normalImageLabel, m_normalImage);
    }
    if (!m_defectImage.empty()) {
        displayImageOnLabel(m_defectImageLabel, m_defectImage);
        m_detectButton->setEnabled(true);
    }

    if (useHSV) {
        BGR2HSVConverter cvt;
        tInput = cvt.convertBGR2HSV(tInput);
        dInput = cvt.convertBGR2HSV(dInput);
    }

    cv::imshow("m_normalImage hsv", tInput);
    cv::imshow("m_defectImage hsv", dInput);

    tInput = mini.removeOuterBorder(tInput, m_removeOuterBorderThickness);
    dInput = mini.removeOuterBorder(dInput, m_removeOuterBorderThickness);

    cv::resize(dInput, dInput, cv::Size(tInput.cols, tInput.rows), 0, 0, cv::INTER_LINEAR);

    tInput = mini.fillCenterWithWhite(tInput, m_detectThickness);
    dInput = mini.fillCenterWithWhite(dInput, m_detectThickness);

    std::vector<cv::Mat> thsvChannels;
    cv::split(tInput, thsvChannels);
    cv::Mat thChannel = thsvChannels[0]; // H通道
    cv::Mat tsChannel = thsvChannels[1]; // S通道
    cv::Mat tvChannel = thsvChannels[2]; // V通道

    // cv::imshow("th", thChannel);
    // cv::imshow("ts", tsChannel);
    // cv::imshow("tv", tvChannel);

    std::vector<cv::Mat> dhsvChannels;
    cv::split(dInput, dhsvChannels);
    cv::Mat dhChannel = dhsvChannels[0]; // H通道
    cv::Mat dsChannel = dhsvChannels[1]; // S通道
    cv::Mat dvChannel = dhsvChannels[2]; // V通道

    // cv::imshow("dh", dhChannel);
    // cv::imshow("ds", dsChannel);
    // cv::imshow("dv", dvChannel);

    cv::Mat tH_BGR, tS_BGR, tV_BGR, dH_BGR, dS_BGR, dV_BGR;
    cv::cvtColor(thChannel, tH_BGR, cv::COLOR_GRAY2BGR);
    cv::cvtColor(tsChannel, tS_BGR, cv::COLOR_GRAY2BGR);
    cv::cvtColor(tvChannel, tV_BGR, cv::COLOR_GRAY2BGR);
    cv::cvtColor(dhChannel, dH_BGR, cv::COLOR_GRAY2BGR);
    cv::cvtColor(dsChannel, dS_BGR, cv::COLOR_GRAY2BGR);
    cv::cvtColor(dvChannel, dV_BGR, cv::COLOR_GRAY2BGR);
    cv::putText(tH_BGR, "T-H", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
    cv::putText(tS_BGR, "T-S", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
    cv::putText(tV_BGR, "T-V", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
    cv::putText(dH_BGR, "D-H", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
    cv::putText(dS_BGR, "D-S", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
    cv::putText(dV_BGR, "D-V", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

    // 3. 分别拼接左右两侧（每侧3个通道垂直堆叠）
    cv::Mat leftCol, rightCol;
    cv::vconcat(std::vector<cv::Mat>{tH_BGR, tS_BGR, tV_BGR}, leftCol); // 左侧垂直拼接
    cv::vconcat(std::vector<cv::Mat>{dH_BGR, dS_BGR, dV_BGR}, rightCol); // 右侧垂直拼接

    // 4. 将左右两侧水平拼接为最终图像
    cv::Mat concatResult;
    cv::hconcat(leftCol, rightCol, concatResult);

    // 5. 显示最终拼接结果
    cv::imshow("HSV Channels Comparison (Left: Template, Right: Detection)", concatResult);


    if (bool showDiff = true) {
        cv::Mat concatDiffResult;

        cv::Mat sdiff;
        cv::absdiff(tsChannel, dsChannel, sdiff);

        cv::Mat vdiff;
        cv::absdiff(tvChannel, dvChannel, vdiff);

        cv::Mat combinedDiff;
        // cv::add(sdiff, vdiff, combinedDiff);

        // cv::imshow("combinedDiff", combinedDiff);

        cv::Mat grayDiff;
        if (vdiff.channels() == 3) {
            cv::cvtColor(vdiff, grayDiff, cv::COLOR_BGR2GRAY);
        } else {
            grayDiff = vdiff;
        }

        // cv::imshow("grayDiff", grayDiff);

        cv::Mat thresholdDiff;
        // 1. 阈值化处理，得到二值图像
        cv::threshold(grayDiff, thresholdDiff, m_whiteThreshold, 255, cv::THRESH_BINARY);

        // 2. 形态学开运算去除小噪点
        int kernalSize = 3;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernalSize, kernalSize)); // 创建3x3矩形结构元素
        cv::morphologyEx(thresholdDiff, thresholdDiff, cv::MORPH_OPEN,
                         kernel); // 执行开运算

        // 3. 统计过滤后的白色像素点
        int whitePixelCount = cv::countNonZero(thresholdDiff);
        qDebug() << "过滤后白色像素点的个数为: " << whitePixelCount;
        // cv::imshow("thresholdDiff", thresholdDiff);

        cv::vconcat(std::vector<cv::Mat>{vdiff, grayDiff, thresholdDiff}, concatDiffResult);
        cv::imshow("concatDiffResult", concatDiffResult);
    }
}
