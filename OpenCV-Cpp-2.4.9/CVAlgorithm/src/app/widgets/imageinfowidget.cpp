#include "imageinfowidget.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QVBoxLayout>

ImageInfoWidget::ImageInfoWidget(QWidget *parent)
    : WidgetBase(parent), m_thumbnailLabel(new QLabel), m_nameLabel(new QLabel),
    m_infoLabel(new QLabel), m_fileSizeBytes(0) {
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    this->setBackgroundColor(QColor(233, 233, 233));
    // this->setRandomColor();

    setupUI();
}

void ImageInfoWidget::setupUI() {
    m_thumbnailLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_thumbnailLabel->setFixedSize(80, 60); // 固定缩略图大小
    m_thumbnailLabel->setStyleSheet(
        "border: 1px solid #cccccc; background-color: #f0f0f0;");
    m_thumbnailLabel->setAlignment(Qt::AlignCenter);
    m_thumbnailLabel->setScaledContents(false);

    // 允许缩略图接收鼠标点击事件
    m_thumbnailLabel->setCursor(Qt::PointingHandCursor);
    m_thumbnailLabel->installEventFilter(this); // 或者重写widget的mousePressEvent

    // 文件名样式
    m_nameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_nameLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    m_nameLabel->setWordWrap(true);

    // 图片信息样式
    m_infoLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_infoLabel->setStyleSheet("color: #666666; font-size: 10pt;");
    m_infoLabel->setWordWrap(true);

    Layouting::RowWithMargin{m_thumbnailLabel, Layouting::ColumnWithMargin{m_nameLabel, m_infoLabel, }}.attachTo(this);
}

void ImageInfoWidget::setFileInfo(const QFileInfo &fileInfo) {
    setThumbnail(QPixmap());
    m_fileName = fileInfo.fileName();
    m_fileSizeBytes = fileInfo.size();

    m_fileName = fileInfo.fileName();
    m_fileSizeBytes = fileInfo.size();

    // 获取图片尺寸并生成缩略图
    QImage image(fileInfo.absoluteFilePath());
    if (!image.isNull()) {
        m_imageSize = image.size();
        // 生成并设置缩略图
        QPixmap thumbnail =
            generateThumbnail(fileInfo.absoluteFilePath(), QSize(80, 60));
        setThumbnail(thumbnail);
    } else {
        m_imageSize = QSize(0, 0);
        // 可以设置一个默认的错误图标
        m_thumbnailLabel->setText("无效图片");
    }

    updateDisplay();
}

QPixmap ImageInfoWidget::generateThumbnail(const QString &filePath,
                                           const QSize &size) {
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        // 返回一个空的默认缩略图
        QPixmap defaultThumbnail(size);
        defaultThumbnail.fill(QColor(240, 240, 240));
        return defaultThumbnail;
    }
    // 缩放图片，保持宽高比，使用平滑变换
    return pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void ImageInfoWidget::setThumbnail(const QPixmap &pixmap) {
    m_thumbnailLabel->setPixmap(pixmap);
}

void ImageInfoWidget::updateDisplay() {
    m_nameLabel->setText(m_fileName);

    QString sizeInfo;
    if (m_imageSize.isValid() && m_imageSize.width() > 0 &&
        m_imageSize.height() > 0) {
        sizeInfo = QString("尺寸: %1 x %2 像素\n")
                       .arg(m_imageSize.width())
                       .arg(m_imageSize.height());
    } else {
        sizeInfo = "尺寸: 未知\n";
    }

    QString sizeStr;
    if (m_fileSizeBytes < 1024) {
        sizeStr = QString("%1 B").arg(m_fileSizeBytes);
    } else if (m_fileSizeBytes < 1024 * 1024) {
        sizeStr = QString("%1 KB").arg(m_fileSizeBytes / 1024.0, 0, 'f', 1);
    } else {
        sizeStr =
            QString("%1 MB").arg(m_fileSizeBytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
    QString fileSizeInfo = QString("大小: %1").arg(sizeStr);

    m_infoLabel->setText(sizeInfo + fileSizeInfo);
}

// 可选的单独设置方法
void ImageInfoWidget::setFileName(const QString &fileName) {
    m_fileName = fileName;
    updateDisplay();
}

void ImageInfoWidget::setImageSize(const QSize &size) {
    m_imageSize = size;
    updateDisplay();
}

void ImageInfoWidget::setFileSize(qint64 bytes) {
    m_fileSizeBytes = bytes;
    updateDisplay();
}

// 如果需要处理缩略图点击事件
void ImageInfoWidget::mousePressEvent(QMouseEvent *event) {
    if (m_thumbnailLabel->geometry().contains(event->pos())) {
        emit thumbnailClicked(); // 需要先在头文件中声明这个信号
    }
    QWidget::mousePressEvent(event);
}
