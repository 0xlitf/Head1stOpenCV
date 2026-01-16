#include "MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include "minimumbounding.h"
#include <opencv2/imgcodecs.hpp>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 初始化UI
    setWindowTitle("HSV缺陷检测系统");
    setMinimumSize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    m_normalLabel = new QLabel("正常图像");
    m_defectLabel = new QLabel("缺陷图像");
    m_resultLabel = new QLabel("检测结果将显示在这里");

    m_detectButton = new QPushButton("开始缺陷检测");

    layout->addWidget(m_normalLabel);
    layout->addWidget(m_defectLabel);
    layout->addWidget(m_detectButton);
    layout->addWidget(m_resultLabel);

    // 初始化检测器
    m_detector = new DefectDetector(this);

    // 连接信号槽
    connect(m_detectButton, &QPushButton::clicked, this, &MainWindow::onDetectDefect);

    MinimumBounding processor;

    cv::Mat templateInput = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ok/2026-01-15_16-07-45_741.png");
    cv::Mat defectInput = cv::imread("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-4.12.0/DefectDetection/2/ng/2026-01-15_16-09-20_925.png");

    // 加载示例图像
    m_normalImage = templateInput;
    m_defectImage = defectInput;
}

MainWindow::~MainWindow()
{
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

    // 保存缺陷热力图
    if (!result.defectMap.empty()) {
        cv::imwrite("defect_map.jpg", result.defectMap);
    }
}
