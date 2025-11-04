cmake_minimum_required(VERSION 3.16)

project(BackgroundSubtraction VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# set(OPENCV_ROOT "C:/opencv-4.12.0-windows")
# set(OPENCV_BUILD_DIR "${OPENCV_ROOT}/build")

set(OpenCV_DIR "C:/opencv-4.12.0-windows/build")
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")

# include_directories(
#     "${OPENCV_BUILD_DIR}/include"
#     "${OPENCV_BUILD_DIR}/include/opencv2"
# )

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        backgroundsubtractor.h
        backgroundsubtractor.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    qt_add_executable(BackgroundSubtraction
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET BackgroundSubtraction APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(BackgroundSubtraction SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(BackgroundSubtraction
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(BackgroundSubtraction PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(BackgroundSubtraction PRIVATE Qt6::Widgets)

target_link_libraries(BackgroundSubtraction PRIVATE ${OpenCV_LIBS})
target_include_directories(BackgroundSubtraction PRIVATE ${OpenCV_INCLUDE_DIRS})

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.BackgroundSubtraction)
endif()
set_target_properties(BackgroundSubtraction PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS BackgroundSubtraction
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BackgroundSubtraction)
endif()
