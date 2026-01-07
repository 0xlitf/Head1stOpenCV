#ifndef HUMOMENTSPAGE_H
#define HUMOMENTSPAGE_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"

class HuMomentsPage : public WidgetBase {
    Q_OBJECT
signals:
    void paramChanged();

public:
    explicit HuMomentsPage(QWidget *parent = nullptr);
    ~HuMomentsPage();

    void createComponents();
    void createConnections();

    void ruHuMomentsMatch(const QString &filePath);

private:
    void loadConfig();
    void saveConfig();

private:
    class ImageGridWidget *m_imageGridWidget = nullptr;
    class TemplateGridWidget *m_templateGridWidget = nullptr;

    class QSpinBox *m_areaMaxSpinBox = nullptr;
    class QSpinBox *m_areaMinSpinBox = nullptr;

    class QSlider *m_areaMaxSlider = nullptr;
    class QSlider *m_areaMinSlider = nullptr;

    class SelectFileWidget *m_selectFileWidget = nullptr;
    class SelectFolderWidget *m_selectFolderWidget = nullptr;
    class SelectFolderWidget *m_selectTemplateFolderWidget = nullptr;

    class QSettings *m_settings = nullptr;

    QString m_currentProcessImageFile{};
    QString m_configPath{};
};

#endif // HUMOMENTSPAGE_H
