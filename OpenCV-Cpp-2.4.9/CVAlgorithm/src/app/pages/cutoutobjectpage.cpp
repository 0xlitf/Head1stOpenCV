#include "cutoutobjectpage.h"
#include "controls/groupbox.h"
#include "controls/clickablewidget.h"
#include "cutoutobject.h"
#include "imageutils.h"
#include "utils/fileutils.h"
#include "widgets/imagegridwidget.h"
#include "widgets/imagelistwidget.h"
#include "widgets/selectfilewidget.h"
#include "widgets/selectfolderwidget.h"
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>

CutoutObjectPage::CutoutObjectPage(QWidget *parent) : WidgetBase{parent} {
    this->createComponents();
    this->createConnections();
    this->loadConfig();
}

CutoutObjectPage::~CutoutObjectPage() { this->saveConfig(); }

void CutoutObjectPage::createComponents() {
    GroupBox *fileGroupBox = [=]() {
        GroupBox *fileGroupBox = new GroupBox("单张");
        fileGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        m_selectFileWidget = new SelectFileWidget();

        ClickableWidget *roundWidget = new ClickableWidget;
        roundWidget->setFixedSize(300, 100);
        ImageInfoWidget *imageInfoWidget = new ImageInfoWidget();
        Layouting::ColumnWithMargin{imageInfoWidget}.attachTo(roundWidget);

        connect(m_selectFileWidget, &SelectFileWidget::fileChanged, this, [=](const QString &imageFilePath) {
            imageInfoWidget->setFileInfo(QFileInfo(imageFilePath));

            m_currentProcessImageFile = imageFilePath;

            m_imageGridWidget->clearAllImages();
            this->runCutoutAlgo(imageFilePath);
        });
        connect(roundWidget, &ClickableWidget::clicked, this, [=]() {
            qDebug() << "selectFileWidget->getSelectFile()" << m_selectFileWidget->getSelectFile();

            m_currentProcessImageFile = m_selectFileWidget->getSelectFile();

            m_imageGridWidget->clearAllImages();
            this->runCutoutAlgo(m_selectFileWidget->getSelectFile());
        });

        Layouting::ColumnWithMargin{m_selectFileWidget, Layouting::Space{5}, roundWidget}.attachTo(fileGroupBox);

        return fileGroupBox;
    }();

    GroupBox *folderGroupBox = [=]() {
        GroupBox *folderGroupBox = new GroupBox("目录");
        folderGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

        m_selectFolderWidget = new SelectFolderWidget();
        ImageListWidget *imageListWidget = new ImageListWidget();

        auto batchProcessButton = new NormalButton("批量处理", this);
        batchProcessButton->setFixedWidth(100);
        batchProcessButton->setEnabled(false);

        connect(m_selectFolderWidget, &SelectFolderWidget::folderChanged, this, [=](const QString &folderPath) {
            QFileInfo info(folderPath);
            if (!info.exists()) {
                batchProcessButton->setEnabled(false);
            } else {
                imageListWidget->loadImagesFromFolder(folderPath);
                batchProcessButton->setEnabled(true);
            }
        });
        connect(imageListWidget, &ImageListWidget::imageSelected, this, [=](const QString &imageFilePath) {
            qDebug() << "imageSelected:" << imageFilePath;

            m_currentProcessImageFile = imageFilePath;

            m_imageGridWidget->clearAllImages();
            this->runCutoutAlgo(imageFilePath);
        });

        connect(batchProcessButton, &QPushButton::clicked, this, [=]() {
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

            CutOutObject cutout;

            for (int i = 0; i < filesList.size(); ++i) {
                QString filePath = filesList[i];
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.fileName();

                cv::Mat imageMat = cv::imread(filePath.toStdString());
                if (!imageMat.empty()) {
                    // m_imageGridWidget->addImage("原图", imageMat);

                    double minArea = 1000.0;
                    double maxArea = 100000.0;

                    cv::Mat eraseBlueBackground;
                    cv::Mat singleChannelZeroImage;
                    std::tie(eraseBlueBackground, singleChannelZeroImage) = cutout.eraseBlueBackground(imageMat, m_colorSpinBox->value(), m_blueSpinBox->value());

                    // m_imageGridWidget->addImage("eraseBlueBackground",
                    // eraseBlueBackground);
                    // m_imageGridWidget->addImage("singleChannelZeroImage",
                    // singleChannelZeroImage);

                    std::vector<ObjectDetectionResult> results = cutout.extractMultipleObjects(singleChannelZeroImage, minArea, maxArea);

                    cv::Mat mask = cutout.getMultipleObjectsInOriginalSize(results, eraseBlueBackground);
                    cv::Mat objsInfo = cutout.drawObjectsInfo(results, singleChannelZeroImage);

                    // m_imageGridWidget->addImage("mask", mask);
                    // m_imageGridWidget->addImage("objsInfo", objsInfo);

                    cv::Mat whiteBackground(singleChannelZeroImage.size(), CV_8UC3, cv::Scalar(255, 255, 255));
                    cv::Mat closeContour = cutout.drawObjectsContour(results, whiteBackground);
                    // m_imageGridWidget->addImage("closeContour", closeContour);

                    QString binarySavePath = filePath;
                    binarySavePath.replace(folderPath, binaryFolder);
                    FileUtils::makeFilePath(binarySavePath);
                    cv::imwrite(binarySavePath.toStdString(), closeContour);


                    cv::Mat colorObject = cutout.getObjectUnderMask(eraseBlueBackground, closeContour);

                    QString colorObjectSavePath = filePath;
                    colorObjectSavePath.replace(folderPath, colorObjectFolder);
                    FileUtils::makeFilePath(colorObjectSavePath);
                    cv::imwrite(colorObjectSavePath.toStdString(), colorObject);


                    std::vector<cv::Mat> boundings = cutout.getMultipleObjectsInBoundingRect(results);

                    int i = 0;
                    for (auto &mat : boundings) {
                        // m_imageGridWidget->addImage(QString("bounding %1").arg(i), mat);
                        QString templateSavePath = filePath;
                        templateSavePath.replace(folderPath, templateFolder);
                        bool ok = FileUtils::makeFilePath(templateSavePath);

                        cv::imwrite(templateSavePath.toStdString(), mat);
                        ++i;

                        break;
                    }
                }
            }

            FileUtils::showInFolder(processFolder);
        });

        Layouting::ColumnWithMargin{m_selectFolderWidget, Layouting::Space{5}, imageListWidget, batchProcessButton}.attachTo(folderGroupBox);

        return folderGroupBox;
    }();

    WidgetBase *rightPart = new WidgetBase();
    rightPart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    GroupBox *paramGroupBox = [=]() {
        GroupBox *paramGroupBox = new GroupBox("参数调整");
        paramGroupBox->setFixedHeight(150);

        int relativeThresholdValue = 30;
        int blueThresholdValue = 50;

        auto colorThresLayout = [=]() {
            QLabel *colorLabel = new QLabel("相对阈值");
            colorLabel->setAlignment(Qt::AlignCenter);

            QSlider *colorSlider = new QSlider(Qt::Horizontal, this);
            colorSlider->setFixedSize(QSize(200, 25));

            colorSlider->setSingleStep(1);
            colorSlider->setPageStep(10);
            colorSlider->setRange(0, 255);

            colorSlider->setTickPosition(QSlider::NoTicks);
            colorSlider->setTickInterval(50);

            m_colorSpinBox = new QSpinBox();
            m_colorSpinBox->setRange(0, 255);
            m_colorSpinBox->setFixedSize(QSize(50, 30));

            connect(colorSlider, &QSlider::valueChanged, m_colorSpinBox, [=](int value) { m_colorSpinBox->setValue(value); });
            connect(m_colorSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), colorSlider, [=](int value) {
                colorSlider->setValue(value);

                emit this->paramChanged();
            });

            m_colorSpinBox->setValue(relativeThresholdValue);

            return Layouting::RowWithMargin{colorLabel, Layouting::Space{5}, colorSlider, Layouting::Space{5}, m_colorSpinBox, Layouting::Stretch{}};
        }();

        auto blueThresLayout = [=]() {
            QLabel *blueLabel = new QLabel("蓝色阈值");
            blueLabel->setAlignment(Qt::AlignCenter);

            QSlider *blueSlider = new QSlider(Qt::Horizontal, this);
            blueSlider->setFixedSize(QSize(200, 25));

            blueSlider->setSingleStep(1);
            blueSlider->setPageStep(10);
            blueSlider->setRange(0, 255);

            blueSlider->setTickPosition(QSlider::NoTicks);
            blueSlider->setTickInterval(50);

            m_blueSpinBox = new QSpinBox(this);
            m_blueSpinBox->setRange(0, 255);
            m_blueSpinBox->setFixedSize(QSize(50, 30));

            connect(blueSlider, &QSlider::valueChanged, m_blueSpinBox, [=](int value) { m_blueSpinBox->setValue(value); });
            connect(m_blueSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), blueSlider, [=](int value) {
                blueSlider->setValue(value);

                emit this->paramChanged();
            });

            m_blueSpinBox->setValue(blueThresholdValue);

            return Layouting::RowWithMargin{blueLabel, Layouting::Space{5}, blueSlider, Layouting::Space{5}, m_blueSpinBox, Layouting::Stretch{}};
        }();

        Layouting::ColumnWithMargin{colorThresLayout, blueThresLayout}.attachTo(paramGroupBox);

        return paramGroupBox;
    }();

    m_imageGridWidget = new ImageGridWidget;
    m_imageGridWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    GroupBox *resultGroupBox = new GroupBox("处理结果");
    Layouting::ColumnWithMargin{m_imageGridWidget}.attachTo(resultGroupBox);

    Layouting::Column{paramGroupBox, Layouting::Space{5}, resultGroupBox}.attachTo(rightPart);
    auto leftPart = Layouting::Column{fileGroupBox, Layouting::Space{5}, folderGroupBox};

    Layouting::RowWithMargin{
        leftPart,
        Layouting::Space{5},
        rightPart,
    }
        .attachTo(this);
}

void CutoutObjectPage::createConnections() {
    connect(this, &CutoutObjectPage::paramChanged, this, [=]() {
        qDebug() << "paramChanged" << m_colorSpinBox->value() << m_blueSpinBox->value() << m_currentProcessImageFile;

        // m_imageGridWidget->clearAllImages();
        this->runCutoutAlgo(m_currentProcessImageFile);
    });
}

void CutoutObjectPage::runCutoutAlgo(const QString &filePath) {
    cv::Mat imageMat = cv::imread(filePath.toStdString());
    if (!imageMat.empty()) {
        // 获取一个唯一的标识名
        QString imageName = filePath;
        m_imageGridWidget->addImage("原图", imageMat);

        CutOutObject cutout;

        double minArea = 1000.0;
        double maxArea = 100000.0;

        cv::Mat eraseBlueBackground;
        cv::Mat singleChannelZeroImage;
        std::tie(eraseBlueBackground, singleChannelZeroImage) = cutout.eraseBlueBackground(imageMat, m_colorSpinBox->value(), m_blueSpinBox->value());

        m_imageGridWidget->addImage("eraseBlueBackground", eraseBlueBackground);
        m_imageGridWidget->addImage("singleChannelZeroImage", singleChannelZeroImage);

        std::vector<ObjectDetectionResult> results = cutout.extractMultipleObjects(singleChannelZeroImage, minArea, maxArea);

        cv::Mat mask = cutout.getMultipleObjectsInOriginalSize(results, eraseBlueBackground);
        cv::Mat objsInfo = cutout.drawObjectsInfo(results, singleChannelZeroImage);

        m_imageGridWidget->addImage("mask", mask);
        m_imageGridWidget->addImage("objsInfo", objsInfo);

        cv::Mat whiteBackground(singleChannelZeroImage.size(), CV_8UC3, cv::Scalar(255, 255, 255));
        cv::Mat closeContour = cutout.drawObjectsContour(results, whiteBackground);
        m_imageGridWidget->addImage("closeContour", closeContour);

        // std::vector<cv::Mat> boundings =
        // cutout.getMultipleObjectsInBoundingRect(results);

        // int i = 0;
        // for (auto &mat : boundings) {
        //     m_imageGridWidget->addImage(QString("bounding %1").arg(i), mat);
        //     ++i;
        // }
    }
}

void CutoutObjectPage::loadConfig() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString configDir = appDir + "/config";

    // 创建配置目录（如果不存在）
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_configPath = configDir + "/CutoutObjectPage.ini";
    m_settings = new QSettings(m_configPath, QSettings::IniFormat, this);

    // 设置配置文件编码
    m_settings->setIniCodec("UTF-8");

    if (!QFile::exists(m_configPath)) {
        qDebug() << "配置文件不存在，创建默认配置";

        // 设置默认值
        QString defaultPath = QCoreApplication::applicationDirPath();
        // m_settings->setValue("Path/filePath", defaultPath);
        // m_settings->setValue("Path/folderPath", defaultPath);
        m_settings->setValue("Parameter/relativeThreshold", 30);
        m_settings->setValue("Parameter/blueThreshold", 50);

        m_settings->sync();
    } else {
        qDebug() << "从配置文件加载:" << m_configPath;
    }

    m_selectFileWidget->setSelectFile(m_settings->value("Path/filePath", "").toString());
    m_selectFolderWidget->setSelectFolder(m_settings->value("Path/folderPath", "").toString());
    m_colorSpinBox->setValue(m_settings->value("Parameter/relativeThreshold", 30).toInt());
    m_blueSpinBox->setValue(m_settings->value("Parameter/blueThreshold", 50).toInt());

    qDebug() << "CutoutObjectPage::loadConfig()";
}

void CutoutObjectPage::saveConfig() {
    m_settings->setValue("Path/filePath", m_selectFileWidget->getSelectFile());
    m_settings->setValue("Path/folderPath", m_selectFolderWidget->getSelectFolder());
    m_settings->setValue("Parameter/relativeThreshold", m_colorSpinBox->value());
    m_settings->setValue("Parameter/blueThreshold", m_blueSpinBox->value());

    m_settings->sync();

    qDebug() << "CutoutObjectPage::saveConfig()";
}
