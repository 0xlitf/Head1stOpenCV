#include "application.h"
#include "appmainwindow.h"
#include <QDebug>
#include <QFontDatabase>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QString>
#include <QTranslator>

#include "utils/messageinstaller.h"

namespace App {

Application::Application(int &argc, char *argv[])
    : SingleApplication(argc, argv, true, SingleApplication::SecondaryNotification | SingleApplication::User | SingleApplication::System | SingleApplication::ExcludeAppPath | SingleApplication::ExcludeAppVersion) {
    qDebug() << "QT_VERSION_STR:" << QT_VERSION_STR;
    qDebug() << "qVersion():" << qVersion();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)    // Qt6
#elif QT_VERSION >= QT_VERSION_CHECK(5, 15, 0) // Qt5.15+
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)  // Qt5+
#else
#endif
}

Application::~Application() {
    qDebug() << "\n\t~Application\n" << QString("#").repeated(100);
}

int Application::startApplication() {
    MessageInstaller::instance()->install();

    qDebug() << "\n" << QString("#").repeated(100) << "\n\tApplication::startApplication()";

    if (!isPrimary()) {
        sendMessage("Sansan");
        QMessageBox::critical(nullptr, tr("警告"), tr("已有一个程序实例在运行"));
        return -1;
    }

    QObject::connect(this, &SingleApplication::receivedMessage, this, [=](quint32, QByteArray) { qDebug() << "other instance started."; });

    QFontDatabase fontDB;
    QStringList fonts = fontDB.families();
    auto fontName = QString("微软雅黑");
    if (fonts.contains(fontName)) {
        QFont font(fontName, 11);
        this->setFont(font);
    } else {
        qDebug() << "微软雅黑字体未找到，使用系统默认字体";
    }

    AppMainWindow mainWindow;
    // mainWindow.show();
    mainWindow.showMaximized();

    m_userSettings = new QSettings(organizationName(), applicationName(), this);

    connect(this, &QCoreApplication::aboutToQuit, this, [=]() {
        qDebug() << "QCoreApplication::aboutToQuit";

        this->closeApplication();

        MessageInstaller::instance()->uninstall();
    });

    auto result = this->exec();
    qDebug() << "程序退出返回值: " << result;
    return result;
}

QVariant Application::getUserSetting(const QString &key, const QVariant &defaultValue) {
    return m_userSettings->value(key, defaultValue);
}

void Application::setUserSetting(const QString &key, QVariant data) {
    m_userSettings->setValue(key, data);
}

void Application::removeUserSetting(const QString &key) {
    m_userSettings->remove(key);
}

void Application::closeApplication() {
    qDebug() << "Application::closeApplication()";
}

bool Application::event(QEvent *e) {
    switch (e->type()) {
    case QEvent::Close:
        break;
    case QEvent::FocusIn:
        break;
    case QEvent::ApplicationActivate: {
#ifdef Q_OS_MAC
        emit this->applicationActivate();
#endif
    } break;
    default:
        break;
    }
    return SingleApplication::event(e);
}

} // namespace App
