#-------------------------------------------------
#
# Project created by QtCreator 2016-08-22T11:09:46
#
#-------------------------------------------------

QT       += core xml

QT       -= gui

INCLUDEPATH += "$$(HALCONROOT)/include" \
    "$$(HALCONROOT)/include/cpp" \
    "C:\opencv\build\include" \
    "C:\opencv\build\include\opencv" \
    "C:\opencv\build\include\opencv2"

TARGET = counter
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    partscounter.cpp \
    partstrainer.cpp

LIBS += -L"$$(HALCONROOT)/lib/$$(HALCONARCH)" \
    -lhalcon \
    -lhalconcpp


LIBS += -L"C:\opencv\build\x64\vc12\lib" \
    -lopencv_video249 \
    -lopencv_core249 \
    -lopencv_highgui249 \
    -lopencv_imgproc249

HEADERS += \
    partscounter.h \
    partstrainer.h
