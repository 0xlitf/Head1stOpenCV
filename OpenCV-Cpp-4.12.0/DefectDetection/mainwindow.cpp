#include "MainWindow.h"
#include "minimumbounding.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 设置窗口属性
    setWindowTitle("HSV缺陷检测系统");
    setMinimumSize(1200, 800);

    // 创建中心部件和布局
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_imagesLayout = new QHBoxLayout();

    // 创建图像显示标签
    m_normalImageLabel = new QLabel("正常图像（未加载）");
    m_defectImageLabel = new QLabel("缺陷图像（未加载）");
    m_resultLabel = new QLabel("检测结果将显示在这里");

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
    connect(m_loadNormalButton, &QPushButton::clicked, this, &MainWindow::onLoadNormalImage);
    connect(m_loadDefectButton, &QPushButton::clicked, this, &MainWindow::onLoadDefectImage);
    connect(m_detectButton, &QPushButton::clicked, this, &MainWindow::onDetectDefect);

    // 加载默认图像（可选）

    cv::Mat tInput = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ok/2026-01-15_16-07-45_741.png");
    cv::Mat dInput = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ng/2026-01-15_16-09-20_925.png");

    MinimumBounding mini;
    m_normalImage = mini.findAndCropObject(tInput);
    m_defectImage = mini.findAndCropObject(dInput);

    if (!m_normalImage.empty()) {
        displayImageOnLabel(m_normalImageLabel, m_normalImage);
    }
    if (!m_defectImage.empty()) {
        displayImageOnLabel(m_defectImageLabel, m_defectImage);
        m_detectButton->setEnabled(true);
    }

}

MainWindow::~MainWindow()
{
}

void MainWindow::loadDefaultImages() {}

void MainWindow::onLoadNormalImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择正常图像", "",
                                                    "图像文件 (*.png *.jpg *.bmp *.jpeg)");
    if (!fileName.isEmpty()) {
        cv::Mat tInput = cv::imread(fileName.toStdString());

        MinimumBounding mini;
        m_normalImage = mini.findAndCropObject(tInput);

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

void MainWindow::onLoadDefectImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择缺陷图像", "",
                                                    "图像文件 (*.png *.jpg *.bmp *.jpeg)");
    if (!fileName.isEmpty()) {
        cv::Mat dInput = cv::imread(fileName.toStdString());

        MinimumBounding mini;
        m_defectImage = mini.findAndCropObject(dInput);

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

void MainWindow::displayImageOnLabel(QLabel *label, const cv::Mat &image)
{
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
    QPixmap scaledPixmap = originalPixmap.scaled(label->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation); // 使用平滑变换以获得更好质量

    // 4. 显示图像
    label->setPixmap(scaledPixmap);
    // 非常重要：禁用自动拉伸，由我们自己控制缩放[7,8](@ref)
    label->setScaledContents(false);
}

cv::Mat MainWindow::resizeImageForDisplay(const cv::Mat &image, QLabel *label)
{
    if (image.empty()) return cv::Mat();

    // 获取标签的可用大小
    QSize labelSize = label->size();
    int maxWidth = labelSize.width() - 20;  // 留出边距
    int maxHeight = labelSize.height() - 20;

    // 如果图像尺寸小于标签尺寸，直接返回原图
    if (image.cols <= maxWidth && image.rows <= maxHeight) {
        return image.clone();
    }

    // 计算缩放比例，保持宽高比 [3](@ref)
    double scale = std::min(static_cast<double>(maxWidth) / image.cols,
                            static_cast<double>(maxHeight) / image.rows);

    int newWidth = static_cast<int>(image.cols * scale);
    int newHeight = static_cast<int>(image.rows * scale);

    cv::Mat resizedImage;
    cv::resize(image, resizedImage, cv::Size(newWidth, newHeight));
    return resizedImage;
}

void MainWindow::onDetectDefect()
{
    if (m_normalImage.empty() || m_defectImage.empty()) {
        QMessageBox::warning(this, "错误", "请先加载正常图像和缺陷图像!");
        return;
    }

    // 执行缺陷检测
    DefectDetector::DefectResult result = m_detector->detectDefect(m_normalImage, m_defectImage);

    // 显示结果
    QString resultText = QString("缺陷检测结果:\n"
                                 "综合损失值: %1\n"
                                 "相关性损失: %2\n"
                                 "卡方损失: %3\n"
                                 "巴氏距离损失: %4\n"
                                 "检测结果: %5")
                             .arg(result.totalLoss, 0, 'f', 3)
                             .arg(result.correlationLoss, 0, 'f', 3)
                             .arg(result.chiSquareLoss, 0, 'f', 3)
                             .arg(result.bhattacharyyaLoss, 0, 'f', 3)
                             .arg(result.hasDefect ? "有缺陷" : "正常");

    m_resultLabel->setText(resultText);

    // 保存并显示缺陷热力图
    if (!result.defectMap.empty()) {
        cv::imwrite("defect_map.jpg", result.defectMap);

        // 可选：在另一个标签中显示缺陷热力图
        // displayImageOnLabel(new QLabel("缺陷热力图"), result.defectMap);
    }
}
