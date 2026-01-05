#include "imagelistviewdelegate.h"

#include <QFileInfo>
#include <QPainter>

ImageListViewDelegate::ImageListViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void ImageListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // 0. 保存painter状态并设置抗锯齿
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 1. 从模型获取数据（假设模型存储了完整文件路径）
    QString filePath = index.data(Qt::UserRole).toString(); // 或 Qt::DisplayRole
    QFileInfo fileInfo(filePath);

    // 2. 绘制背景（处理选中状态）
    if (option.state & QStyle::State_Selected) {
        // 选中状态下的背景色
        painter->fillRect(option.rect, option.palette.highlight());
        // 可以同时设置选中状态下的文本颜色
        // painter->setPen(option.palette.highlightedText().color());
    } else {
        // 默认背景色，模拟你原来Widget的背景 (#e9e9e9)
        painter->fillRect(option.rect, QColor(233, 233, 233));
    }

    // 3. 定义布局参数（边距、间距、缩略图尺寸等）
    const int margin = 5;
    const int spacing = 8;
    const QSize thumbnailSize(80, 60);

    QRect rect = option.rect;                      // 当前项的总区域
    rect.adjust(margin, margin, -margin, -margin); // 应用边距

    // 4. 绘制缩略图区域
    QRect thumbRect = rect;
    thumbRect.setSize(thumbnailSize);

    // 绘制缩略图背景边框
    painter->setPen(QColor(0xcc, 0xcc, 0xcc));
    painter->setBrush(QColor(0xf0, 0xf0, 0xf0));
    painter->drawRect(thumbRect);

    // 生成并绘制缩略图
    QPixmap thumbnail = generateThumbnail(filePath, thumbnailSize);
    if (!thumbnail.isNull()) {
        // 计算居中显示的矩形
        QPixmap scaledThumb = thumbnail.scaled(thumbRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPoint centerPoint = thumbRect.center() - QPoint(scaledThumb.width() / 2, scaledThumb.height() / 2);
        painter->drawPixmap(centerPoint, scaledThumb);
    } else {
        // 绘制错误提示
        painter->drawText(thumbRect, Qt::AlignCenter, "无效图片");
    }

    // 5. 绘制文本区域（文件名和图片信息）
    QRect textRect = rect;
    textRect.setLeft(thumbRect.right() + spacing); // 文本区域从缩略图右侧开始
    textRect.setRight(rect.right());

    // 计算文本区域内的布局
    QRect fileNameRect = textRect;
    fileNameRect.setHeight(option.fontMetrics.height() * 1.2); // 文件名占1.5行高

    QRect infoRect = textRect;
    infoRect.setTop(fileNameRect.bottom());

    // 绘制文件名（加粗，稍大字号）
    QFont nameFont = option.font;
    nameFont.setPixelSize(14);
    nameFont.setBold(true);
    // nameFont.setPointSize(nameFont.pointSize() + 1); // 可选：增大字号
    painter->setFont(nameFont);
    painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : Qt::black);

    QString fileName = fileInfo.fileName();
    QString elidedFileName = option.fontMetrics.elidedText(fileName, Qt::ElideRight, fileNameRect.width());
    painter->drawText(fileNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedFileName);

    // 绘制图片信息（灰色，正常字号）
    QFont infoFont = option.font;
    infoFont.setPixelSize(12);
    painter->setFont(infoFont);
    painter->setPen(option.state & QStyle::State_Selected ? option.palette.highlightedText().color() : QColor(0x66, 0x66, 0x66));

    // 组装信息字符串（这部分逻辑与你原Widget中的updateDisplay()类似）
    QImage image(filePath);
    QString sizeInfo;
    if (!image.isNull()) {
        sizeInfo = QString("尺寸: %1 x %2 像素\n").arg(image.width()).arg(image.height());
    } else {
        sizeInfo = "尺寸: 未知\n";
    }

    qint64 fileSizeBytes = fileInfo.size();
    QString sizeStr;
    if (fileSizeBytes < 1024) {
        sizeStr = QString("%1 B").arg(fileSizeBytes);
    } else if (fileSizeBytes < 1024 * 1024) {
        sizeStr = QString("%1 KB").arg(fileSizeBytes / 1024.0, 0, 'f', 1);
    } else {
        sizeStr = QString("%1 MB").arg(fileSizeBytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
    QString fileSizeInfo = QString("大小: %1").arg(sizeStr);

    QString infoText = sizeInfo + fileSizeInfo;
    painter->drawText(infoRect, Qt::AlignLeft | Qt::AlignTop, infoText);

    // 6. 恢复painter状态
    painter->restore();
}

QSize ImageListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // 返回一个固定的项大小，例如：缩略图高度60 + 上下边距各5 =
    // 70，宽度可以设为-1（由视图决定）或一个固定值
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(-1, 70); // 高度固定为70像素
}

QPixmap ImageListViewDelegate::generateThumbnail(const QString &filePath, const QSize &size) const {
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        return QPixmap(); // 返回空Pixmap
    }
    return pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}
