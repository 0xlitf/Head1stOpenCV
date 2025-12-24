
QT       += core

TARGET = computer_vision
TEMPLATE = lib
CONFIG += shared c++11
CONFIG -= app_bundle
CONFIG -= console

DEFINES += QT_MESSAGELOGCONTEXT

include($$PWD/../install.pri)
include($$PWD/computer_vision/computer_vision.pri)
include($$PWD/computer_vision/compile.pri)

INCLUDEPATH += $$PWD/3rdParty/spdlog-1.2.1/include

LIB_OPENCV_PATH = C:/opencv-4.12.0-windows/build
INCLUDEPATH += $$LIB_OPENCV_PATH/include/

LIB_HALCON_PATH = C:/MVTec-24.05.0.0/HALCON-24.05-Progress
INCLUDEPATH += $$LIB_HALCON_PATH/include/
INCLUDEPATH += $$LIB_HALCON_PATH/include/halconcpp
INCLUDEPATH += $$LIB_HALCON_PATH/include/hdevengine

message("LIB_HALCON_PATH: $$LIB_HALCON_PATH")

CONFIG(debug, debug|release) {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc16/lib -lopencv_world4120d

    # QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roid.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc16/bin/opencv_world4120d.dll\" \"$${PROJECT_LIB_DIR}\"
} else {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc16/lib -lopencv_world4120
    # QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roi.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK +=   cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc16/bin/opencv_world4120.dll\" \"$${PROJECT_LIB_DIR}\"
}

LIBS += -L$$LIB_HALCON_PATH/lib/x64-win64 -lhalcon \
                                            -lhalconcpp \
                                            -lhdevenginecpp
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_HALCON_PATH/bin/x64-win64/halcon.dll\" \"$${PROJECT_LIB_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_HALCON_PATH/bin/x64-win64/halconcpp.dll\" \"$${PROJECT_LIB_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_HALCON_PATH/bin/x64-win64/hdevenginecpp.dll\" \"$${PROJECT_LIB_DIR}\"

CODECFORTR = UTF-8

msvc {
    QMAKE_CXXFLAGS += /MP
    QMAKE_CFLAGS += /MP
}
