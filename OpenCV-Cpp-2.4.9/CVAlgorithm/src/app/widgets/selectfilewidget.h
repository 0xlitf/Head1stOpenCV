#ifndef SELECTFILEWIDGET_H
#define SELECTFILEWIDGET_H

#include "controls/controls.h"

class SelectFileWidget: public WidgetBase {
    Q_OBJECT
signals:
    void fileChanged(const QString& folderPath);

public:
    explicit SelectFileWidget(QWidget *parent = nullptr);

private:
    void createComponents();
};

#endif // SELECTFILEWIDGET_H
