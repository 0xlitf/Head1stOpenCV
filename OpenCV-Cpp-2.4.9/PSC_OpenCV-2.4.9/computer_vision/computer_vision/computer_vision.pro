
QT       += core

TARGET = algorithm
TEMPLATE = lib
CONFIG += shared c++11

DEFINES += QT_MESSAGELOGCONTEXT

include(../../install.pri)
include(computer_vision.pri)
include(compile.pri)

INCLUDEPATH += $$PWD/3rdParty/spdlog-1.2.1/include

LIB_OPENCV_PATH = C:/opencv-2.4.9/build
INCLUDEPATH += $$LIB_OPENCV_PATH/include/

LIB_HALCON_PATH = C:/MVTec-24.05.0.0/HALCON-24.05-Progress
INCLUDEPATH += $$LIB_HALCON_PATH/include/
INCLUDEPATH += $$LIB_HALCON_PATH/include/halconcpp
INCLUDEPATH += $$LIB_HALCON_PATH/include/hdevengine

CONFIG(debug, debug|release) {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249d \
                                             -lopencv_highgui249d \
                                             -lopencv_imgproc249d

    # QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roid.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK +=   cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249d.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249d.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249d.dll\" \"$${PROJECT_LIB_DIR}\"
} else {
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249 \
                                             -lopencv_highgui249 \
                                             -lopencv_imgproc249
    # QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roi.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK +=   cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249.dll\" \"$${PROJECT_LIB_DIR}\"
}

LIBS += -L$$LIB_HALCON_PATH/lib/x64-win64 -lhalcon \
                                            -lhalconcpp \
                                            -lhdevenginecpp
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_HALCON_PATH/bin/x64-win64/halcon.dll\" \"$${PROJECT_LIB_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_HALCON_PATH/bin/x64-win64/halconcpp.dll\" \"$${PROJECT_LIB_DIR}\"
QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_HALCON_PATH/bin/x64-win64/hdevenginecpp.dll\" \"$${PROJECT_LIB_DIR}\"

RESOURCES += \

HEADERS += \

SOURCES += \

CODECFORTR = UTF-8

msvc {
    QMAKE_CXXFLAGS += /MP
    QMAKE_CFLAGS += /MP
}
