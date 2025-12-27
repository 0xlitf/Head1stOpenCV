#ifndef CUTOUTOBJECTPAGE_H
#define CUTOUTOBJECTPAGE_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"
#include "cutoutobject.h"

class CutoutObjectPage : public WidgetBase {
    Q_OBJECT
public:
    explicit CutoutObjectPage(QWidget *parent = nullptr);

    void createComponents();
signals:
};

#endif // CUTOUTOBJECTPAGE_H
