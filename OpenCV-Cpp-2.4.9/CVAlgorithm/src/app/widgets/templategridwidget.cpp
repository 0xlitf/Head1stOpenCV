#include "templategridwidget.h"
#include <QDebug>

TemplateGridWidget::TemplateGridWidget(QWidget *parent)
    : QScrollArea(parent)
    , m_containerWidget(new QWidget)
    , m_gridLayout(new QGridLayout(m_containerWidget)) {
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWidget(m_containerWidget);
    setWidgetResizable(true);
    m_containerWidget->setLayout(m_gridLayout);
    m_gridLayout->setSpacing(10);
    m_gridLayout->setAlignment(Qt::AlignTop);

    m_resizeTimer->setSingleShot(true);

    connect(m_resizeTimer, &QTimer::timeout, this, [=]() {
        // qDebug() << "TemplateGridWidget::updateChildren";
        this->updateChildren();
    });
}

void TemplateGridWidget::addImage(const QString &name, const cv::Mat &image, const MatchResult &result, bool isTemplate) {
    qDebug() << "TemplateGridWidget::addImage" << name ;

    QString matchedTemplateName = std::get<0>(result);                   // 名称
    std::vector<cv::Point> contour = std::get<1>(result); // 轮廓
    cv::Point2f center = std::get<2>(result);             // 中心点
    double score = std::get<3>(result);                   // 分数
    double areaDifferencePercent = std::get<4>(result);                   // 面积差值百分比

    // qDebug() << "\t名称:" << matchedTemplateName;
    // qDebug() << "\t中心坐标: (" << center.x << "," << center.y << ")";
    // qDebug() << "\t轮廓点数:" << contour.size();
    qDebug() << "\t误差分数:" << QString::number(score, 'f', 6);
    qDebug() << "\t面积差值百分比:" << areaDifferencePercent;  // 如果模板没有匹配到，面积差值百分比为-100

    if (m_imageNames.contains(name)) {
        qDebug() << "m_imageItems.contains(name)";
        // removeImage(name);

        // qDebug() << "m_imageItems.contains" << name;
        // TemplateGridItem *item = m_imageItems[name];
        // item->setImageMat(image, result);
        // return;
    }

    TemplateGridItem *newItem = new TemplateGridItem(name, image, result, m_containerWidget);
    newItem->setIsTemplate(isTemplate);

    if (idealItemWidth > 0 && idealItemHeight > 0) {
        newItem->setFixedWidth(idealItemWidth);
        newItem->setFixedHeight(idealItemHeight);
    }

    m_imageNames.append(name);
    m_imageItems.append(newItem);

    int totalItems = m_imageItems.size() - 1; // 新项插入前的总数，即新索引
    int row = totalItems / m_maxColumns;
    int column = totalItems % m_maxColumns;

    m_gridLayout->addWidget(newItem, row, column);
}

void TemplateGridWidget::removeImage(const QString &name) {
    if (m_imageNames.contains(name)) {
        int index = m_imageNames.indexOf(name);
        TemplateGridItem *itemToRemove = m_imageItems[index];
        m_gridLayout->removeWidget(itemToRemove);
        m_imageNames.removeAt(index);
        m_imageItems.removeAt(index);
        itemToRemove->deleteLater();
    }
}

void TemplateGridWidget::clearAllImages() {
    for (int i = 0; i < m_imageItems.size(); ++i) {
        m_gridLayout->removeWidget(m_imageItems[i]);
        m_imageItems[i]->deleteLater();
    }

    m_imageNames.clear();
    m_imageItems.clear();
}

void TemplateGridWidget::resizeEvent(QResizeEvent *event) {
    QScrollArea::resizeEvent(event);

    qDebug() << "TemplateGridWidget::resizeEvent";
    m_resizeTimer->start(100);
}

void TemplateGridWidget::updateChildren() {
    QSize viewportSize = viewport()->size();

    idealItemWidth = calculateIdealItemWidth(viewportSize.width());
    idealItemHeight = calculateIdealItemHeight(viewportSize.height());

    qDebug() << "idealItemWidth Height" << idealItemWidth << idealItemHeight;

    for (TemplateGridItem *item : m_imageItems) {
        item->setFixedWidth(idealItemWidth);
        item->setFixedHeight(idealItemHeight);
    }
}

int TemplateGridWidget::calculateIdealItemWidth(int parentWidth) {
    int spacing = 10;
    int totalSpacing = (m_maxColumns - 1) * spacing;

    int availableWidth = parentWidth - totalSpacing - 20;
    int idealWidth = availableWidth / m_maxColumns;

    idealWidth = std::max(120, idealWidth);
    return idealWidth;
}

int TemplateGridWidget::calculateIdealItemHeight(int parentHeight) {
    int availableHeight = parentHeight - 30;
    int idealHeight = availableHeight / 2;

    idealHeight = std::max(120, idealHeight);
    return idealHeight;
}

int TemplateGridWidget::maxColumns() const {
    return m_maxColumns;
}

void TemplateGridWidget::setMaxColumns(int newMaxColumns) {
    m_maxColumns = newMaxColumns;
}
