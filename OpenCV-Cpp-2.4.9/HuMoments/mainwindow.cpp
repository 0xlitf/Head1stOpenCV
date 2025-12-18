#include "mainwindow.h"
#include "layoutbuilder.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setMinimumSize(1200, 850);
    this->createComponents();

    connect(&m_matcher, &HuMomentsMatcher::sendLog, this,
            [=](const QString &logStr) { m_logTextEdit->append(logStr); });

    connect(&m_matcher, &HuMomentsMatcher::errorOccured, this,
            [=](HuMomentsMatcher::ErrorCode errorCode, const QString &errorStr) {
        m_logTextEdit->append(
            QString("HuMomentsMatcher算法错误, 错误码:%1, 错误描述:%2")
                .arg(errorCode)
                .arg(errorStr));
    });

    m_matcher.setTemplateFolder(
        "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/HuMoments/dataset_foler");
}

void MainWindow::createComponents() {
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // 顶部按钮区
    m_loadTemplateButton = new QPushButton("1. 加载模板图像", this);
    m_loadSceneButton = new QPushButton("2. 加载检测图像", this);
    m_matchButton = new QPushButton("3. 开始识别与定位", this);

    m_templateFolderLabel = new QLabel(
        "C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/HuMoments/dataset_foler", this);
    m_templateDescLabel = new QLabel("模板文件夹路径: ", this);
    m_templateFolderLabel->setSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Fixed);

    m_templateFolderLabel->setFixedHeight(30);
    m_templateDescLabel->setFixedHeight(30);

    auto btnLayout = Layouting::Column{
        Layouting::Row{
            m_loadTemplateButton,
            m_loadSceneButton,
            m_matchButton,
        },
        Layouting::Row{
            m_templateDescLabel,
            m_templateFolderLabel,
        },
    };

    // 中间图像显示区
    QHBoxLayout *imgLayout = new QHBoxLayout();
    m_templateLabel = new QLabel("模板预览", this);
    m_templateLabel->setAlignment(Qt::AlignCenter);
    m_templateLabel->setStyleSheet("border: 1px solid gray; background: #eee;");
    m_templateLabel->setMinimumSize(300, 300);

    m_contourLabel = new QLabel("轮廓预览", this);
    m_contourLabel->setAlignment(Qt::AlignCenter);
    m_contourLabel->setStyleSheet("border: 1px solid gray; background: #eee;");
    m_contourLabel->setMinimumSize(300, 300);

    m_sceneLabel = new QLabel("检测场景预览", this);
    m_sceneLabel->setAlignment(Qt::AlignCenter);
    m_sceneLabel->setStyleSheet("border: 1px solid gray; background: #eee;");
    m_sceneLabel->setMinimumSize(500, 300);

    imgLayout->addWidget(
        Layouting::Column{m_templateLabel, m_contourLabel}.emerge(), 1);
    imgLayout->addWidget(m_sceneLabel, 2);

    // 底部日志区
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setReadOnly(true);

    Layouting::Column{btnLayout, imgLayout, m_logTextEdit}.attachTo(
        centralWidget);

    resize(1000, 700);
    setWindowTitle("OpenCV形状匹配与定位");

    connect(m_loadTemplateButton, &QPushButton::clicked, this, [=]() {
        QElapsedTimer timer;
        timer.start();
        // this->onLoadTemplate();

        this->onLoadTemplateFolder();
        qDebug() << "onLoadTemplate elapsed:" << timer.elapsed();
    });
    connect(m_loadSceneButton, &QPushButton::clicked, this, [=]() {
        QElapsedTimer timer;
        timer.start();
        this->onLoadScene();
        qDebug() << "onLoadScene elapsed:" << timer.elapsed();
    });
    connect(m_matchButton, &QPushButton::clicked, this, [=]() {
        QElapsedTimer timer;
        timer.start();
        this->onRunMatching();
        qDebug() << "onRunMatching elapsed:" << timer.elapsed();
    });
}

MainWindow::~MainWindow() {}

void MainWindow::onLoadTemplateFolder() {
    auto folderName = FileUtils::selectFolderDialog(this);
    if (folderName.isEmpty()) {
        return;
    }
    m_matcher.setTemplateFolder(folderName);
}

void MainWindow::onLoadTemplate() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择模板图片", "",
                                                    "Images (*.png *.jpg *.bmp)");
    if (fileName.isEmpty())
        return;

    cv::Mat templateImg = m_matcher.addTemplate(fileName);

    if (templateImg.empty())
        return;

    // 显示原图
    m_templateLabel->setPixmap(
        m_matcher.cvMatToQPixmap(templateImg)
            .scaled(m_templateLabel->size(), Qt::KeepAspectRatio));

    cv::threshold(templateImg, templateImg, 240, 255, cv::THRESH_BINARY);

    // 预处理并提取特征
    m_templateContour = m_matcher.findLargestContour(templateImg, true);
    if (!m_templateContour.empty()) {
        // 1. 创建黑色画布
        // cv::Mat canvas = cv::Mat::zeros(templateImg.size(), CV_8UC3);
        cv::Mat canvas;
        if (templateImg.channels() == 1) {
            cv::cvtColor(templateImg, canvas, cv::COLOR_GRAY2BGR);
        } else {
            canvas = templateImg.clone(); // 深拷贝
        }

        // 2. 绘制实心或空心轮廓
        std::vector<std::vector<cv::Point>> contoursToDraw = {m_templateContour};
        cv::drawContours(canvas, contoursToDraw, 0, cv::Scalar(0, 255, 0),
                         2); // 绿色线条

        auto cropped = m_matcher.croppedCanvas(templateImg, m_templateContour);

        // 6. 显示裁剪后的图像
        // m_contourLabel->setPixmap(m_matcher.cvMatToQPixmap(croppedCanvas).scaled(m_contourLabel->size(),
        // Qt::KeepAspectRatio));
        m_contourLabel->setPixmap(m_matcher.cvMatToQPixmap(cropped).scaled(
            m_contourLabel->size(), Qt::KeepAspectRatio));
    }

    if (!m_templateContour.empty()) {
        m_logTextEdit->append("<b>[模板加载成功]</b>");
        m_logTextEdit->append("Hu 矩特征 (Log Scale):");
        // 计算 Hu 矩
        QString huStr = m_matcher.calcHuMoments(m_templateContour);

        m_logTextEdit->append(huStr);
    }
}

void MainWindow::onLoadScene() {
    m_sceneFileName = QFileDialog::getOpenFileName(this, "选择场景图片", "",
                                                   "Images (*.png *.jpg *.bmp)");
    if (m_sceneFileName.isEmpty())
        return;

    // 这里读取彩色图，方便最后画绿色的框
    m_sceneImg = cv::imread(m_sceneFileName.toStdString(), cv::IMREAD_COLOR);
    if (m_sceneImg.empty())
        return;

    m_sceneLabel->setPixmap(
        m_matcher.cvMatToQPixmap(m_sceneImg)
            .scaled(m_sceneLabel->size(), Qt::KeepAspectRatio));
    m_logTextEdit->append("<b>[场景加载成功]</b> 等待识别...");
}

void MainWindow::onRunMatching() {
    // if (m_sceneImg.empty() || m_templateContour.empty()) {
    //     QMessageBox::warning(this, "提示", "请先加载模板和场景图片！");
    //     return;
    // }

    m_logTextEdit->append("--- 开始匹配 ---");

    m_matcher.matchImage(m_sceneFileName);

    // 更新界面显示
    // m_sceneLabel->setPixmap(m_matcher.cvMatToQPixmap(resultImg).scaled(
    //     m_sceneLabel->size(), Qt::KeepAspectRatio));
}
