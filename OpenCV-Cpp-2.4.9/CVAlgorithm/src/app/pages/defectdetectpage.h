#ifndef DEFECTDETECTPAGE_H
#define DEFECTDETECTPAGE_H

#pragma execution_character_set("utf-8")

#include <controls/controls.h>
#include "contourextractor.h"

class DefectDetectPage : public WidgetBase {
    Q_OBJECT
signals:
    void paramChanged();

public:
    DefectDetectPage();

    void createComponents();
    void createConnections();

    void runDefectDetectAlgo(const QString &filePath);

private:
    void loadConfig();
    void saveConfig();

private:
    class ImageGridWidget *m_imageGridWidget = nullptr;
    class TemplateGridWidget *m_templateGridWidget = nullptr;

    class SelectFileWidget *m_selectFileWidget = nullptr;
    class SelectFolderWidget *m_selectFolderWidget = nullptr;
    // class SelectFolderWidget *m_selectTemplateFolderWidget = nullptr;

    TextEdit* m_resultText = nullptr;

    class QSettings *m_settings = nullptr;

    QString m_currentProcessImageFile{};
    QString m_configPath{};

};

#endif // DEFECTDETECTPAGE_H
