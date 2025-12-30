#include "cutoutobjectpage.h"
#include "controls/groupbox.h"
#include "cutoutobject.h"
#include "imageutils.h"
#include "widgets/imagegridwidget.h"
#include "widgets/imagelistwidget.h"
#include "widgets/selectfilewidget.h"
#include "widgets/selectfolderwidget.h"
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>

CutoutObjectPage::CutoutObjectPage(QWidget *parent) : WidgetBase{parent} {
    this->createComponents();
}

void CutoutObjectPage::createComponents() {
    GroupBox *fileGroupBox = new GroupBox("单张");
    fileGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    GroupBox *folderGroupBox = new GroupBox("目录");
    folderGroupBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    SelectFileWidget *selectFileWidget = new SelectFileWidget();

    ImageInfoWidget *imageInfoWidget = new ImageInfoWidget();
    imageInfoWidget->setFixedSize(300, 100);

    SelectFolderWidget *selectFolderWidget = new SelectFolderWidget();

    ImageListWidget *imageListWidget = new ImageListWidget();

    m_imageGridWidget = new ImageGridWidget;
    m_imageGridWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(selectFileWidget, &SelectFileWidget::fileChanged, this, [=](const QString &imageFilePath) {
        imageInfoWidget->setFileInfo(QFileInfo(imageFilePath));
        m_imageGridWidget->clearAllImages();
        this->runCutoutAlgo(imageFilePath);
    });
    connect(selectFolderWidget, &SelectFolderWidget::folderChanged, this, [=](const QString &folderPath) { imageListWidget->loadImagesFromFolder(folderPath); });
    connect(imageListWidget, &ImageListWidget::imageSelected, this, [=](const QString &imageFilePath) {
        qDebug() << "imageSelected:" << imageFilePath;
        m_imageGridWidget->clearAllImages();
        this->runCutoutAlgo(imageFilePath);
    });

    Layouting::ColumnWithMargin{selectFileWidget, Layouting::Space{5}, imageInfoWidget}.attachTo(fileGroupBox);
    Layouting::ColumnWithMargin{selectFolderWidget, Layouting::Space{5}, imageListWidget}.attachTo(folderGroupBox);
    auto leftSelectColumn = Layouting::Column{fileGroupBox, Layouting::Space{5}, folderGroupBox};

    Layouting::RowWithMargin{
        leftSelectColumn,
        Layouting::Space{5},
        m_imageGridWidget,
    }
        .attachTo(this);
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
        std::tie(eraseBlueBackground, singleChannelZeroImage) = cutout.eraseBlueBackground(imageMat, 30, 50);

        m_imageGridWidget->addImage("eraseBlueBackground", eraseBlueBackground);
        m_imageGridWidget->addImage("singleChannelZeroImage", singleChannelZeroImage);

        std::vector<ObjectDetectionResult> results = cutout.extractMultipleObjects(singleChannelZeroImage, minArea, maxArea);

        cv::Mat mask = cutout.getMultipleObjectsInOriginalSize(results, eraseBlueBackground);
        cv::Mat objsInfo = cutout.drawObjectsInfo(results, singleChannelZeroImage);

        m_imageGridWidget->addImage("mask", mask);
        m_imageGridWidget->addImage("objsInfo", objsInfo);

        std::vector<cv::Mat> boundings = cutout.getMultipleObjectsInBoundingRect(results);

        int i = 0;
        for (auto &mat : boundings) {
            m_imageGridWidget->addImage(QString("bounding %1").arg(i), mat);
            ++i;
        }
    }
}
