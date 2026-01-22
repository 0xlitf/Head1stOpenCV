
QT       += core gui axcontainer xml network concurrent serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 6): QT += core5compat

CONFIG += c++11

DEFINES += CVA_LIBRARY
DEFINES += QT_MESSAGELOGCONTEXT
DEFINES += PROJECT_DIR=\\\"$$PWD\\\"

include($$PWD/../install.pri)
include($$PWD/compile.pri)

INCLUDEPATH += $$absolute_path($$PWD/../share)

LIB_OPENCV_PATH = C:/opencv-2.4.9/build
INCLUDEPATH += $$LIB_OPENCV_PATH/include/

INCLUDEPATH += $$PWD/../CVAlgorithm/src/cva/

CONFIG(debug, debug|release) {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249d \
                                             -lopencv_highgui249d \
                                             -lopencv_imgproc249d
    QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roid.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249d.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249d.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249d.dll\" \"$${PROJECT_LIB_DIR}\"
} else {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249 \
                                             -lopencv_highgui249 \
                                             -lopencv_imgproc249
    QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roi.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249.dll\" \"$${PROJECT_LIB_DIR}\"
}

RC_ICONS = $$PWD/opencv.ico

RESOURCES += \

HEADERS += \
    $$PWD/../share/layoutbuilder.h \
    $$PWD/../share/messageinstaller.h \
    $$PWD/bgr2hsvconverter.h \
    $$PWD/defectdetector.h \
    $$PWD/minimumbounding.h \
    contourdefectdetector.h \
    contourextractor.h \
    mainwindow.h

SOURCES += \
    $$PWD/../share/layoutbuilder.cpp \
    $$PWD/../share/messageinstaller.cpp \
    $$PWD/bgr2hsvconverter.cpp \
    $$PWD/defectdetector.cpp \
    $$PWD/main.cpp \
    $$PWD/minimumbounding.cpp \
    contourdefectdetector.cpp \
    contourextractor.cpp \
    mainwindow.cpp

CODECFORTR = UTF-8

msvc {
    QMAKE_CXXFLAGS += /MP
    QMAKE_CFLAGS += /MP
}
