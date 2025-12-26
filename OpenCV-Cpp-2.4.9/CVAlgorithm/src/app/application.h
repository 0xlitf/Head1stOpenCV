#pragma once

#pragma execution_character_set("utf-8")

#include "SingleApplication/singleapplication.h"

#include <functional>

class QSettings;

typedef std::function<void()> Functional;

namespace App {
    class Application : public SingleApplication {
		Q_OBJECT
		
	public:
		Q_PROPERTY(bool debugMode READ debugMode CONSTANT)
        quint64 debugMode() {
#ifndef QT_DEBUG
            return false;
#endif
            return true;
        }
		
    signals:
        void applicationActivate();
	public:
        explicit Application(int& argc, char *argv[]);
		~Application();

		static Application *application() {
			return qobject_cast<Application*>(QApplication::instance());
		}

		int startApplication();

        QVariant getUserSetting(const QString& key, const QVariant &defaultValue = QVariant());

        void setUserSetting(const QString&key,QVariant data);

        void removeUserSetting(const QString& key);

    protected:
		bool event(QEvent *e) override;
		
        Q_INVOKABLE void _runOnMainThread(Functional func) {
            func();
        }

	public slots:

		void closeApplication();

    private:
        QSettings* m_userSettings = nullptr;
	};
}
