
QT       += core gui axcontainer xml network concurrent serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 6): QT += core5compat

CONFIG += c++11

DEFINES += QT_MESSAGELOGCONTEXT

include(../install.pri)
include(computer_vision.pri)
include(compile.pri)

INCLUDEPATH += $$LIB_SHARE_PATH/share
INCLUDEPATH += $$LIB_SHARE_PATH/3rdParty/spdlog-1.2.1/include
INCLUDEPATH += $$LIB_SHARE_PATH/3rdParty/hwinfo-vs2013/include

LIB_OPENCV_PATH = $$LIB_SHARE_PATH/3rdParty/opencv-2.4.9/build
INCLUDEPATH += $$LIB_OPENCV_PATH/include/

win32-msvc*:CONFIG(release, debug|release) {
    # 禁用严格字符串检查
    QMAKE_CXXFLAGS_RELEASE += /Zc:strictStrings-

    # 禁用特定警告
    QMAKE_CXXFLAGS_RELEASE += /wd4996    # 不安全函数警告
    QMAKE_CXXFLAGS_RELEASE += /wd4267    # 大小转换警告
    QMAKE_CXXFLAGS_RELEASE += /wd4244    # 类型转换警告

    # 预处理器定义
    DEFINES += '_STRICT'
    DEFINES += '_CRT_SECURE_NO_WARNINGS'
    DEFINES += '_CRT_NONSTDC_NO_WARNINGS'
    DEFINES += '_WIN32_WINNT=0x0600'
}

CONFIG(debug, debug|release) {
    LIBS += -L$$PROJECT_LIB_DIR -lworkflowd -lresourcesd -lalgorithmd -lcamerad -lplcd

    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249d \
                                             -lopencv_highgui249d \
                                             -lopencv_imgproc249d
    QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roid.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249d.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249d.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249d.dll\" \"$${PROJECT_LIB_DIR}\"

    LIBS += -L$$LIB_SHARE_PATH/3rdParty/hwinfo-vs2013/debug/lib -lhwinfo_battery -lhwinfo_cpu -lhwinfo_disk -lhwinfo_gpu -lhwinfo_mainboard -lhwinfo_network -lhwinfo_os -lhwinfo_ram

    COPY_DLL_FROM = $$LIB_SHARE_PATH/3rdParty/hwinfo-vs2013/debug/bin
    COPY_DLL_FROM = $$replace(COPY_DLL_FROM, /, \\)
    COPY_DLL_TO = $$replace(PROJECT_BIN_DIR, /, \\)

    QMAKE_POST_LINK += & cmd /c if not exist \"$$COPY_DLL_TO\" mkdir \"$$COPY_DLL_TO\"
    QMAKE_POST_LINK += & cmd /c xcopy /I /E /Y \"$${COPY_DLL_FROM}\\*.dll\" \"$${COPY_DLL_TO}\"
} else {
    LIBS += -L$$PROJECT_LIB_DIR -lworkflow -lresources -lalgorithm -lcamera -lplc
    LIBS += -L$$LIB_OPENCV_PATH/x64/vc12/lib -lopencv_core249 \
                                             -lopencv_highgui249 \
                                             -lopencv_imgproc249
    QMAKE_POST_LINK += cmd /c xcopy /D /F /Y \"$$LIB_SHARE_PATH/libroi/bin/roi.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_core249.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_highgui249.dll\" \"$${PROJECT_LIB_DIR}\"
    QMAKE_POST_LINK += & cmd /c xcopy /D /F /Y \"$$LIB_OPENCV_PATH/x64/vc12/bin/opencv_imgproc249.dll\" \"$${PROJECT_LIB_DIR}\"

    LIBS += -L$$LIB_SHARE_PATH/3rdParty/hwinfo-vs2013/release/lib -lhwinfo_battery -lhwinfo_cpu -lhwinfo_disk -lhwinfo_gpu -lhwinfo_mainboard -lhwinfo_network -lhwinfo_os -lhwinfo_ram

    COPY_DLL_FROM = $$LIB_SHARE_PATH/3rdParty/hwinfo-vs2013/release/bin
    COPY_DLL_FROM = $$replace(COPY_DLL_FROM, /, \\)
    COPY_DLL_TO = $$replace(PROJECT_BIN_DIR, /, \\)

    QMAKE_POST_LINK += & cmd /c if not exist \"$$COPY_DLL_TO\" mkdir \"$$COPY_DLL_TO\"
    QMAKE_POST_LINK += & cmd /c xcopy /I /E /Y \"$${COPY_DLL_FROM}\\*.dll\" \"$${COPY_DLL_TO}\"
}

LIBS += -lpsapi

RC_ICONS = $$PWD/../resources/image/handtech.ico

LIBS += -L$$LIB_SHARE_PATH/windows/ -lAdvAPI32 -lUser32 -lshell32

RESOURCES += \

HEADERS += \

SOURCES += \

CODECFORTR = UTF-8

msvc {
    QMAKE_CXXFLAGS += /MP
    QMAKE_CFLAGS += /MP
}
