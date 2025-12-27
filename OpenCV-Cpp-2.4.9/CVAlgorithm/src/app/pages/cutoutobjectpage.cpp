#include "cutoutobjectpage.h"
#include <QElapsedTimer>
#include <QFile>
#include <QLabel>
#include "widgets/selectfilewidget.h"
#include "utils/imageutils.h"
#include "widgets/selectfolderwidget.h"
#include "widgets/imagelistwidget.h"

CutoutObjectPage::CutoutObjectPage(QWidget *parent) : WidgetBase{parent} {
    this->createComponents();

    // QLabel *label = new QLabel("CutoutObjectPage", this);

    CutOutObject cutout;
    // cutout.testExtractLargestContour(imageName.toStdString());

    // QString imageName = QString(PROJECT_DIR) + "/dataset/bg.png"; // 纯背景
    // QString imageName = QString(PROJECT_DIR) + "/dataset/1.png"; // 1个物体
    // QString imageName = QString(PROJECT_DIR) + "/dataset/2.png"; // 2个物体
    QString imageName = QString(PROJECT_DIR) + "/cannotdetectobject.png";
    // QString imageName = QString(PROJECT_DIR) + "/CutOutObjectd_Z6YHwZteAM.png";
    // QString imageName = QString(PROJECT_DIR) + "/Cam1_18-32-25-052.png";
    qDebug() << "imageName:" << imageName;

    QString trueFormat = ImageUtils::getImageFormatBySignature(imageName);
    qDebug() << "图像的真实格式是：" << trueFormat;

    auto image = cv::imread(imageName.toStdString());
    if (image.empty()) {
        qDebug() << "无法读取图像文件:" << imageName;
        return;
    } else {
        int width = image.cols;          // 图像宽度（像素）
        int height = image.rows;         // 图像高度（像素）
        int channels = image.channels(); // 通道数[6](@ref)

        qDebug() << "=== 图像信息 ===";
        qDebug() << "图像路径:" << imageName;
        qDebug() << "宽度:" << width << "像素";
        qDebug() << "高度:" << height << "像素";
        qDebug() << "通道数:" << channels;

        int depth = image.depth();
        QString depthStr;
        switch (depth) {
        case CV_8U:
            depthStr = "8位无符号整数";
            break;
        case CV_32F:
            depthStr = "32位浮点数";
            break;
        // ... 其他深度类型
        default:
            depthStr = "其他";
        }
        qDebug() << "位图深度:" << depthStr.toUtf8().constData();
    }
    cv::imshow("image", image);

    double minArea = 1000.0;   // 最小面积阈值
    double maxArea = 100000.0; // 最大面积阈值

    cutout.testExtractMultipleObjects(imageName, minArea, maxArea);

    QElapsedTimer timer;
    timer.start();

    std::vector<cv::Mat> boundings;
    for (int t = 0; t < 1; ++t) {
        boundings = cutout.getMultipleObjectsInBoundingRect(image, 30, 50, 3,
                                                            minArea, maxArea);
    }
    qDebug() << "getMultipleObjectsInBoundingRect elapsed:" << timer.elapsed();

    timer.restart();
    cv::Mat mask;
    for (int t = 0; t < 1; ++t) {
        mask = cutout.getMultipleObjectsInOriginalSize(image, 30, 50, 3, minArea,
                                                       maxArea);
    }
    qDebug() << "getMultipleObjectsInBoundingRect elapsed:" << timer.elapsed();

    qDebug() << "boundings.size:" << boundings.size();
    int i = 0;
    for (auto &mat : boundings) {
        cv::imshow(QString("bounding %1").arg(i).toStdString(), mat);
        ++i;
    }

    cv::imshow(QString("mask %1").arg(i).toStdString(), mask);

    cv::waitKey(0);
    cv::destroyAllWindows();
}

void CutoutObjectPage::createComponents()
{

    SelectFileWidget* selectFileWidget = new SelectFileWidget(this);
    ImageInfoWidget* imageInfoWidget = new ImageInfoWidget(this);
    imageInfoWidget->setFixedSize(300, 100);
    // imageInfoWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    SelectFolderWidget* selectFolderWidget = new SelectFolderWidget(this);
    ImageListWidget* imageListWidget = new ImageListWidget(this);
    connect(selectFileWidget, &SelectFileWidget::fileChanged, this, [=](const QString& filePath){
        imageInfoWidget->setFileInfo(QFileInfo(filePath));
    });
    connect(selectFolderWidget, &SelectFolderWidget::folderChanged, this, [=](const QString& folderPath){
        imageListWidget->loadImagesFromFolder(folderPath);
    });
    connect(imageListWidget, &ImageListWidget::imageSelected, this, [=](const QString& imageFilePath){
        qDebug() << "imageSelected:" << imageFilePath;
    });
    auto leftSelectColumn = Layouting::Column{selectFileWidget, imageInfoWidget, Layouting::Space{5}, selectFolderWidget, Layouting::Space{5}, imageListWidget};


    Layouting::ColumnWithMargin{leftSelectColumn}.attachTo(this);
}
