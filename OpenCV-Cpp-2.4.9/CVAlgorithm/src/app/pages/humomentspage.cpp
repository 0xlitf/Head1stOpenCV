#include "humomentspage.h"
#include "controls/groupbox.h"
#include "controls/clickablewidget.h"
#include "cutoutobject.h"
#include "imageutils.h"
#include "utils/fileutils.h"
#include "widgets/imagegridwidget.h"
#include "widgets/templategridwidget.h"
#include "widgets/imagelistviewwidget.h"
#include "widgets/imagelistwidget.h"
#include "widgets/selectfilewidget.h"
#include "widgets/selectfolderwidget.h"
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>

HuMomentsPage::HuMomentsPage(QWidget *parent) : WidgetBase{parent} {
    this->createComponents();
    this->createConnections();
    this->loadConfig();
}

HuMomentsPage::~HuMomentsPage() { this->saveConfig(); }

void HuMomentsPage::createComponents() {
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

                    cutout.setColorThreshold(m_areaMaxSpinBox->value());
                    cutout.setBlueThreshold(m_areaMinSpinBox->value());

                    std::tie(eraseBlueBackground, singleChannelZeroImage) = cutout.eraseBlueBackground(imageMat);

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


                    cv::Mat colorObject = cutout.getObjectUnderMask(imageMat, closeContour);

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

        Layouting::ColumnWithMargin{m_selectFolderWidget, Layouting::Space{5}, imageListViewWidget, batchProcessButton}.attachTo(folderGroupBox);

        return folderGroupBox;
    }();

    // WidgetBase *middlePart = new WidgetBase();
    // middlePart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    GroupBox *paramGroupBox = [=]() {
        GroupBox *paramGroupBox = new GroupBox("参数调整");
        paramGroupBox->setFixedHeight(150);

        int areaMaxValue = 1000000;
        int areaMinValue = 1000;

        int areaMaxDefaultValue = 100000;
        int areaMinDefaultValue = 2000;

        auto areaMaxThresLayout = [=]() {
            QLabel *areaMaxLabel = new QLabel("面积上限");
            areaMaxLabel->setAlignment(Qt::AlignCenter);

            m_areaMaxSlider = new QSlider(Qt::Horizontal, this);
            m_areaMaxSlider->setFixedSize(QSize(200, 25));

            m_areaMaxSlider->setSingleStep(100);
            m_areaMaxSlider->setPageStep(100);
            m_areaMaxSlider->setRange(1000, areaMaxValue);

            m_areaMaxSlider->setTickPosition(QSlider::NoTicks);
            m_areaMaxSlider->setTickInterval(50);

            m_areaMaxSpinBox = new QSpinBox();
            m_areaMaxSpinBox->setRange(1000, areaMaxValue);
            m_areaMaxSpinBox->setFixedSize(QSize(100, 30));
            m_areaMaxSpinBox->setSingleStep(100);

            m_areaMaxSpinBox->setValue(areaMaxDefaultValue);

            return Layouting::RowWithMargin{areaMaxLabel, Layouting::Space{5}, m_areaMaxSlider, Layouting::Space{5}, m_areaMaxSpinBox, Layouting::Stretch{}};
        }();

        auto areaMinThresLayout = [=]() {
            QLabel *areaMinLabel = new QLabel("面积下限");
            areaMinLabel->setAlignment(Qt::AlignCenter);

            m_areaMinSlider = new QSlider(Qt::Horizontal, this);
            m_areaMinSlider->setFixedSize(QSize(200, 25));

            m_areaMinSlider->setSingleStep(100);
            m_areaMinSlider->setPageStep(100);
            m_areaMinSlider->setRange(1000, areaMaxValue);

            m_areaMinSlider->setTickPosition(QSlider::NoTicks);
            m_areaMinSlider->setTickInterval(50);

            m_areaMinSpinBox = new QSpinBox(this);
            m_areaMinSpinBox->setRange(1000, areaMaxValue);
            m_areaMinSpinBox->setFixedSize(QSize(100, 30));
            m_areaMinSpinBox->setSingleStep(100);

            m_areaMinSpinBox->setValue(areaMinDefaultValue);

            return Layouting::RowWithMargin{areaMinLabel, Layouting::Space{5}, m_areaMinSlider, Layouting::Space{5}, m_areaMinSpinBox, Layouting::Stretch{}};
        }();

        connect(m_areaMaxSlider, &QSlider::valueChanged, m_areaMaxSpinBox, [=](int value) {
            // if (value < m_areaMinSlider->value()) {
            //     m_areaMinSlider->setValue(value);
            // }
            m_areaMinSlider->setMaximum(value);
            m_areaMinSpinBox->setMaximum(value);

            m_areaMaxSpinBox->setValue(value);
        });
        connect(m_areaMaxSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_areaMaxSlider, [=](int value) {
            m_areaMinSlider->setMaximum(value);
            m_areaMinSpinBox->setMaximum(value);

            m_areaMaxSlider->setValue(value);

            emit this->paramChanged();
        });

        connect(m_areaMinSlider, &QSlider::valueChanged, m_areaMinSpinBox, [=](int value) {
            m_areaMaxSlider->setMinimum(value);
            m_areaMaxSpinBox->setMinimum(value);

            m_areaMinSpinBox->setValue(value);
        });
        connect(m_areaMinSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), m_areaMinSlider, [=](int value) {
            m_areaMaxSlider->setMinimum(value);
            m_areaMaxSpinBox->setMinimum(value);

            m_areaMinSlider->setValue(value);

            emit this->paramChanged();
        });

        Layouting::ColumnWithMargin{areaMaxThresLayout, areaMinThresLayout}.attachTo(paramGroupBox);

        return paramGroupBox;
    }();

    m_imageGridWidget = new ImageGridWidget;
    m_imageGridWidget->setMaxColumns(1);
    m_imageGridWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    GroupBox *imageResultGroupBox = new GroupBox("输入图片处理结果");
    Layouting::ColumnWithMargin{m_imageGridWidget}.attachTo(imageResultGroupBox);

    m_templateGridWidget = new TemplateGridWidget;
    m_templateGridWidget->setMaxColumns(2);
    m_templateGridWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    GroupBox *templateResultGroupBox = new GroupBox("模板匹配结果");
    Layouting::ColumnWithMargin{m_templateGridWidget}.attachTo(templateResultGroupBox);

    // WidgetBase *rightPart = new WidgetBase();
    // rightPart->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);


    GroupBox *templateGroupBox = [=]() {
        GroupBox *folderGroupBox = new GroupBox("模板目录");
        folderGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

        m_selectTemplateFolderWidget = new SelectFolderWidget();
        ImageListViewWidget *templateListViewWidget = new ImageListViewWidget();

        connect(m_selectTemplateFolderWidget, &SelectFolderWidget::folderChanged, this, [=](const QString &folderPath) {
            QFileInfo info(folderPath);
            if (!info.exists()) {

            } else {
                templateListViewWidget->loadImagesFromFolder(folderPath);
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

    auto rightPart = Layouting::Column{templateGroupBox};

    QHBoxLayout* resultLayout = new QHBoxLayout;
    resultLayout->addWidget(imageResultGroupBox, 1);
    resultLayout->addSpacing(5);
    resultLayout->addWidget(templateResultGroupBox, 2);

    auto middlePart = Layouting::Column{paramGroupBox, Layouting::Space{5}, resultLayout};

    auto leftPart = Layouting::Column{fileGroupBox, Layouting::Space{5}, folderGroupBox};

    Layouting::RowWithMargin{leftPart, Layouting::Space{5}, middlePart, Layouting::Space{5}, rightPart}.attachTo(this);
}

void HuMomentsPage::createConnections() {
    connect(this, &HuMomentsPage::paramChanged, this, [=]() {
        qDebug() << "paramChanged" << m_areaMaxSpinBox->value() << m_areaMinSpinBox->value() << m_currentProcessImageFile;

        // m_imageGridWidget->clearAllImages();
        this->runCutoutAlgo(m_currentProcessImageFile);
    });
}

void HuMomentsPage::runCutoutAlgo(const QString &filePath) {
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

        cutout.setColorThreshold(m_areaMaxSpinBox->value());
        cutout.setBlueThreshold(m_areaMinSpinBox->value());

        std::tie(eraseBlueBackground, singleChannelZeroImage) = cutout.eraseBlueBackground(imageMat);

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

void HuMomentsPage::loadConfig() {
    QString appDir = QCoreApplication::applicationDirPath();
    QString configDir = appDir + "/config";

    // 创建配置目录（如果不存在）
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_configPath = configDir + "/HuMomentsPage.ini";
    m_settings = new QSettings(m_configPath, QSettings::IniFormat, this);

    // 设置配置文件编码
    m_settings->setIniCodec("UTF-8");

    if (!QFile::exists(m_configPath)) {
        qDebug() << "配置文件不存在，创建默认配置";

        // 设置默认值
        QString defaultPath = QCoreApplication::applicationDirPath();
        // m_settings->setValue("Path/filePath", defaultPath);
        // m_settings->setValue("Path/folderPath", defaultPath);
        m_settings->setValue("Parameter/areaMaxValue", 100000);
        m_settings->setValue("Parameter/areaMinValue", 2000);

        m_settings->sync();
    } else {
        qDebug() << "从配置文件加载:" << m_configPath;
    }

    m_selectFileWidget->setSelectFile(m_settings->value("Path/filePath", "").toString());
    m_selectFolderWidget->setSelectFolder(m_settings->value("Path/folderPath", "").toString());
    m_selectTemplateFolderWidget->setSelectFolder(m_settings->value("Path/templateFolderPath", "").toString());
    m_areaMaxSpinBox->setValue(m_settings->value("Parameter/areaMaxValue", 100000).toInt());
    m_areaMinSpinBox->setValue(m_settings->value("Parameter/areaMinValue", 2000).toInt());

    qDebug() << "HuMomentsPage::loadConfig()";
}

void HuMomentsPage::saveConfig() {
    m_settings->setValue("Path/filePath", m_selectFileWidget->getSelectFile());
    m_settings->setValue("Path/folderPath", m_selectFolderWidget->getSelectFolder());
    m_settings->setValue("Path/templateFolderPath", m_selectTemplateFolderWidget->getSelectFolder());
    m_settings->setValue("Parameter/areaMaxValue", m_areaMaxSpinBox->value());
    m_settings->setValue("Parameter/areaMinValue", m_areaMinSpinBox->value());

    m_settings->sync();

    qDebug() << "HuMomentsPage::saveConfig()";
}
