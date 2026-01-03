#ifndef IMAGELISTVIEWDELEGATE_H
#define IMAGELISTVIEWDELEGATE_H

#pragma execution_character_set("utf-8")

#include <QStyledItemDelegate>

class ImageListViewDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    ImageListViewDelegate(QObject *parent = 0);

    // 重写绘制方法
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    // 返回项的大小
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    // 生成缩略图的辅助函数
    QPixmap generateThumbnail(const QString &filePath, const QSize &size) const;
};

#endif // IMAGELISTVIEWDELEGATE_H
