#include "messageinstaller.h"
#include "qtspdlogintegration.h"
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include <QTextStream>
#include <QDir>
#include <QApplication>
#include <QProcessEnvironment>

MessageInstaller* MessageInstaller::m_instance = nullptr;
QMutex MessageInstaller::m_mutex;
static QFile file{};
static QString currentLogFileName{};
static bool needToCheckSize = true;

MessageInstaller::MessageInstaller(QObject *parent) : QObject(parent)
{
    // this->logEnv();
}

MessageInstaller::~MessageInstaller()
{
    qInstallMessageHandler(nullptr);
    m_instance = nullptr;
}

MessageInstaller* MessageInstaller::instance()
{
    if (!m_instance) {
        m_instance = new MessageInstaller();
    }
    return m_instance;
}

void MessageInstaller::logEnv()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList envKeys = env.keys();

    envKeys.sort();

    for (const QString &key : envKeys) {
        qDebug() << key << "=" << env.value(key);
    }
}

void MessageInstaller::install()
{
    QString output;
    QTextStream ts(&output);
    if (!qEnvironmentVariableIsEmpty("VisualStudioEdition")) { // start direct in vs
        qApp->setProperty("RunEvn", "vs");
		qSetMessagePattern("%{file}(%{line}): "
			"[%{type}]:"
			" [%{function}]:\n"
			" [%{time yyyy-MM-dd h:mm:ss.zzz t}]"
			" %{message}");
		ts << "run in vs: " << qgetenv("VisualStudioEdition");
	}
    else if (auto debugInQtCreator6 = !qEnvironmentVariableIsEmpty("_QTC_Path")) { // start direct in QtCreator 6
        qApp->setProperty("RunEvn", "qc6");
        qSetMessagePattern("\033[1;37m [file://%{file}:%{line}]:\033[0m"
                           "[\033[1;35m%{threadid}\033[0m]"
                           "%{if-debug}\033[1;36m%{endif}"
                           "%{if-info}\033[1;44m%{endif}"
                           "%{if-warning}\033[1;43m%{endif}"
                           "%{if-critical}\033[1;41m%{endif}"
                           "%{if-fatal}\033[1;41m%{endif}[%{type}]:\033[0m"
                           "\033[1;35m [%{function}]:\033[0m\n"
                           "\033[0;37m [%{time yyyy-MM-dd h:mm:ss.zzz t}]\033[0m"
                           "\033[1;91m %{message}\033[0m");
        ts << "run in QtCreator6: " << debugInQtCreator6 << ", "  << qgetenv("_QTC_Path");
    }
    else if (auto debugInQtCreator5 = !qEnvironmentVariableIsEmpty("VISUALSTUDIOVERSION")) { // start direct in QtCreator 5
        qApp->setProperty("RunEvn", "qc5");
        qSetMessagePattern("\033[1;37m [file://%{file}:%{line}]:\033[0m"
                           "[\033[1;35m%{threadid}\033[0m]"
                           "%{if-debug}\033[1;36m%{endif}"
                           "%{if-info}\033[1;44m%{endif}"
                           "%{if-warning}\033[1;43m%{endif}"
                           "%{if-critical}\033[1;41m%{endif}"
                           "%{if-fatal}\033[1;41m%{endif}[%{type}]:\033[0m"
                           "\033[1;35m [%{function}]:\033[0m\n"
                           "\033[0;37m [%{time yyyy-MM-dd h:mm:ss.zzz t}]\033[0m"
                           "\033[1;91m %{message}\033[0m");
        ts << "run in QtCreator5: " << debugInQtCreator5 << ", " << qgetenv("VISUALSTUDIOVERSION");
    } else if (!qEnvironmentVariableIsEmpty("VSCODE_PID")) { // start direct in VSCode
        qApp->setProperty("RunEvn", "vsc");
        qSetMessagePattern("\033[1;37m [%{file}:%{line}]:\033[0m"
                           "[\033[1;35m%{threadid}\033[0m]"
                           "%{if-debug}\033[1;36m%{endif}"
                           "%{if-info}\033[1;44m%{endif}"
                           "%{if-warning}\033[1;43m%{endif}"
                           "%{if-critical}\033[1;41m%{endif}"
                           "%{if-fatal}\033[1;41m%{endif}[%{type}]:\033[0m"
                           "\033[1;35m [%{function}]:\033[0m\n"
                           "\033[0;37m [%{time yyyy-MM-dd h:mm:ss.zzz t}]\033[0m"
                           "\033[1;91m %{message}\033[0m");
        ts << "run in VSCode: " << qgetenv("VSCODE_PID");
    } else {
        qApp->setProperty("RunEvn", "exe");
        // qInstallMessageHandler(MessageInstaller::messageHandler); // run by .exe

        QtSpdlogIntegration::install();

        ts << "run by .exe, write log into file";
    }

    QString architecture;
    if (QSysInfo::WordSize == 32) {
        architecture = "win32";
    } else if (QSysInfo::WordSize == 64) {
        architecture = "x64";
    } else {
        architecture = "Unknown";
    }

    QString buildType;
#ifdef _DEBUG
    buildType = "Debug";
#else
    buildType = "Release";
#endif

    qInfo().nospace().noquote()
            << "\n"
            << "\t" << output << "\n"
            << "\tarchitecture: " << architecture << "\n"
            << "\tbuildType: " << buildType << "\n"
            << "\tVisualStudioEdition exist: " << qEnvironmentVariableIsSet("VisualStudioEdition") << ", " << qgetenv("VisualStudioEdition") << "\n"
            << "\tQTDIR exist: " << qEnvironmentVariableIsSet("QTDIR") << ", " <<
               []() {
        if (qEnvironmentVariableIsSet("QTDIR")) {
            return qgetenv("QTDIR");
        }
        return QByteArray();
    }()
    << "\n"
    << "\t_QTC_Path exist: " << qEnvironmentVariableIsSet("_QTC_Path") << qgetenv("_QTC_Path") << "\n"
    << "\tVSCODE_PID exist: " << qEnvironmentVariableIsSet("VSCODE_PID");
}

void MessageInstaller::uninstall()
{
    QtSpdlogIntegration::uninstall();
}

void MessageInstaller::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QMutexLocker locker(&m_mutex);
    if (!m_instance) return;

    auto openFile = []() {
        QDir logDir{"."};

        currentLogFileName = QString("app_%1.log").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
        file.setFileName(logDir.filePath(currentLogFileName));
        file.open(QIODevice::ReadWrite | QIODevice::Append);
    };

    if (!file.isOpen()) {
        openFile();
        // ts.setDevice(&file);
        // ts.setCodec("UTF-8");
        // ts << "\r\n" << "\r\n";
        // /*QControls::*/file.flush();
    }

    if (needToCheckSize && file.size() > 104857600) { // 100MB = 100 * 1024 * 1024 = 104857600字节
        file.close();
        openFile();
        needToCheckSize = false;
    }

    QString text;

    switch (type) {
    case QtDebugMsg: {
        text = QString("Debug");
        break;
    }
    case QtWarningMsg: {
        text = QString("Warning");
        break;
    }
    case QtCriticalMsg: {
        text = QString("Critical");
        break;
    }
    case QtFatalMsg: {
        text = QString("Fatal");
        break;
    }
    case QtInfoMsg:; {
            text = QString("Info");
            break;
        }
    }

    QString message = QString("[File:%1:Line %2]: [Thread %3] [%4] [%5] %6")
                          .arg(QString::fromLocal8Bit(context.file))
                          .arg(QString::number(context.line))
                          .arg(reinterpret_cast<qint64>(QThread::currentThreadId()))
                          .arg(text)
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                          .arg(msg);

    QTextStream ts(&file);
    // ts.setCodec("UTF-8");
    ts << message << "\r\n";
    file.flush();
    // file.close();

    emit m_instance->messageLogged(message, type, reinterpret_cast<qint64>(QThread::currentThreadId()));
}
