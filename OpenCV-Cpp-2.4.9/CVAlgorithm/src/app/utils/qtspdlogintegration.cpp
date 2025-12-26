#include "qtspdlogintegration.h"
#include <QDateTime>
#include <QDir>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_sinks.h"

std::shared_ptr<spdlog::logger> QtSpdlogIntegration::s_logger = nullptr;

void QtSpdlogIntegration::install() {
    try {
        QDir().mkpath("logs");

        auto logFileName = QString("app_%1.log").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
        s_logger = spdlog::basic_logger_mt("qt_app", logFileName.toStdString());

        s_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
        s_logger->set_level(spdlog::level::debug);

        qInstallMessageHandler(qtMessageHandler);

        s_logger->info("Qt spdlog 集成初始化成功");
        QtConcurrent::run([](){
            qDebug() << "QtConcurrent::run test thread id";
        });

    } catch (const spdlog::spdlog_ex& ex) {
        s_logger = spdlog::stdout_logger_mt("console");
        qInstallMessageHandler(qtMessageHandler);
        s_logger->warn("使用控制台日志作为回退: {}", ex.what());
    }
}

void QtSpdlogIntegration::uninstall() {
    if (s_logger) {
        s_logger->info("QtSpdlogIntegration::shutdown");
        spdlog::drop_all();
    }
}

void QtSpdlogIntegration::qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (!s_logger) return;

    auto message = QString("[File:%1:Line %2][Thread %3]: %5")
                          .arg(QString::fromLocal8Bit(context.file))
                          .arg(QString::number(context.line))
                          .arg(reinterpret_cast<qint64>(QThread::currentThreadId()))
                          .arg(msg).toStdString();

    switch (type) {
    case QtDebugMsg:
        s_logger->debug("{}", message);
        break;
    case QtInfoMsg:
        s_logger->info("{}", message);
        break;
    case QtWarningMsg:
        s_logger->warn("{}", message);
        break;
    case QtCriticalMsg:
        s_logger->error("{}", message);
        break;
    case QtFatalMsg:
        s_logger->critical("{}", message);
        break;
    }

    if (type >= QtCriticalMsg) {
        s_logger->flush();
    }
}
