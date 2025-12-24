
QT       += core gui widgets xml

TARGET = using_computer_vision
TEMPLATE = app
CONFIG   += c++11

include(using_computer_vision.pri)
include($$PWD/../install.pri)

INCLUDEPATH += $$PWD/3rdParty/spdlog-1.2.1/include

LIB_OPENCV_PATH = C:/opencv-4.12.0-windows/build
INCLUDEPATH += $$LIB_OPENCV_PATH/include/

LIB_HALCON_PATH = C:/MVTec-24.05.0.0/HALCON-24.05-Progress
INCLUDEPATH += $$LIB_HALCON_PATH/include/
INCLUDEPATH += $$LIB_HALCON_PATH/include/halconcpp
INCLUDEPATH += $$LIB_HALCON_PATH/include/hdevengine

CONFIG(debug, debug|release) {
    LIBS += -L$$PROJECT_LIB_DIR -lcomputer_visiond
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc16/lib -lopencv_world4120d

    QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc16/bin/opencv_world4120d.dll\" \"$${PROJECT_LIB_DIR}\"
} else {
    LIBS += -L$$PROJECT_LIB_DIR -lcomputer_vision
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc16/lib -lopencv_world4120

    QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc16/bin/opencv_world4120.dll\" \"$${PROJECT_LIB_DIR}\"
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
