#ifndef QTSPDLOGINTEGRATION_H
#define QTSPDLOGINTEGRATION_H

#include "spdlog/spdlog.h"
#include <QObject>

#pragma execution_character_set("utf-8")

class QtSpdlogIntegration {
public:
    static void install();
    static void uninstall();

    // Qt 消息处理器
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static std::shared_ptr<spdlog::logger> s_logger;
};

#endif // QTSPDLOGINTEGRATION_H
