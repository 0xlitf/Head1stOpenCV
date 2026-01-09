#include "imagelistviewmodel.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>

ImageListViewModel::ImageListViewModel(QObject *parent)
    : QAbstractListModel{parent} {}

int ImageListViewModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_imageFiles.size();
}

QVariant ImageListViewModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_imageFiles.size())
        return QVariant();

    QFileInfo fileInfo(m_imageFiles.at(index.row()));
    QString folderName = fileInfo.dir().dirName();

    switch (role) {
    case Qt::DisplayRole: {  // 返回用于显示的文件名, 控制搜索, 在delegate中控制显示
        return QString("%1/%2").arg(folderName).arg(fileInfo.fileName());
        // return fileInfo.fileName();
    }
    case Qt::UserRole: { // 返回完整文件路径
        return fileInfo.absoluteFilePath();
    }
    default: {
        return QVariant();
    }
    }
}

void ImageListViewModel::setImageFiles(const QStringList &fileList) {
    beginResetModel(); // 通知视图模型数据将发生重大变化
    m_imageFiles = fileList;
    endResetModel(); // 通知视图可以更新了
}

QString ImageListViewModel::filePath(const QModelIndex &index) const {
    if (index.isValid() && index.row() < m_imageFiles.size())
        return m_imageFiles.at(index.row());
    return QString();
}
