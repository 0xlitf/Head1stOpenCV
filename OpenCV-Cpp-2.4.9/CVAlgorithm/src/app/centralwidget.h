#pragma once

#pragma execution_character_set("utf-8")

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGridLayout>
#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QStackedWidget>
#include <QTableView>
#include <QTextEdit>
#include <QTextStream>
#include <QThread>
#include <QTime>
#include <QWidget>
#include "controls/widgetbase.h"
#include "controls/layoutbuilder.h"

class CentralWidget : public WidgetBase {
    Q_OBJECT
public:
    explicit CentralWidget(QWidget* parent = nullptr);

private:
    void setDefaultPageIndex(int i);
    void createTopButton(const QString &text);
    void addPage(const QString &text, QWidget *w);

    QStringList m_buttonStringList;
    void createComponents();

    QStackedWidget *m_stackedWidget = nullptr;
    QList<QPushButton *> m_buttonList;
    QHBoxLayout *m_topButtonRow = nullptr;
    QButtonGroup *m_buttonGroup = nullptr;
};
