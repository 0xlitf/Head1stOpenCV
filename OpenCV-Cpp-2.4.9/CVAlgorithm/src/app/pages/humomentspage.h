#ifndef HUMOMENTSPAGE_H
#define HUMOMENTSPAGE_H

#pragma execution_character_set("utf-8")

#include "controls/controls.h"
#include "humomentsmatcher.h"

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

    class QDoubleSpinBox *m_scoreThresholdSpinBox = nullptr;
    class QSpinBox *m_whiteThresholdSpinBox = nullptr;
    class QDoubleSpinBox *m_areaThresholdSpinBox = nullptr;

    class QSlider *m_scoreThresholdSlider = nullptr;
    class QSlider *m_whiteThresholdSlider = nullptr;
    class QSlider *m_areaThresholdSlider = nullptr;

    class SelectFileWidget *m_selectFileWidget = nullptr;
    class SelectFolderWidget *m_selectFolderWidget = nullptr;
    class SelectFolderWidget *m_selectTemplateFolderWidget = nullptr;

    class QSettings *m_settings = nullptr;

    QString m_currentProcessImageFile{};
    QString m_configPath{};

    HuMomentsMatcher matcher;

};

#endif // HUMOMENTSPAGE_H
