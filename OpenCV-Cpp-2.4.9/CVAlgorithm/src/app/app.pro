
QT       += core gui axcontainer xml network concurrent serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 6): QT += core5compat

CONFIG += c++11

DEFINES += QT_MESSAGELOGCONTEXT

TEMP_DIR = $$PWD/temp

include(../install.pri)
include(app.pri)
include(compile.pri)
include($$PWD/SingleApplication/singleapplication.pri)

INCLUDEPATH += $$PWD/spdlog-1.2.1/include

LIB_OPENCV_PATH = $$LIB_SHARE_PATH/opencv-2.4.9/build
INCLUDEPATH += $$LIB_OPENCV_PATH/include/

LIB_HALCON_PATH = $$LIB_SHARE_PATH/MVTec-24.05.0.0/HALCON-24.05-Progress
INCLUDEPATH += $$LIB_HALCON_PATH/include/

CONFIG(debug, debug|release) {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249d \
                                             -lopencv_highgui249d \
                                             -lopencv_imgproc249d
    QMAKE_POST_LINK +=   cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249d.dll\" \"$${PROJECT_BIN_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249d.dll\" \"$${PROJECT_BIN_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249d.dll\" \"$${PROJECT_BIN_DIR}\"
} else {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249 \
                                             -lopencv_highgui249 \
                                             -lopencv_imgproc249
    QMAKE_POST_LINK +=   cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249.dll\" \"$${PROJECT_BIN_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249.dll\" \"$${PROJECT_BIN_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249.dll\" \"$${PROJECT_BIN_DIR}\"

}

LIBS += -L$$LIB_HALCON_PATH/lib/x64-win64 -lhalcon -lhalconcpp -lhdevenginecpp

COPY_DLL_FROM = $$LIB_HALCON_PATH/bin/x64-win64
COPY_DLL_FROM = $$replace(COPY_DLL_FROM, /, \\)
COPY_DLL_TO = $$replace(PROJECT_BIN_DIR, /, \\)

QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$COPY_DLL_FROM/halcon.dll\" \"$${PROJECT_BIN_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$COPY_DLL_FROM/halconc.dll\" \"$${PROJECT_BIN_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$COPY_DLL_FROM/halconcpp.dll\" \"$${PROJECT_BIN_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$COPY_DLL_FROM/halconxl.dll\" \"$${PROJECT_BIN_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$COPY_DLL_FROM/hdevenginecpp.dll\" \"$${PROJECT_BIN_DIR}\"

RC_ICONS = $$PWD/image/handtech.ico

LIBS += -L$$LIB_SHARE_PATH/windows/ -lAdvAPI32 -lUser32 -lshell32

RESOURCES += resources.qrc

HEADERS += \

SOURCES += \

CODECFORTR = UTF-8

msvc {
    QMAKE_CXXFLAGS += /MP
    QMAKE_CFLAGS += /MP
}
