#ifndef SELECTFILEWIDGET_H
#define SELECTFILEWIDGET_H

#include "controls/controls.h"

class SelectFileWidget : public WidgetBase {
    Q_OBJECT
signals:
    void fileChanged(const QString &folderPath);

public:
    explicit SelectFileWidget(QWidget *parent = nullptr);
    QString getSelectFile() { return m_textEdit->toPlainText(); }
    void setSelectFile(const QString &path) { m_textEdit->setText(path); }

private:
    void createComponents();
    TextEdit *m_textEdit = nullptr;
};

#endif // SELECTFILEWIDGET_H
