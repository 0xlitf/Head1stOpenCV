#ifndef IMAGELISTVIEWFILTERMODEL_H
#define IMAGELISTVIEWFILTERMODEL_H

#include <QSortFilterProxyModel>

class ImageListViewFilterModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit ImageListViewFilterModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif // IMAGELISTVIEWFILTERMODEL_H
