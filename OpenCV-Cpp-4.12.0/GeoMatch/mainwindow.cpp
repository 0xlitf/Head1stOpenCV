#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QTimer>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QDateTime>
#include <QStyleFactory>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(new QWidget(this))
    , mainSplitter(new QSplitter(Qt::Horizontal, centralWidget))
    , templateScene(new QGraphicsScene(this))
    , searchScene(new QGraphicsScene(this))
{
    // 设置窗口属性
    setWindowTitle("基于边缘的几何模板匹配系统");
    setMinimumSize(1200, 700);

    // 应用样式
    qApp->setStyle(QStyleFactory::create("Fusion"));

    // 初始化UI
    setupUI();
    setupMenuBar();
    setupConnections();

    // 设置中心窗口
    setCentralWidget(centralWidget);

    // 初始化状态
    updateStatus("就绪 - 请选择模板图像和搜索图像");
}

MainWindow::~MainWindow()
{
    // Qt的智能指针会自动清理
}

void MainWindow::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // 创建主分割器
    mainSplitter->setHandleWidth(3);
    mainSplitter->setChildrenCollapsible(false);

    // 创建左侧模板面板
    setupTemplatePanel();

    // 创建右侧面板容器
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(5);

    // 创建搜索面板
    setupSearchPanel();

    // 创建控制面板
    setupControlPanel();

    // 创建日志区域
    QGroupBox *logGroup = new QGroupBox("处理日志");
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    logTextEdit = new QTextEdit();
    logTextEdit->setMaximumHeight(150);
    logTextEdit->setReadOnly(true);
    logLayout->addWidget(logTextEdit);

    // 组装右侧面板
    rightLayout->addWidget(searchGroup, 3);
    rightLayout->addWidget(controlGroup, 1);
    rightLayout->addWidget(logGroup, 1);

    // 添加分割器
    mainSplitter->addWidget(templateGroup);
    mainSplitter->addWidget(rightPanel);

    // 设置分割比例
    mainSplitter->setStretchFactor(0, 2);
    mainSplitter->setStretchFactor(1, 3);

    // 添加到主布局
    mainLayout->addWidget(mainSplitter);

    // 设置状态栏
    setupStatusBar();
}

void MainWindow::setupMenuBar()
{
    m_menuBar = this->menuBar();

    // 文件菜单
    fileMenu = m_menuBar->addMenu("文件(&F)");

    openTemplateAction = new QAction("打开模板图像(&T)...", this);
    openTemplateAction->setShortcut(QKeySequence("Ctrl+T"));
    openTemplateAction->setStatusTip("打开模板图像文件");

    openSearchAction = new QAction("打开搜索图像(&S)...", this);
    openSearchAction->setShortcut(QKeySequence("Ctrl+S"));
    openSearchAction->setStatusTip("打开搜索图像文件");

    saveResultAction = new QAction("保存匹配结果(&R)...", this);
    saveResultAction->setShortcut(QKeySequence("Ctrl+R"));
    saveResultAction->setStatusTip("保存匹配结果图像");
    saveResultAction->setEnabled(false);

    fileMenu->addAction(openTemplateAction);
    fileMenu->addAction(openSearchAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveResultAction);
    fileMenu->addSeparator();

    exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence("Ctrl+Q"));
    exitAction->setStatusTip("退出应用程序");
    fileMenu->addAction(exitAction);

    // 编辑菜单
    editMenu = m_menuBar->addMenu("编辑(&E)");

    resetAction = new QAction("重置参数(&R)", this);
    resetAction->setShortcut(QKeySequence("F5"));
    resetAction->setStatusTip("重置所有参数为默认值");
    editMenu->addAction(resetAction);

    // 帮助菜单
    helpMenu = m_menuBar->addMenu("帮助(&H)");

    aboutAction = new QAction("关于(&A)...", this);
    aboutAction->setStatusTip("关于本程序");
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupTemplatePanel()
{
    templateGroup = new QGroupBox("模板图像");
    templateGroup->setMinimumWidth(400);

    QVBoxLayout *templateLayout = new QVBoxLayout(templateGroup);

    // 顶部按钮区域
    QHBoxLayout *topLayout = new QHBoxLayout();
    templateLabel = new QLabel("模板图像:");
    selectTemplateBtn = new QPushButton("选择模板...");
    selectTemplateBtn->setFixedWidth(120);
    topLayout->addWidget(templateLabel);
    topLayout->addWidget(selectTemplateBtn);
    topLayout->addStretch();
    templateLayout->addLayout(topLayout);

    // 图像显示区域
    templateView = new QGraphicsView();
    templateView->setScene(templateScene);
    templateView->setRenderHint(QPainter::Antialiasing);
    templateView->setRenderHint(QPainter::SmoothPixmapTransform);
    templateView->setDragMode(QGraphicsView::ScrollHandDrag);
    templateView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    templateView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(templateView);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    templateLayout->addWidget(scrollArea, 4);

    // 参数控制区域
    QGroupBox *paramGroup = new QGroupBox("边缘检测参数");
    QGridLayout *paramLayout = new QGridLayout(paramGroup);

    // 边缘检测方法选择
    QLabel *methodLabel = new QLabel("边缘检测方法:");
    edgeMethodCombo = new QComboBox();
    edgeMethodCombo->addItems({"Canny检测", "Sobel算子", "Laplacian算子"});
    paramLayout->addWidget(methodLabel, 0, 0);
    paramLayout->addWidget(edgeMethodCombo, 0, 1);

    // 低阈值
    lowThreshLabel = new QLabel("低阈值:");
    lowThreshSlider = new QSlider(Qt::Horizontal);
    lowThreshSlider->setRange(0, 100);
    lowThreshSlider->setValue(lowThreshold);
    lowThreshSlider->setTickPosition(QSlider::TicksBelow);
    lowThreshSlider->setTickInterval(10);
    paramLayout->addWidget(lowThreshLabel, 1, 0);
    paramLayout->addWidget(lowThreshSlider, 1, 1);

    // 高阈值
    highThreshLabel = new QLabel("高阈值:");
    highThreshSlider = new QSlider(Qt::Horizontal);
    highThreshSlider->setRange(0, 255);
    highThreshSlider->setValue(highThreshold);
    highThreshSlider->setTickPosition(QSlider::TicksBelow);
    highThreshSlider->setTickInterval(25);
    paramLayout->addWidget(highThreshLabel, 2, 0);
    paramLayout->addWidget(highThreshSlider, 2, 1);

    // 当前阈值显示
    QLabel *currentThreshLabel = new QLabel("当前阈值:");
    QLabel *threshValueLabel = new QLabel(QString("%1 / %2").arg(lowThreshold).arg(highThreshold));
    threshValueLabel->setObjectName("threshValueLabel");
    threshValueLabel->setStyleSheet("font-weight: bold; color: blue;");
    paramLayout->addWidget(currentThreshLabel, 3, 0);
    paramLayout->addWidget(threshValueLabel, 3, 1);

    // 连接阈值更新
    connect(lowThreshSlider, &QSlider::valueChanged, this, [=](int value){
        lowThreshold = value;
        threshValueLabel->setText(QString("%1 / %2").arg(lowThreshold).arg(highThreshold));
        updateTemplatePreview();
    });

    connect(highThreshSlider, &QSlider::valueChanged, this, [=](int value){
        highThreshold = value;
        threshValueLabel->setText(QString("%1 / %2").arg(lowThreshold).arg(highThreshold));
        updateTemplatePreview();
    });

    // 模板信息
    templateInfoLabel = new QLabel("未加载模板图像");
    templateInfoLabel->setAlignment(Qt::AlignCenter);
    templateInfoLabel->setStyleSheet("font-style: italic; color: gray;");

    templateLayout->addWidget(paramGroup, 1);
    templateLayout->addWidget(templateInfoLabel);
}

void MainWindow::setupSearchPanel()
{
    searchGroup = new QGroupBox("搜索图像与结果");

    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);

    // 顶部按钮区域
    QHBoxLayout *topLayout = new QHBoxLayout();
    searchLabel = new QLabel("搜索图像:");
    selectSearchBtn = new QPushButton("选择搜索图...");
    selectSearchBtn->setFixedWidth(120);
    topLayout->addWidget(searchLabel);
    topLayout->addWidget(selectSearchBtn);
    topLayout->addStretch();
    searchLayout->addLayout(topLayout);

    // 图像显示区域
    searchView = new QGraphicsView();
    searchView->setScene(searchScene);
    searchView->setRenderHint(QPainter::Antialiasing);
    searchView->setRenderHint(QPainter::SmoothPixmapTransform);
    searchView->setDragMode(QGraphicsView::ScrollHandDrag);
    searchView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    searchView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(searchView);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    searchLayout->addWidget(scrollArea, 4);

    // 结果信息
    resultLabel = new QLabel("匹配结果: 未开始");
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setStyleSheet("font-weight: bold; padding: 5px; border: 1px solid #ccc;");

    // 搜索图像信息
    searchInfoLabel = new QLabel("未加载搜索图像");
    searchInfoLabel->setAlignment(Qt::AlignCenter);
    searchInfoLabel->setStyleSheet("font-style: italic; color: gray;");

    searchLayout->addWidget(resultLabel);
    searchLayout->addWidget(searchInfoLabel);
}

void MainWindow::setupControlPanel()
{
    controlGroup = new QGroupBox("匹配控制");

    QGridLayout *controlLayout = new QGridLayout(controlGroup);

    // 最小分数
    minScoreLabel = new QLabel("最小匹配分数:");
    minScoreSpin = new QDoubleSpinBox();
    minScoreSpin->setRange(0.0, 1.0);
    minScoreSpin->setSingleStep(0.05);
    minScoreSpin->setValue(minScore);
    minScoreSpin->setDecimals(3);
    controlLayout->addWidget(minScoreLabel, 0, 0);
    controlLayout->addWidget(minScoreSpin, 0, 1);

    // 贪婪度
    greedinessLabel = new QLabel("贪婪度:");
    greedinessSpin = new QDoubleSpinBox();
    greedinessSpin->setRange(0.0, 1.0);
    greedinessSpin->setSingleStep(0.1);
    greedinessSpin->setValue(greediness);
    greedinessSpin->setDecimals(2);
    controlLayout->addWidget(greedinessLabel, 1, 0);
    controlLayout->addWidget(greedinessSpin, 1, 1);

    // 匹配按钮
    matchBtn = new QPushButton("开始匹配");
    matchBtn->setFixedHeight(40);
    matchBtn->setStyleSheet("font-weight: bold; font-size: 14px;");
    controlLayout->addWidget(matchBtn, 2, 0, 1, 2);

    // 重置按钮
    resetBtn = new QPushButton("重置参数");
    resetBtn->setFixedHeight(30);
    controlLayout->addWidget(resetBtn, 3, 0, 1, 2);

    // 匹配结果显示
    matchResultLabel = new QLabel("准备就绪");
    matchResultLabel->setAlignment(Qt::AlignCenter);
    matchResultLabel->setWordWrap(true);
    matchResultLabel->setStyleSheet("padding: 10px; border: 1px solid #4CAF50; border-radius: 5px; background-color: #f8fff8;");
    controlLayout->addWidget(matchResultLabel, 4, 0, 1, 2);

    // 添加弹簧
    controlLayout->setRowStretch(5, 1);
}

void MainWindow::setupStatusBar()
{
    m_statusBar = this->statusBar();
    statusLabel = new QLabel("就绪");
    m_statusBar->addWidget(statusLabel);

    // 添加永久部件
    QLabel *versionLabel = new QLabel("几何模板匹配 v1.0");
    versionLabel->setStyleSheet("color: gray;");
    m_statusBar->addPermanentWidget(versionLabel);
}

void MainWindow::setupConnections()
{
    // 菜单栏连接
    connect(openTemplateAction, &QAction::triggered, this, &MainWindow::openTemplateImage);
    connect(openSearchAction, &QAction::triggered, this, &MainWindow::openSearchImage);
    connect(saveResultAction, &QAction::triggered, this, &MainWindow::saveResultImage);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::aboutApplication);
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetParameters);

    // 按钮连接
    connect(selectTemplateBtn, &QPushButton::clicked, this, &MainWindow::openTemplateImage);
    connect(selectSearchBtn, &QPushButton::clicked, this, &MainWindow::openSearchImage);
    connect(matchBtn, &QPushButton::clicked, this, &MainWindow::startMatching);
    connect(resetBtn, &QPushButton::clicked, this, &MainWindow::resetParameters);

    // 参数变化连接
    connect(minScoreSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::minScoreChanged);
    connect(greedinessSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::greedinessChanged);
    connect(edgeMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::edgeMethodChanged);
}

void MainWindow::openTemplateImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "选择模板图像",
                                                    "",
                                                    "图像文件 (*.jpg *.jpeg *.png *.bmp *.tiff *.tif);;所有文件 (*.*)");

    if (fileName.isEmpty()) return;

    // 读取图像
    cv::Mat image = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        showError("无法加载模板图像: " + fileName);
        return;
    }

    templateImage = image.clone();
    templateImagePath = fileName;

    // 更新显示
    updateTemplateDisplay();
    updateTemplatePreview();

    // 记录日志
    QString log = QString("[%1] 加载模板图像: %2 (%3x%4)")
                      .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                      .arg(QFileInfo(fileName).fileName())
                      .arg(image.cols)
                      .arg(image.rows);
    logTextEdit->append(log);

    updateStatus("模板图像已加载: " + QFileInfo(fileName).fileName());
}

void MainWindow::openSearchImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "选择搜索图像",
                                                    "",
                                                    "图像文件 (*.jpg *.jpeg *.png *.bmp *.tiff *.tif);;所有文件 (*.*)");

    if (fileName.isEmpty()) return;

    // 读取图像
    cv::Mat image = cv::imread(fileName.toStdString(), cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        showError("无法加载搜索图像: " + fileName);
        return;
    }

    searchImage = image.clone();
    searchImagePath = fileName;

    // 更新显示
    updateSearchDisplay();

    // 记录日志
    QString log = QString("[%1] 加载搜索图像: %2 (%3x%4)")
                      .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                      .arg(QFileInfo(fileName).fileName())
                      .arg(image.cols)
                      .arg(image.rows);
    logTextEdit->append(log);

    updateStatus("搜索图像已加载: " + QFileInfo(fileName).fileName());
}

void MainWindow::startMatching()
{
    if (templateImage.empty()) {
        showError("请先选择模板图像");
        return;
    }

    if (searchImage.empty()) {
        showError("请先选择搜索图像");
        return;
    }

    // 禁用界面防止重复点击
    matchBtn->setEnabled(false);
    matchBtn->setText("匹配中...");
    QApplication::processEvents();

    // 执行匹配
    performMatching();

    // 重新启用按钮
    matchBtn->setEnabled(true);
    matchBtn->setText("开始匹配");

    // 启用保存结果功能
    saveResultAction->setEnabled(isMatched);
}

void MainWindow::performMatching()
{
    try {
        // 记录开始时间
        QElapsedTimer timer;
        timer.start();

        // 创建模板模型
        if (!geoMatcher.CreateGeoMatchModel(templateImage, lowThreshold, highThreshold)) {
            showError("创建模板模型失败");
            return;
        }

        // 执行匹配
        matchScore = geoMatcher.FindGeoMatchModel(searchImage, minScore, greediness, &matchPoint);
        matchTime = timer.elapsed(); // 毫秒

        isMatched = (matchScore >= minScore);

        // 更新结果
        if (isMatched) {
            // 创建结果图像
            cv::cvtColor(searchImage, resultImage, cv::COLOR_GRAY2BGR);
            geoMatcher.DrawContours(resultImage, matchPoint, cv::Scalar(0, 255, 0), 2);

            // 添加标注
            cv::rectangle(resultImage,
                          cv::Rect(matchPoint.x, matchPoint.y, templateImage.cols, templateImage.rows),
                          cv::Scalar(0, 255, 0), 2);

            cv::putText(resultImage,
                        cv::format("匹配分数: %.3f", matchScore),
                        cv::Point(matchPoint.x, matchPoint.y - 10),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

            cv::putText(resultImage,
                        cv::format("位置: (%d, %d)", matchPoint.x, matchPoint.y),
                        cv::Point(matchPoint.x, matchPoint.y + templateImage.rows + 20),
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 0), 1);

            // 更新显示
            displayImage(resultImage, searchView, "匹配结果");

            // 更新结果标签
            QString resultText = QString("✓ 匹配成功!\n"
                                         "位置: (%1, %2)\n"
                                         "分数: %3\n"
                                         "用时: %4ms")
                                     .arg(matchPoint.x)
                                     .arg(matchPoint.y)
                                     .arg(matchScore, 0, 'f', 3)
                                     .arg(matchTime);
            matchResultLabel->setText(resultText);
            matchResultLabel->setStyleSheet("padding: 10px; border: 2px solid #4CAF50; border-radius: 5px; background-color: #f8fff8; color: #2E7D32;");

            resultLabel->setText(QString("匹配成功 - 分数: %1").arg(matchScore, 0, 'f', 3));
            resultLabel->setStyleSheet("font-weight: bold; padding: 5px; border: 2px solid #4CAF50; background-color: #e8f5e8;");

            // 记录日志
            QString log = QString("[%1] 匹配成功! 位置: (%2, %3), 分数: %4, 用时: %5ms")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(matchPoint.x)
                              .arg(matchPoint.y)
                              .arg(matchScore, 0, 'f', 3)
                              .arg(matchTime);
            logTextEdit->append(log);

            updateStatus(QString("匹配成功 - 分数: %1, 用时: %2ms").arg(matchScore, 0, 'f', 3).arg(matchTime));

        } else {
            // 未匹配成功
            displayImage(searchImage, searchView, "搜索图像");

            QString resultText = QString("✗ 未找到匹配\n"
                                         "最佳分数: %1\n"
                                         "阈值: %2")
                                     .arg(matchScore, 0, 'f', 3)
                                     .arg(minScore, 0, 'f', 2);
            matchResultLabel->setText(resultText);
            matchResultLabel->setStyleSheet("padding: 10px; border: 2px solid #f44336; border-radius: 5px; background-color: #fff8f8; color: #c62828;");

            resultLabel->setText("未找到匹配");
            resultLabel->setStyleSheet("font-weight: bold; padding: 5px; border: 2px solid #f44336; background-color: #ffebee;");

            // 记录日志
            QString log = QString("[%1] 匹配失败! 最佳分数: %2 (阈值: %3)")
                              .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                              .arg(matchScore, 0, 'f', 3)
                              .arg(minScore, 0, 'f', 2);
            logTextEdit->append(log);

            updateStatus(QString("匹配失败 - 最佳分数: %1").arg(matchScore, 0, 'f', 3));
        }

    } catch (const std::exception& e) {
        showError(QString("匹配过程中发生错误: %1").arg(e.what()));
        matchBtn->setEnabled(true);
        matchBtn->setText("开始匹配");
    }
}

void MainWindow::updateTemplateDisplay()
{
    if (!templateImage.empty()) {
        // 显示原始模板图像
        displayImage(templateImage, templateView, "模板图像");

        // 更新信息
        QString info = QString("尺寸: %1×%2\n通道: 灰度")
                           .arg(templateImage.cols)
                           .arg(templateImage.rows);
        templateInfoLabel->setText(info);
    }
}

void MainWindow::updateTemplatePreview()
{
    if (!templateImage.empty()) {
        // 根据选择的边缘检测方法处理图像
        cv::Mat edges;
        switch (edgeMethod) {
        case 0: // Canny
            cv::Canny(templateImage, edges, lowThreshold, highThreshold);
            break;
        case 1: // Sobel
        {
            cv::Mat grad_x, grad_y;
            cv::Sobel(templateImage, grad_x, CV_16S, 1, 0, 3);
            cv::Sobel(templateImage, grad_y, CV_16S, 0, 1, 3);
            cv::convertScaleAbs(grad_x, grad_x);
            cv::convertScaleAbs(grad_y, grad_y);
            cv::addWeighted(grad_x, 0.5, grad_y, 0.5, 0, edges);
        }
        break;
        case 2: // Laplacian
            cv::Laplacian(templateImage, edges, CV_16S, 3);
            cv::convertScaleAbs(edges, edges);
            break;
        }

        // 转换为彩色显示
        cv::Mat colorEdges;
        cv::cvtColor(edges, colorEdges, cv::COLOR_GRAY2BGR);

        // 显示边缘检测结果
        displayImage(colorEdges, templateView, "边缘检测预览");

        // 记录日志
        QString methodName = edgeMethodCombo->currentText();
        QString log = QString("[%1] 模板边缘检测更新 - 方法: %2, 阈值: %3/%4")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                          .arg(methodName)
                          .arg(lowThreshold)
                          .arg(highThreshold);
        logTextEdit->append(log);
    }
}

void MainWindow::updateSearchDisplay()
{
    if (!searchImage.empty()) {
        displayImage(searchImage, searchView, "搜索图像");

        QString info = QString("尺寸: %1×%2")
                           .arg(searchImage.cols)
                           .arg(searchImage.rows);
        searchInfoLabel->setText(info);
    }
}

void MainWindow::displayImage(const cv::Mat &mat, QGraphicsView *view, const QString &title)
{
    if (mat.empty()) return;

    QImage image = matToQImage(mat);
    if (image.isNull()) return;

    QGraphicsScene *scene = view->scene();
    if (!scene) {
        scene = new QGraphicsScene(view);
        view->setScene(scene);
    }

    scene->clear();

    // 添加图像
    QPixmap pixmap = QPixmap::fromImage(image);
    QGraphicsPixmapItem *item = scene->addPixmap(pixmap);
    item->setTransformationMode(Qt::SmoothTransformation);

    // 添加标题
    if (!title.isEmpty()) {
        QGraphicsTextItem *textItem = scene->addText(title);
        textItem->setDefaultTextColor(Qt::red);
        textItem->setPos(10, 10);
    }

    scene->setSceneRect(QRectF(pixmap.rect()));
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

QImage MainWindow::matToQImage(const cv::Mat &mat)
{
    if (mat.empty()) {
        return QImage();
    }

    switch (mat.type()) {
    case CV_8UC1:  // 8-bit, 1 channel
    {
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image.copy();
    }
    case CV_8UC3:  // 8-bit, 3 channels
    {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        QImage image(rgb.data, rgb.cols, rgb.rows,
                     static_cast<int>(rgb.step), QImage::Format_RGB888);
        return image.copy();
    }
    case CV_8UC4:  // 8-bit, 4 channels
    {
        QImage image(mat.data, mat.cols, mat.rows,
                     static_cast<int>(mat.step), QImage::Format_ARGB32);
        return image.copy();
    }
    default:
        cv::Mat converted;
        mat.convertTo(converted, CV_8UC(mat.channels() == 1 ? 1 : 3));
        return matToQImage(converted);
    }
}

void MainWindow::saveResultImage()
{
    if (!isMatched || resultImage.empty()) {
        showError("没有可保存的匹配结果");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存匹配结果",
                                                    "匹配结果.jpg",
                                                    "JPEG图像 (*.jpg *.jpeg);;PNG图像 (*.png);;BMP图像 (*.bmp)");

    if (fileName.isEmpty()) return;

    if (cv::imwrite(fileName.toStdString(), resultImage)) {
        showInfo("结果已保存到: " + fileName);

        // 记录日志
        QString log = QString("[%1] 保存匹配结果: %2")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                          .arg(QFileInfo(fileName).fileName());
        logTextEdit->append(log);

        updateStatus("结果已保存: " + QFileInfo(fileName).fileName());
    } else {
        showError("保存图像失败");
    }
}

void MainWindow::resetParameters()
{
    // 重置参数
    lowThreshold = 10;
    highThreshold = 100;
    minScore = 0.7;
    greediness = 0.8;
    edgeMethod = 0;

    // 更新UI
    lowThreshSlider->setValue(lowThreshold);
    highThreshSlider->setValue(highThreshold);
    minScoreSpin->setValue(minScore);
    greedinessSpin->setValue(greediness);
    edgeMethodCombo->setCurrentIndex(edgeMethod);

    // 更新模板预览
    if (!templateImage.empty()) {
        updateTemplatePreview();
    }

    // 重置结果
    matchResultLabel->setText("参数已重置为默认值");
    matchResultLabel->setStyleSheet("padding: 10px; border: 1px solid #ccc; border-radius: 5px; background-color: #f5f5f5;");

    resultLabel->setText("匹配结果: 未开始");
    resultLabel->setStyleSheet("font-weight: bold; padding: 5px; border: 1px solid #ccc;");

    // 记录日志
    QString log = QString("[%1] 参数已重置为默认值")
                      .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    logTextEdit->append(log);

    updateStatus("参数已重置");
}

void MainWindow::exitApplication()
{
    close();
}

void MainWindow::aboutApplication()
{
    QMessageBox::about(this, "关于",
                       "<h3>基于边缘的几何模板匹配系统 v1.0</h3>"
                       "<p>本程序实现了基于边缘特征的几何模板匹配算法，"
                       "用于在图像中定位特定形状的模板。</p>"
                       "<p><b>功能特点：</b></p>"
                       "<ul>"
                       "<li>多种边缘检测方法（Canny、Sobel、Laplacian）</li>"
                       "<li>实时参数调整和预览</li>"
                       "<li>详细的匹配结果和日志记录</li>"
                       "<li>结果图像保存功能</li>"
                       "</ul>"
                       "<p><b>参数说明：</b></p>"
                       "<ul>"
                       "<li><b>低/高阈值：</b>边缘检测的敏感度控制</li>"
                       "<li><b>最小分数：</b>匹配成功的最低阈值（0.0-1.0）</li>"
                       "<li><b>贪婪度：</b>搜索算法的启发式参数（0.0-1.0）</li>"
                       "</ul>"
                       "<p>基于 OpenCV 和 Qt 6 开发</p>");
}

void MainWindow::lowThresholdChanged(int value)
{
    lowThreshold = value;
    updateTemplatePreview();
}

void MainWindow::highThresholdChanged(int value)
{
    highThreshold = value;
    updateTemplatePreview();
}

void MainWindow::minScoreChanged(double value)
{
    minScore = value;
}

void MainWindow::greedinessChanged(double value)
{
    greediness = value;
}

void MainWindow::edgeMethodChanged(int index)
{
    edgeMethod = index;
    updateTemplatePreview();
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, "错误", message);
    logTextEdit->append(QString("[ERROR] %1").arg(message));
    updateStatus("错误: " + message);
}

void MainWindow::showInfo(const QString &message)
{
    QMessageBox::information(this, "信息", message);
    updateStatus(message);
}

void MainWindow::updateStatus(const QString &message)
{
    statusLabel->setText(message);
}

void MainWindow::updateSearchPreview()
{
    if (!searchImage.empty()) {
        // 如果有匹配结果，显示结果预览
        if (isMatched && !resultImage.empty()) {
            displayImage(resultImage, searchView, "匹配结果");
        } else {
            // 否则显示原始搜索图像
            displayImage(searchImage, searchView, "搜索图像");

            // 如果有模板图像，可以在搜索图像上显示一个矩形来表示预期的搜索区域
            if (!templateImage.empty()) {
                // 创建一个带矩形标注的预览图像
                cv::Mat preview = searchImage.clone();
                cv::cvtColor(preview, preview, cv::COLOR_GRAY2BGR);

                // 在图像中心绘制一个半透明的矩形来表示模板尺寸
                int centerX = preview.cols / 2;
                int centerY = preview.rows / 2;
                int templateHalfWidth = templateImage.cols / 2;
                int templateHalfHeight = templateImage.rows / 2;

                // 计算矩形位置，确保不超出图像边界
                int rectX = std::max(0, centerX - templateHalfWidth);
                int rectY = std::max(0, centerY - templateHalfHeight);
                int rectWidth = std::min(templateImage.cols, preview.cols - rectX);
                int rectHeight = std::min(templateImage.rows, preview.rows - rectY);

                // 绘制半透明矩形
                cv::Mat overlay = preview.clone();
                cv::rectangle(overlay,
                              cv::Rect(rectX, rectY, rectWidth, rectHeight),
                              cv::Scalar(255, 0, 0), 2);

                // 添加透明度
                cv::addWeighted(preview, 0.7, overlay, 0.3, 0, preview);

                // 添加文本标注
                cv::putText(preview,
                            cv::format("模板: %dx%d", templateImage.cols, templateImage.rows),
                            cv::Point(rectX, rectY - 10),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);

                displayImage(preview, searchView, "搜索预览（显示模板尺寸）");
            }
        }

        // 记录日志
        QString log = QString("[%1] 更新搜索图像预览")
                          .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
        if (isMatched) {
            log += QString(" - 匹配结果分数: %1").arg(matchScore, 0, 'f', 3);
        }
        logTextEdit->append(log);
    }
}
