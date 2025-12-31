#ifndef CUTOUTOBJECTPAGE_H
#define CUTOUTOBJECTPAGE_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"
#include "cutoutobject.h"

#include <QSpinBox>

class CutoutObjectPage : public WidgetBase {
    Q_OBJECT
public:
    explicit CutoutObjectPage(QWidget *parent = nullptr);

    void createComponents();
    void createConnections();

    void runCutoutAlgo(const QString &filePath);
signals:
    void paramChanged();

private:
    class ImageGridWidget *m_imageGridWidget = nullptr;

    QSpinBox *colorSpinBox = nullptr;
    QSpinBox *blueSpinBox = nullptr;
};

#endif // CUTOUTOBJECTPAGE_H
