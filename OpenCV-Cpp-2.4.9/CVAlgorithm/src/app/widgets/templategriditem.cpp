#include "templategriditem.h"
#include "controls/layoutbuilder.h"
#include "imageutils.h"
#include <QDebug>

TemplateGridItem::TemplateGridItem(const QString &imageName, const cv::Mat &imageData, QWidget *parent)
    : QWidget(parent)
    , m_imageName(imageName)
    , m_imageData(imageData)
    , m_imageLabel(new QLabel)
    , m_infoLabel(new QLabel) {
    m_resizeTimer->setSingleShot(true);

    connect(m_resizeTimer, &QTimer::timeout, this, [=] {
        // qDebug() << "TemplateGridItem::updatePixmap";
        this->updatePixmap();
    });

    setupUI();
}

void TemplateGridItem::setImageMat(const cv::Mat &imageData) {
    m_imageData = imageData.clone();
    m_originalPixmap = ImageUtils::cvMatToQPixmap(imageData);
    this->updatePixmap();
}

void TemplateGridItem::setupUI() {
    if (!m_imageData.empty()) {
        m_originalPixmap = ImageUtils::cvMatToQPixmap(m_imageData);
    }

    // updatePixmap();

    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("border: 1px solid #cccccc;");
    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString infoText = QString("名称: %1\n尺寸: %2x%3\n通道: %4").arg(m_imageName).arg(m_imageData.cols).arg(m_imageData.rows).arg(m_imageData.channels());
    m_infoLabel->setText(infoText);
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_infoLabel->setStyleSheet("font-size: 9pt; color: #666666;");
    m_infoLabel->setWordWrap(true);

    Layouting::ColumnWithMargin{m_imageLabel, m_infoLabel}.attachTo(this);
}

void TemplateGridItem::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    m_resizeTimer->start(200);
}

void TemplateGridItem::updatePixmap() {
    if (m_imageData.empty() || !m_imageLabel) {
        return;
    }
    auto imageWidth = m_imageData.cols;
    auto imageHeight = m_imageData.rows;

    QSize labelSize = m_imageLabel->size();

    if (labelSize.width() <= 0 || labelSize.height() <= 0) {
        return;
    }

    if (imageWidth < labelSize.width() && imageHeight < labelSize.height()) {
        m_imageLabel->setPixmap(m_originalPixmap);
    } else {
        QPixmap scaledPixmap = m_originalPixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        m_imageLabel->setPixmap(scaledPixmap);
    }
}
