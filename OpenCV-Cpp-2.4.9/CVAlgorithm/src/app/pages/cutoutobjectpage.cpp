#include "cutoutobjectpage.h"
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
    SelectFileWidget *selectFileWidget = new SelectFileWidget(this);
    ImageInfoWidget *imageInfoWidget = new ImageInfoWidget(this);
    imageInfoWidget->setFixedSize(300, 100);
    // imageInfoWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    SelectFolderWidget *selectFolderWidget = new SelectFolderWidget(this);
    ImageListWidget *imageListWidget = new ImageListWidget(this);

    m_imageGridWidget = new ImageGridWidget;

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
    auto leftSelectColumn = Layouting::Column{selectFileWidget, imageInfoWidget, Layouting::Space{5}, selectFolderWidget, Layouting::Space{5}, imageListWidget};

    auto rightSelectColumn = Layouting::Column{Layouting::Space{5}, Layouting::Space{5}, m_imageGridWidget};

    Layouting::RowWithMargin{leftSelectColumn, Layouting::Space{5}, rightSelectColumn}.attachTo(this);
}

void CutoutObjectPage::runCutoutAlgo(const QString &filePath) {
    cv::Mat imageMat = cv::imread(filePath.toStdString());
    if (!imageMat.empty()) {
        // 获取一个唯一的标识名，这里使用文件名示例
        QString imageName = filePath;
        // 将图像添加到网格中
        m_imageGridWidget->addImage("原图", imageMat); // 假设通过ui对象访问

        CutOutObject cutout;

        double minArea = 1000.0;   // 最小面积阈值
        double maxArea = 100000.0; // 最大面积阈值

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
    }
}
