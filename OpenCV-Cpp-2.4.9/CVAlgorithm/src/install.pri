
LIB_SHARE_PATH = C:/
LIB_SHARE_PATH = $$absolute_path($$LIB_SHARE_PATH)

CONFIG(debug, debug|release) {
    BUILD_TYPE = debug
    TARGET = $$join(TARGET,,,d)
} else {
    BUILD_TYPE = release
}

message("BUILD_TYPE: $$BUILD_TYPE")

win32 {
    CONFIG(debug, debug|release) {
        BUILD_DIR = $$absolute_path($$PWD/../../bin/Debug)
        TEMP_DIR = $$PWD/../../temp/Debug
    }
    else {
        BUILD_DIR = $$absolute_path($$PWD/../../bin/Release)
        TEMP_DIR = $$PWD/../../temp/Release
    }
}

PROJECT_BIN_DIR = $$BUILD_DIR
PROJECT_LIB_DIR = $$BUILD_DIR

DESTDIR = $$BUILD_DIR
OBJECTS_DIR = $$TEMP_DIR/obj
MOC_DIR = $$TEMP_DIR/moc
RCC_DIR = $$TEMP_DIR/rcc
UI_DIR = $$TEMP_DIR/ui

message("BUILD_DIR: $$BUILD_DIR")

PROJECT_LIB_DIR = $$absolute_path($$PROJECT_LIB_DIR)
message("PROJECT_LIB_DIR: $$PROJECT_LIB_DIR")

PROJECT_BIN_DIR = $$absolute_path($$PROJECT_BIN_DIR)
message("PROJECT_BIN_DIR: $$PROJECT_BIN_DIR")

CONFIG(debug, debug|release) {
    DEFAULT_DESTDIR = $$OUT_PWD/debug
} else {
    DEFAULT_DESTDIR = $$OUT_PWD/release
}
