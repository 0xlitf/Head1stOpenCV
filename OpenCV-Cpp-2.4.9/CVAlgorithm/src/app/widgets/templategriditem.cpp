#include "templategriditem.h"
#include "controls/layoutbuilder.h"
#include "imageutils.h"
#include <QDebug>

TemplateGridItem::TemplateGridItem(const QString &imageName, const cv::Mat &imageData, const MatchResult &result, QWidget *parent)
    : QWidget(parent) {
    m_imageName = imageName;
    m_imageData = imageData;
    m_matchedTemplateName = std::get<0>(result);
    m_contour = std::get<1>(result);
    m_center = std::get<2>(result);
    m_score = std::get<3>(result);
    m_areaDifferencePercent = std::get<4>(result);

    QString matchedTemplateName = std::get<0>(result);                   // 名称
    std::vector<cv::Point> contour = std::get<1>(result); // 轮廓
    cv::Point2f center = std::get<2>(result);             // 中心点
    double score = std::get<3>(result);                   // 分数
    double areaDifferencePercent = std::get<4>(result);                   // 面积差值百分比

    qDebug() << "\t名称m_matchedTemplateName:" << m_matchedTemplateName;
    qDebug() << "\t中心坐标m_center: (" << m_center.x << "," << m_center.y << ")";
    qDebug() << "\t轮廓点数m_contour:" << m_contour.size();
    qDebug() << "\t误差分数m_score:" << m_score;
    qDebug() << "\t面积差值百分比m_areaDifferencePercent:" << m_areaDifferencePercent;  // 如果模板没有匹配到，面积差值百分比为-100

    m_resizeTimer->setSingleShot(true);

    connect(m_resizeTimer, &QTimer::timeout, this, [=] {
        // qDebug() << "TemplateGridItem::updatePixmap";
        this->updatePixmap();
    });

    setupUI();
}

void TemplateGridItem::setImageMat(const cv::Mat &imageData, const MatchResult &result) {
    m_imageData = imageData.clone();
    m_originalPixmap = ImageUtils::cvMatToQPixmap(imageData);
    m_matchedTemplateName = std::get<0>(result);
    m_contour = std::get<1>(result);
    m_center = std::get<2>(result);
    m_score = std::get<3>(result);
    m_areaDifferencePercent = std::get<4>(result);

    this->updatePixmap();
    this->updateText();
}

void TemplateGridItem::setupUI() {
    if (!m_imageData.empty()) {
        m_originalPixmap = ImageUtils::cvMatToQPixmap(m_imageData);
    }

    // updatePixmap();

    m_imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("border: 1px solid #cccccc;");

    m_infoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_infoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_infoLabel->setStyleSheet("font-size: 9pt; color: #666666;");
    m_infoLabel->setWordWrap(true);
    this->updateText();

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

void TemplateGridItem::updateText() {
    QString infoText;
    if (!m_isTemplate) {
        infoText = QString("名称: %1\n尺寸: %2x%3\n通道: "
                           "%4\n匹配模板名:%5\n误差分数:%6\n面积偏差:%7")
                       .arg(m_imageName)
                       .arg(m_imageData.cols)
                       .arg(m_imageData.rows)
                       .arg(m_imageData.channels())
                       .arg(m_matchedTemplateName)
                       .arg(QString::number(m_score, 'f', 6))
                       .arg(QString::number(m_areaDifferencePercent, 'f', 6));
    } else {
        infoText = QString("名称: %1\n尺寸: %2x%3\n通道: %4")
                       .arg(m_imageName)
                       .arg(m_imageData.cols)
                       .arg(m_imageData.rows)
                       .arg(m_imageData.channels());
    }
    m_infoLabel->setText(infoText);
}

double TemplateGridItem::getAreaDifferencePercent() const {
    return m_areaDifferencePercent;
}

void TemplateGridItem::setAreaDifferencePercent(double newAreaDifferencePercent) {
    m_areaDifferencePercent = newAreaDifferencePercent;

    this->updateText();
}

double TemplateGridItem::getScore() const {
    return m_score;
}

void TemplateGridItem::setScore(double newScore) {
    m_score = newScore;

    this->updateText();
}

cv::Point2f TemplateGridItem::getCenter() const {
    return m_center;
}

void TemplateGridItem::setCenter(cv::Point2f newCenter) {
    m_center = newCenter;

    this->updateText();
}

std::vector<cv::Point> TemplateGridItem::getContour() const {
    return m_contour;
}

void TemplateGridItem::setContour(const std::vector<cv::Point> &newContour) {
    m_contour = newContour;

    this->updateText();
}

QString TemplateGridItem::getMatchedTemplateName() const {
    return m_matchedTemplateName;
}

void TemplateGridItem::setMatchedTemplateName(const QString &newMatchedTemplateName) {
    m_matchedTemplateName = newMatchedTemplateName;

    this->updateText();
}

bool TemplateGridItem::isTemplate() const {
    return m_isTemplate;
}

void TemplateGridItem::setIsTemplate(bool newIsTemplate) {
    m_isTemplate = newIsTemplate;

    this->updateText();
}
