#include "defectdetectpage.h"
#include "controls/clickablewidget.h"
#include "imageutils.h"
#include "utils/fileutils.h"
#include "widgets/imagegridwidget.h"
#include "widgets/templategridwidget.h"
#include "widgets/imagelistviewwidget.h"
#include "widgets/imagelistwidget.h"
#include "widgets/selectfilewidget.h"
#include "widgets/selectfolderwidget.h"
#include "minimumbounding.h"
#include "opencv2/opencv.hpp"
#include "bgr2hsvconverter.h"
#include "defectdetector.h"
#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>

DefectDetectPage::DefectDetectPage() {
    this->createComponents();
    this->createConnections();
    this->loadConfig();

    // 细小角落缺陷 2026-01-15_16-09-37_766.png
    // 明显缺角 2026-01-15_16-09-20_925.png
    // runDefectDetectAlgo("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection/2/NG/2026-01-15_16-09-37_766.png");
}

DefectDetectPage::~DefectDetectPage() { this->saveConfig(); }

void DefectDetectPage::runDefectDetectAlgo(const QString &filePath) {
    DefectDetector detector;

    QStringList descList;
    descList << "ok";
    QStringList folderList;
    folderList << QString("C:/GitHub/Head1stOpenCV/OpenCV-Cpp-2.4.9/DefectDetection").append("/template_black"); // template_brown template_black
    detector.setTemplateFolder(descList, folderList);

    auto dInputMat = cv::imread(filePath.toStdString());

    std::tuple<int, cv::Mat> analyzeResult = ContourExtractor::analyzeAndDrawContour(dInputMat);
    int count = std::get<0>(analyzeResult);
    if (count != 1) {
        qDebug() << "analyzeResult 物体数量 != 1" << count;
        cv::imshow("analyzeResult", std::get<1>(analyzeResult));
    }

    detector.setInputMat(dInputMat);

    QElapsedTimer timer;
    timer.start();
    std::tuple<bool, double> areaDiff = detector.p0_matchArea(); // 整体轮廓面积，小于0.01合格，对于比较厚的物料，适当增大本阈值
    std::tuple<bool, double> shapeDiff = detector.p1_matchShapes(); // 整体轮廓形状分数，小于0.01~0.05合格
    std::tuple<bool, double> subAreaDiff = detector.p2_matchSubAreas(); // 子区域轮廓面积，对于细微的角落缺陷，小于0.02合格
    std::tuple<bool, double> subShapeDiff = detector.p3_matchSubShapes(); // 子区域轮廓形状分数，小于0.002合格
    std::tuple<bool, double> defectScore = detector.p4_fullMatchMatPixel(); // 缺陷像素点数，小于15合格

    qDebug() << "p0_matchArea areaDiff" << std::get<0>(areaDiff) << std::get<1>(areaDiff);
    qDebug() << "p1_matchShapes shapeDiff" << std::get<0>(shapeDiff) << std::get<1>(shapeDiff);
    qDebug() << "p2_matchSubAreas subAreaDiff" << std::get<0>(subAreaDiff) << std::get<1>(subAreaDiff);
    qDebug() << "p3_matchSubShapes subShapeDiff" << std::get<0>(subShapeDiff) << std::get<1>(subShapeDiff);
    qDebug() << "p4_fullMatchMatPixel defectScore" << std::get<0>(defectScore) << std::get<1>(defectScore);

    QString color = std::get<0>(areaDiff) ? "green" : "red";
    m_resultText->append("-------------------------");
    m_resultText->append(m_currentProcessImageFile);
    m_resultText->append(QString("<font color=\"%1\">总轮廓面积 %2 %3</font>").arg(std::get<0>(areaDiff) ? "green" : "red").arg(std::get<0>(areaDiff) ? "通过" : "失败").arg(std::get<1>(areaDiff)));
    m_resultText->append(QString("<font color=\"%1\">总轮廓分数 %2 %3</font>").arg(std::get<0>(shapeDiff) ? "green" : "red").arg(std::get<0>(shapeDiff) ? "通过" : "失败").arg(std::get<1>(shapeDiff)));
    m_resultText->append(QString("<font color=\"%1\">子轮廓面积 %2 %3</font>").arg(std::get<0>(subAreaDiff) ? "green" : "red").arg(std::get<0>(subAreaDiff) ? "通过" : "失败").arg(std::get<1>(subAreaDiff)));
    m_resultText->append(QString("<font color=\"%1\">子轮廓分数 %2 %3</font>").arg(std::get<0>(subShapeDiff) ? "green" : "red").arg(std::get<0>(subShapeDiff) ? "通过" : "失败").arg(std::get<1>(subShapeDiff)));
    m_resultText->append(QString("<font color=\"%1\">缺陷像素 %2 %3</font>").arg(std::get<0>(defectScore) ? "green" : "red").arg(std::get<0>(defectScore) ? "通过" : "失败").arg(std::get<1>(defectScore)));
    m_resultText->append(QString("elapsed: %1 ms").arg(double(timer.nsecsElapsed()) / 1e6));
    m_resultText->append("-------------------------");
}

void DefectDetectPage::createComponents() {
    GroupBox *fileGroupBox = [=]() {
        GroupBox *fileGroupBox = new GroupBox("单张");
        fileGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        m_selectFileWidget = new SelectFileWidget();

        ClickableWidget *roundWidget = new ClickableWidget;
        roundWidget->setFixedSize(300, 100);
        ImageInfoWidget *imageInfoWidget = new ImageInfoWidget();
        Layouting::ColumnWithMargin{imageInfoWidget}.attachTo(roundWidget);

        connect(m_selectFileWidget, &SelectFileWidget::fileChanged, this, [=](const QString &imageFilePath) {
            if (imageFilePath.isEmpty()) {
                return;
            }

            imageInfoWidget->setFileInfo(QFileInfo(imageFilePath));

            m_currentProcessImageFile = imageFilePath;

            m_imageGridWidget->clearAllImages();
            this->runDefectDetectAlgo(imageFilePath);
        });
        connect(roundWidget, &ClickableWidget::clicked, this, [=]() {
            qDebug() << "selectFileWidget->getSelectFile()" << m_selectFileWidget->getSelectFile();

            m_currentProcessImageFile = m_selectFileWidget->getSelectFile();

            m_imageGridWidget->clearAllImages();
            this->runDefectDetectAlgo(m_selectFileWidget->getSelectFile());
        });

        Layouting::ColumnWithMargin{m_selectFileWidget, Layouting::Space{5}, roundWidget}.attachTo(fileGroupBox);

        return fileGroupBox;
    }();

    GroupBox *folderGroupBox = [=]() {
        GroupBox *folderGroupBox = new GroupBox("目录");
        folderGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

        m_selectFolderWidget = new SelectFolderWidget();
        ImageListViewWidget *imageListViewWidget = new ImageListViewWidget();

        auto batchProcessButton = new NormalButton("批量处理", this);
        batchProcessButton->setFixedWidth(100);
        batchProcessButton->setEnabled(false);

        connect(m_selectFolderWidget, &SelectFolderWidget::folderChanged, this, [=](const QString &folderPath) {
            QFileInfo info(folderPath);
            if (!info.exists()) {
                batchProcessButton->setEnabled(false);
            } else {
                imageListViewWidget->loadImagesFromFolder(folderPath);
                batchProcessButton->setEnabled(true);
            }
        });
        connect(imageListViewWidget, &ImageListViewWidget::imageSelected, this, [=](const QString &imageFilePath) {
            qDebug() << "imageSelected:" << imageFilePath;

            m_currentProcessImageFile = imageFilePath;

            m_imageGridWidget->clearAllImages();
            this->runDefectDetectAlgo(imageFilePath);
        });

        connect(batchProcessButton, &QPushButton::clicked, this, [=]() {
            qDebug() << "录入模板";

            QString folderPath = m_selectFolderWidget->getSelectFolder();
            QDir dir(folderPath);
            QString absolutePath = dir.absolutePath();

            QString processFolder = absolutePath + "_cutout";
            QString templateFolder = processFolder + "_template";
            QString binaryFolder = processFolder + "_binary";
            QString colorObjectFolder = processFolder + "_colorObject";

            FileUtils::removeFolder(templateFolder);
            FileUtils::removeFolder(binaryFolder);
            FileUtils::removeFolder(colorObjectFolder);

            auto filesList = FileUtils::findAllImageFiles(folderPath, true);

            qDebug() << "fileList.size" << filesList;

            for (int i = 0; i < filesList.size(); ++i) {
                QString filePath = filesList[i];
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.fileName();

                cv::Mat imageMat = cv::imread(filePath.toStdString());
                if (!imageMat.empty()) {

                }
            }

            FileUtils::showInFolder(processFolder);
        });

        Layouting::ColumnWithMargin{m_selectFolderWidget, Layouting::Space{5}, imageListViewWidget, batchProcessButton}.attachTo(folderGroupBox);

        return folderGroupBox;
    }();

    // WidgetBase *middlePart = new WidgetBase();
    // middlePart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    GroupBox *paramGroupBox = [=]() {
        GroupBox *paramGroupBox = new GroupBox("参数调整");
        paramGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
        paramGroupBox->setFixedWidth(300);
        paramGroupBox->setFixedHeight(300);

        Layouting::ColumnWithMargin{}.attachTo(paramGroupBox);

        return paramGroupBox;
    }();

    m_imageGridWidget = new ImageGridWidget;
    m_imageGridWidget->setMaxColumns(1);
    // m_imageGridWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    GroupBox *imageResultGroupBox = new GroupBox("输入图片处理结果");
    // imageResultGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    Layouting::ColumnWithMargin{m_imageGridWidget}.attachTo(imageResultGroupBox);

    m_templateGridWidget = new TemplateGridWidget;
    m_templateGridWidget->setMaxColumns(2);
    // m_templateGridWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_resultText = new TextEdit;
    m_resultText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    GroupBox *templateResultGroupBox = new GroupBox("模板匹配结果");
    // templateResultGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    Layouting::ColumnWithMargin{m_templateGridWidget, Layouting::Space{5}, m_resultText}.attachTo(templateResultGroupBox);

    GroupBox *templateGroupBox = [=]() {
        GroupBox *folderGroupBox = new GroupBox("模板目录");
        // folderGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

        m_selectTemplateFolderWidget = new SelectFolderWidget();
        ImageListViewWidget *templateListViewWidget = new ImageListViewWidget();

        connect(m_selectTemplateFolderWidget, &SelectFolderWidget::folderChanged, this, [=](const QString &folderPath) {
            QFileInfo info(folderPath);
            if (!info.exists()) {

            } else {
                templateListViewWidget->loadImagesFromFolder(folderPath);

                // QStringList baseNameResult;
                // QStringList fullNameResult;
                // std::tie(baseNameResult, fullNameResult) = FileUtils::findDepth1Folder(folderPath);
                // matcher.setTemplateFolder(baseNameResult, fullNameResult);
            }
        });
        connect(templateListViewWidget, &ImageListViewWidget::imageSelected, this, [=](const QString &imageFilePath) {
            qDebug() << "templateSelected:" << imageFilePath;
            return;

            // m_currentProcessImageFile = imageFilePath;

            // m_imageGridWidget->clearAllImages();
            // this->runCutoutAlgo(imageFilePath);
        });

        Layouting::ColumnWithMargin{m_selectTemplateFolderWidget, Layouting::Space{5}, templateListViewWidget}.attachTo(folderGroupBox);

        return folderGroupBox;
    }();

    auto rightPart = Layouting::Column{paramGroupBox, templateGroupBox};

    QVBoxLayout* resultLayout = new QVBoxLayout;
    resultLayout->addWidget(imageResultGroupBox, 1);
    resultLayout->addSpacing(5);
    resultLayout->addWidget(templateResultGroupBox, 2);

    auto middlePart = Layouting::Column{resultLayout};

    auto leftPart = Layouting::Column{fileGroupBox, Layouting::Space{5}, folderGroupBox};

    Layouting::RowWithMargin{leftPart, Layouting::Space{5}, middlePart, Layouting::Space{5}, rightPart}.attachTo(this);
}

void DefectDetectPage::createConnections() {
    connect(this, &DefectDetectPage::paramChanged, this, [=]() {
        qDebug() << "paramChanged m_currentProcessImageFile" << m_currentProcessImageFile;

        // m_imageGridWidget->clearAllImages();
        this->runDefectDetectAlgo(m_currentProcessImageFile);
    });
}

void DefectDetectPage::loadConfig() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString configDir = appDir + "/config";

    // 创建配置目录（如果不存在）
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_configPath = configDir + "/DefectDetectPage.ini";
    m_settings = new QSettings(m_configPath, QSettings::IniFormat, this);

    // 设置配置文件编码
    m_settings->setIniCodec("UTF-8");

    if (!QFile::exists(m_configPath)) {
        qDebug() << "配置文件不存在，创建默认配置";

        // 设置默认值
        QString defaultPath = QCoreApplication::applicationDirPath();
        m_settings->setValue("Path/filePath", defaultPath);
        m_settings->setValue("Path/folderPath", defaultPath);
        m_settings->setValue("Path/templateFolderPath", defaultPath);
        // m_settings->setValue("Parameter/scoreThreshold", 0.1);
        // m_settings->setValue("Parameter/whiteThreshold", 250);
        // m_settings->setValue("Parameter/areaThreshold", 0.2);

        m_settings->sync();
    } else {
        qDebug() << "从配置文件加载:" << m_configPath;
    }

    m_selectFileWidget->setSelectFile(m_settings->value("Path/filePath", "").toString());
    m_selectFolderWidget->setSelectFolder(m_settings->value("Path/folderPath", "").toString());
    m_selectTemplateFolderWidget->setSelectFolder(m_settings->value("Path/templateFolderPath", "").toString());
    // m_scoreThresholdSpinBox->setValue(m_settings->value("Parameter/scoreThreshold", 0.1).toDouble());
    // m_whiteThresholdSpinBox->setValue(m_settings->value("Parameter/whiteThreshold", 250).toInt());
    // m_areaThresholdSpinBox->setValue(m_settings->value("Parameter/areaThreshold", 0.2).toDouble());

    qDebug() << "DefectDetectPage::loadConfig()";
}

void DefectDetectPage::saveConfig() {
    m_settings->setValue("Path/filePath", m_selectFileWidget->getSelectFile());
    m_settings->setValue("Path/folderPath", m_selectFolderWidget->getSelectFolder());
    m_settings->setValue("Path/templateFolderPath", m_selectTemplateFolderWidget->getSelectFolder());
    // m_settings->setValue("Parameter/scoreThreshold", m_scoreThresholdSpinBox->value());
    // m_settings->setValue("Parameter/whiteThreshold", m_whiteThresholdSpinBox->value());
    // m_settings->setValue("Parameter/areaThreshold", m_areaThresholdSpinBox->value());

    m_settings->sync();

    qDebug() << "DefectDetectPage::saveConfig()";
}
