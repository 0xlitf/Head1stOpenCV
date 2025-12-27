#include "imagelistwidget.h"

ImageListWidget::ImageListWidget(QWidget *parent)
    : QWidget(parent), m_pathEdit(new QLineEdit), m_browseButton(new QPushButton("浏览文件夹")), m_listWidget(new QListWidget)
{
    this->setMinimumWidth(300);
    this->setMaximumWidth(500);

    // 设置窗口标题和大小
    setWindowTitle("图片列表浏览器");
    resize(600, 500);

    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    // 图片列表控件
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection); // 单行选择
    mainLayout->addWidget(m_listWidget);
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

        // 将自定义控件设置为列表项
        m_listWidget->setItemWidget(listItem, itemWidget);
    }
}
