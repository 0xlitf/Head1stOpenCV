#include "imagelistwidget.h"
#include <QDebug>

ImageListWidget::ImageListWidget(QWidget *parent)
    : QWidget(parent), m_pathEdit(new QLineEdit), m_browseButton(new QPushButton("浏览文件夹")), m_listWidget(new ListWidget)
{
    m_listWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    this->setMinimumWidth(300);
    this->setMaximumWidth(300);


    // 设置窗口标题和大小
    setWindowTitle("图片列表浏览器");
    // resize(600, 500);

    // 图片列表控件
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection); // 单行选择
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    Layouting::Column{m_listWidget}.attachTo(this);

    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, [=](){
        qDebug() << "itemSelectionChanged:";
        this->onListItemSelectionChanged();
    });
}

void ImageListWidget::loadImagesFromFolder(const QString& folderPath)
{
    // 清空当前列表
    m_listWidget->clear();

    QDir dir(folderPath);
    if (!dir.exists()) {
        QMessageBox::warning(this, "警告", "指定的文件夹不存在！");
        return;
    }

    // 获取支持的图片格式 [1](@ref)
    QStringList nameFilters;
    for (const QByteArray& format : QImageReader::supportedImageFormats()) {
        nameFilters << QString("*.") + QString(format).toLower();
    }

    // 获取文件夹中所有支持的图片文件 [11](@ref)
    QFileInfoList imageFiles = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

    if (imageFiles.isEmpty()) {
        QMessageBox::information(this, "提示", "该文件夹下未找到支持的图片文件。");
        return;
    }

    // 为每个图片文件创建列表项 [6,7](@ref)
    for (const QFileInfo& fileInfo : imageFiles) {
        // 创建自定义的列表项控件
        ImageListItem* itemWidget = new ImageListItem(fileInfo);

        // 创建QListWidgetItem并设置大小
        QListWidgetItem* listItem = new QListWidgetItem(m_listWidget);
        listItem->setSizeHint(itemWidget->sizeHint()); // 关键：设置项的大小以适应自定义控件

        listItem->setData(Qt::UserRole, QVariant(fileInfo.absoluteFilePath()));

        // 将自定义控件设置为列表项
        m_listWidget->setItemWidget(listItem, itemWidget);
    }
}

void ImageListWidget::onListItemSelectionChanged()
{
    // 获取当前选中的项
    QListWidgetItem* currentItem = m_listWidget->currentItem();
    qDebug() << "onListItemSelectionChanged currentItem:" << currentItem;
    if (currentItem) {
        // 从项的数据中获取完整路径[8](@ref)
        QString imagePath = currentItem->data(Qt::UserRole).toString();
        qDebug() << "onListItemSelectionChanged imagePath:" << imagePath;
        // 或者，如果使用路径映射：
        // QString imagePath = m_itemToPathMap.value(currentItem);

        if (!imagePath.isEmpty()) {
            // 发射自定义信号[1,2](@ref)
            emit imageSelected(imagePath);
        }
    }
}
