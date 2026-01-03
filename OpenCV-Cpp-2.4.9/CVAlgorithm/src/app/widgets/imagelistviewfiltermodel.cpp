#include "imagelistviewfiltermodel.h"

ImageListViewFilterModel::ImageListViewFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent) {}

bool ImageListViewFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    if (filterRegExp().isEmpty()) {
        return true; // 搜索框为空时显示所有行
    }

    // 获取源模型中该行的数据（假设文件名存储在DisplayRole）
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    QString fileName = sourceModel()->data(index, Qt::DisplayRole).toString();

    // 检查文件名是否包含搜索文本（不区分大小写）
    return fileName.contains(filterRegExp());
}
