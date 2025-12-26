#include "appmainwindow.h"
#include <QStackedLayout>
#include <QString>
#include <QHostInfo>
#include "compile.h"

AppMainWindow::AppMainWindow(QWidget *parent)
    : QMainWindow{parent} {
    this->setMinimumSize(1280, 900);
    this->setContentsMargins(0, 0, 0, 0);

    this->createComponents();
    this->setCentralWidget(m_central);
}

void AppMainWindow::createComponents() {
    QApplication::setOrganizationName("Sansan");
    QApplication::setOrganizationDomain("www.33tech.cn");

    QString buildType;
#ifdef QT_DEBUG
    buildType = QString("Debug");
#else
    buildType = QString("Release");
#endif
    QString appName(tr("Qt5.5.0 App"));
    QString fullAppName;
    fullAppName = appName + QString(" (%1 %2 %3 %4) (%5)").arg(buildType).arg(GIT_BRANCH).arg(GIT_COMMIT_HASH).arg(BUILD_DATE).arg(QHostInfo::localHostName());
    QApplication::setApplicationName(fullAppName);
}
