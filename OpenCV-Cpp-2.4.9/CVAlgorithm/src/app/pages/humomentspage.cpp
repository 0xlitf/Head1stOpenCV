#include "humomentspage.h"
#include "controls/clickablewidget.h"
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
            this->runHuMomentsMatch(imageFilePath);
        });
        connect(roundWidget, &ClickableWidget::clicked, this, [=]() {
            qDebug() << "selectFileWidget->getSelectFile()" << m_selectFileWidget->getSelectFile();

            m_currentProcessImageFile = m_selectFileWidget->getSelectFile();

            m_imageGridWidget->clearAllImages();
            this->runHuMomentsMatch(m_selectFileWidget->getSelectFile());
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
            this->runHuMomentsMatch(imageFilePath);
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
        paramGroupBox->setFixedHeight(150);

        auto scoreThresLayout = [=]() {
            double scoreThresholdDefaultValue = 0.1;

            QLabel *scoreThresholdLabel = new QLabel("得分误差");
            scoreThresholdLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
            scoreThresholdLabel->setAlignment(Qt::AlignCenter);

            m_scoreThresholdSlider = new QSlider(Qt::Horizontal, this);
            m_scoreThresholdSlider->setFixedHeight(25);
            // m_areaMaxSlider->setFixedSize(QSize(200, 25));

            m_scoreThresholdSlider->setSingleStep(1);
            m_scoreThresholdSlider->setPageStep(1);
            m_scoreThresholdSlider->setRange(1, 100);

            m_scoreThresholdSlider->setTickPosition(QSlider::NoTicks);
            m_scoreThresholdSlider->setTickInterval(1);

            m_scoreThresholdSlider->setValue(scoreThresholdDefaultValue * 100);

            m_scoreThresholdSpinBox = new QDoubleSpinBox();
            m_scoreThresholdSpinBox->setRange(0.01, 1);
            m_scoreThresholdSpinBox->setFixedSize(QSize(100, 30));
            m_scoreThresholdSpinBox->setSingleStep(0.01);

            m_scoreThresholdSpinBox->setValue(scoreThresholdDefaultValue);

            return Layouting::RowWithMargin{scoreThresholdLabel, Layouting::Space{5}, m_scoreThresholdSlider, Layouting::Space{5}, m_scoreThresholdSpinBox};
        }();

        auto whiteThresLayout = [=]() {
            int whiteThresholdDefaultValue = 250;

            QLabel *whiteThresholdLabel = new QLabel("白色阈值");
            whiteThresholdLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
            whiteThresholdLabel->setAlignment(Qt::AlignCenter);

            m_whiteThresholdSlider = new QSlider(Qt::Horizontal, this);
            m_whiteThresholdSlider->setFixedHeight(25);
            // m_areaMinSlider->setFixedSize(QSize(200, 25));

            m_whiteThresholdSlider->setSingleStep(1);
            m_whiteThresholdSlider->setPageStep(1);
            m_whiteThresholdSlider->setRange(1, 254);

            m_whiteThresholdSlider->setTickPosition(QSlider::NoTicks);
            m_whiteThresholdSlider->setTickInterval(50);

            m_whiteThresholdSlider->setValue(whiteThresholdDefaultValue);

            m_whiteThresholdSpinBox = new QSpinBox(this);
            m_whiteThresholdSpinBox->setRange(1, 254);
            m_whiteThresholdSpinBox->setFixedSize(QSize(100, 30));
            m_whiteThresholdSpinBox->setSingleStep(1000);

            m_whiteThresholdSpinBox->setValue(whiteThresholdDefaultValue);

            return Layouting::RowWithMargin{whiteThresholdLabel, Layouting::Space{5}, m_whiteThresholdSlider, Layouting::Space{5}, m_whiteThresholdSpinBox};
        }();

        auto areaThresLayout = [=]() {
            double areaThresholdDefaultValue = 0.2;

            QLabel *areaThresholdLabel = new QLabel("面积偏差");
            areaThresholdLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
            areaThresholdLabel->setAlignment(Qt::AlignCenter);

            m_areaThresholdSlider = new QSlider(Qt::Horizontal, this);
            m_areaThresholdSlider->setFixedHeight(25);
            // m_areaMinSlider->setFixedSize(QSize(200, 25));

            m_areaThresholdSlider->setSingleStep(1);
            m_areaThresholdSlider->setPageStep(1);
            m_areaThresholdSlider->setRange(1, 100);

            m_areaThresholdSlider->setTickPosition(QSlider::NoTicks);
            m_areaThresholdSlider->setTickInterval(1);

            m_areaThresholdSlider->setValue(areaThresholdDefaultValue * 100);

            m_areaThresholdSpinBox = new QDoubleSpinBox(this);
            m_areaThresholdSpinBox->setRange(0.01, 1);
            m_areaThresholdSpinBox->setFixedSize(QSize(100, 30));
            m_areaThresholdSpinBox->setSingleStep(0.01);

            m_areaThresholdSpinBox->setValue(areaThresholdDefaultValue);

            return Layouting::RowWithMargin{areaThresholdLabel, Layouting::Space{5}, m_areaThresholdSlider, Layouting::Space{5}, m_areaThresholdSpinBox};
        }();

        connect(m_scoreThresholdSlider, &QSlider::valueChanged, this, [=](int value) {
            m_scoreThresholdSpinBox->setValue(value / 100.);
        });
        connect(m_scoreThresholdSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [=](double value) {
            m_scoreThresholdSlider->setValue(value * 100);

            matcher.setScoreThreshold(value);

            emit this->paramChanged();
        });

        connect(m_whiteThresholdSlider, &QSlider::valueChanged, this, [=](int value) {
            m_whiteThresholdSpinBox->setValue(value);
        });
        connect(m_whiteThresholdSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value) {
            m_whiteThresholdSlider->setValue(value);

            matcher.setWhiteThreshold(value);

            emit this->paramChanged();
        });

        connect(m_areaThresholdSlider, &QSlider::valueChanged, this, [=](int value) {
            m_areaThresholdSpinBox->setValue(value / 100.);
        });
        connect(m_areaThresholdSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [=](double value) {
            m_areaThresholdSlider->setValue(value * 100);

            matcher.setAreaThreshold(value);

            emit this->paramChanged();
        });

        Layouting::ColumnWithMargin{scoreThresLayout, whiteThresLayout, areaThresLayout}.attachTo(paramGroupBox);

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

    GroupBox *templateResultGroupBox = new GroupBox("模板匹配结果");
    // templateResultGroupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    Layouting::ColumnWithMargin{m_templateGridWidget}.attachTo(templateResultGroupBox);

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

                QStringList baseNameResult;
                QStringList fullNameResult;
                std::tie(baseNameResult, fullNameResult) = FileUtils::findDepth1Folder(folderPath);
                matcher.setTemplateFolder(baseNameResult, fullNameResult);
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

    auto rightPart = Layouting::Column{paramGroupBox, Layouting::Space{5}, templateGroupBox};

    QHBoxLayout* resultLayout = new QHBoxLayout;
    resultLayout->addWidget(imageResultGroupBox, 1);
    resultLayout->addSpacing(5);
    resultLayout->addWidget(templateResultGroupBox, 2);

    auto middlePart = Layouting::Column{resultLayout};

    auto leftPart = Layouting::Column{fileGroupBox, Layouting::Space{5}, folderGroupBox};

    Layouting::RowWithMargin{leftPart, Layouting::Space{5}, middlePart, Layouting::Space{5}, rightPart}.attachTo(this);
}

void HuMomentsPage::createConnections() {
    connect(this, &HuMomentsPage::paramChanged, this, [=]() {
        qDebug() << "paramChanged" << m_scoreThresholdSpinBox->value() << m_whiteThresholdSpinBox->value() << m_currentProcessImageFile;

        // m_imageGridWidget->clearAllImages();
        this->runHuMomentsMatch(m_currentProcessImageFile);
    });
}

void HuMomentsPage::runHuMomentsMatch(const QString &filePath) {

    m_templateGridWidget->clearAllImages();

    cv::Mat imageMat = cv::imread(filePath.toStdString());
    if (!imageMat.empty()) {
        // 获取一个唯一的标识名
        QString imageName = filePath;
        m_imageGridWidget->addImage("origin image", imageMat);

        auto binary = matcher.binaryProcess(imageMat);
        m_imageGridWidget->addImage("binary", binary);
        QList<MatchResult> matchResults = matcher.quickMatchMat(binary);

        int i = 0;
        for (const MatchResult &result : matchResults) {
            QString name = std::get<0>(result);                   // 名称
            qDebug() << "name" << name;
            if (name.isEmpty()) {
                qDebug() << "图中没有物体或者物体为杂料";
                // continue;
            }

            std::vector<cv::Point> contour = std::get<1>(result); // 轮廓
            cv::Point2f center = std::get<2>(result);             // 中心点
            double score = std::get<3>(result);                   // 分数
            double areaDifferencePercent = std::get<4>(result);   // 面积差值百分比
            QString templateFileName = std::get<5>(result); // 匹配到的模板文件

            qDebug() << "结果" << i + 1 << ":";
            qDebug() << "\t名称:" << name;
            qDebug() << "\t匹配分数:" << QString::number(score, 'f', 6);
            qDebug() << "\t中心坐标: (" << center.x << "," << center.y << ")";
            qDebug() << "\t轮廓点数:" << contour.size();
            qDebug() << "\t面积差值百分比:" << areaDifferencePercent;  // 如果模板没有匹配到，面积差值百分比为-100

            auto mask = imageMat.clone();
            auto resultImage = matcher.drawResultOnImage(mask, result);

            m_templateGridWidget->addImage(QString("matchResult %1").arg(i), resultImage, result);

            auto t = cv::imread(templateFileName.toStdString());
            m_templateGridWidget->addImage(QString(QFileInfo(templateFileName).fileName()), t, result, true);

            ++i;
        }
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
        m_settings->setValue("Parameter/scoreThreshold", 0.1);
        m_settings->setValue("Parameter/whiteThreshold", 250);
        m_settings->setValue("Parameter/areaThreshold", 0.2);

        m_settings->sync();
    } else {
        qDebug() << "从配置文件加载:" << m_configPath;
    }

    m_selectFileWidget->setSelectFile(m_settings->value("Path/filePath", "").toString());
    m_selectFolderWidget->setSelectFolder(m_settings->value("Path/folderPath", "").toString());
    m_selectTemplateFolderWidget->setSelectFolder(m_settings->value("Path/templateFolderPath", "").toString());
    m_scoreThresholdSpinBox->setValue(m_settings->value("Parameter/scoreThreshold", 0.1).toDouble());
    m_whiteThresholdSpinBox->setValue(m_settings->value("Parameter/whiteThreshold", 250).toInt());
    m_areaThresholdSpinBox->setValue(m_settings->value("Parameter/areaThreshold", 0.2).toDouble());

    qDebug() << "HuMomentsPage::loadConfig()";
}

void HuMomentsPage::saveConfig() {
    m_settings->setValue("Path/filePath", m_selectFileWidget->getSelectFile());
    m_settings->setValue("Path/folderPath", m_selectFolderWidget->getSelectFolder());
    m_settings->setValue("Path/templateFolderPath", m_selectTemplateFolderWidget->getSelectFolder());
    m_settings->setValue("Parameter/scoreThreshold", m_scoreThresholdSpinBox->value());
    m_settings->setValue("Parameter/whiteThreshold", m_whiteThresholdSpinBox->value());
    m_settings->setValue("Parameter/areaThreshold", m_areaThresholdSpinBox->value());

    m_settings->sync();

    qDebug() << "HuMomentsPage::saveConfig()";
}
