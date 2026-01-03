#ifndef IMAGELISTVIEWMODEL_H
#define IMAGELISTVIEWMODEL_H

#pragma execution_character_set("utf-8")

#include <QAbstractListModel>

class ImageListViewModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit ImageListViewModel(QObject *parent = nullptr);

    // 必须重写的基类函数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // 更新模型数据
    void setImageFiles(const QStringList &fileList);
    // 根据模型索引获取完整文件路径
    QString filePath(const QModelIndex &index) const;

private:
    QStringList m_imageFiles; // 存储图片路径列表
};

#endif // IMAGELISTVIEWMODEL_H
