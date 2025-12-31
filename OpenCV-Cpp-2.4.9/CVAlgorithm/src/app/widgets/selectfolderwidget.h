#ifndef SELECTFOLDERWIDGET_H
#define SELECTFOLDERWIDGET_H

#include "controls/controls.h"

class SelectFolderWidget: public WidgetBase {
    Q_OBJECT
signals:
    void folderChanged(const QString& folderPath);

public:
    explicit SelectFolderWidget(QWidget *parent = nullptr);
    QString getSelectFolder() {
        return m_textEdit->toPlainText();
    }

private:
    void createComponents();
    TextEdit* m_textEdit = nullptr;
};

#endif // SELECTFOLDERWIDGET_H
