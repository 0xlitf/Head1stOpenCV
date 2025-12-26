QT       -= gui

TARGET = resources
TEMPLATE = lib
CONFIG += staticlib resources_big

DEFINES += QT_MESSAGELOGCONTEXT

DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        resources.cpp

HEADERS += \
        resources.h
        
unix {
    target.path = /usr/lib
    INSTALLS += target
}

include(../install.pri)

RESOURCES += \
    resources.qrc

QML_IMPORT_PATH += $$PWD
