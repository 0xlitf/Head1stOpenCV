#ifndef MESSAGEINSTALLER_H
#define MESSAGEINSTALLER_H

#include <QObject>
#include <QMutex>
#include <QString>

class MessageInstaller : public QObject
{
    Q_OBJECT
public:
    void install();
    void uninstall();

    static MessageInstaller* instance();

    void logEnv();

signals:
    void messageLogged(const QString& message, QtMsgType type, qint64 threadId);

private:
    explicit MessageInstaller(QObject *parent = nullptr);
    ~MessageInstaller();

    static MessageInstaller* m_instance;
    static QMutex m_mutex;

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // MESSAGEINSTALLER_H
