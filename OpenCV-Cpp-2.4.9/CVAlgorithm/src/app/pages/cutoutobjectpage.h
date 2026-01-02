#ifndef CUTOUTOBJECTPAGE_H
#define CUTOUTOBJECTPAGE_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"
#include "cutoutobject.h"

class CutoutObjectPage : public WidgetBase {
    Q_OBJECT
public:
    explicit CutoutObjectPage(QWidget *parent = nullptr);
    ~CutoutObjectPage();

    void createComponents();
    void createConnections();

    void runCutoutAlgo(const QString &filePath);
signals:
    void paramChanged();

private:
    void loadConfig();
    void saveConfig();

    class ImageGridWidget *m_imageGridWidget = nullptr;

    QString m_currentProcessImageFile{};

    class QSpinBox *colorSpinBox = nullptr;
    class QSpinBox *blueSpinBox = nullptr;

    class SelectFileWidget *m_selectFileWidget = nullptr;
    class SelectFolderWidget *m_selectFolderWidget = nullptr;

    class QSettings *m_settings;
    QString m_configPath;
};

#endif // CUTOUTOBJECTPAGE_H
