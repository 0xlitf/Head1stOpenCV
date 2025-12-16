cmake_minimum_required(VERSION 3.16)

get_filename_component(current_dir "${CMAKE_CURRENT_LIST_DIR}" NAME)
string(REPLACE " " "" current_dir "${current_dir}")
string(REPLACE "-" "_" current_dir "${current_dir}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_VERBOSE_MAKEFILE ON)
message("CMake start \n\t directory name: ${current_dir}")

project("${current_dir}" VERSION 0.1 LANGUAGES CXX)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# insure this block is before add_subdirectory(...)
set(CUSTOM_LIB_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_lib)
set(CUSTOM_BIN_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_bin)
if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME})

    # set(EXACT_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})

    set(EXACT_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/Debug)
    set(EXACT_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/Release)
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})

    set(EXACT_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})
endif()

message(STATUS "***** EXACT_OUTPUT_DIRECTORY: ${EXACT_OUTPUT_DIRECTORY} *****")


set(3rdParty_DIR ${CMAKE_CURRENT_LIST_DIR}/../3rdParty CACHE STRING "3rdParty_DIR CACHED")


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# set(OPENCV_ROOT "C:/opencv-4.12.0-windows")
# set(OPENCV_BUILD_DIR "${OPENCV_ROOT}/build")

set(OpenCV_DIR "C:/opencv-4.12.0-windows/build")
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(QT_DEBUG_FIND_PACKAGE ON)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)


# include_directories(
#     "${OPENCV_BUILD_DIR}/include"
#     "${OPENCV_BUILD_DIR}/include/opencv2"
# )


# file(GLOB_RECURSE ${PROJECT_NAME_UPPER}_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*")
file(GLOB_RECURSE ${PROJECT_NAME_UPPER}_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hh"
)

# message("${PROJECT_NAME_UPPER}_SRC: ${${PROJECT_NAME_UPPER}_SRC}")

# 排除build目录
list(FILTER ${PROJECT_NAME_UPPER}_SRC EXCLUDE REGEX ".*/build/.*")
list(FILTER ${PROJECT_NAME_UPPER}_SRC EXCLUDE REGEX ".*/moc_.*\\..*$")
# 排除qrc_开头的文件
list(FILTER ${PROJECT_NAME_UPPER}_SRC EXCLUDE REGEX ".*/qrc_.*\\..*$")
# 排除ui_开头的文件
list(FILTER ${PROJECT_NAME_UPPER}_SRC EXCLUDE REGEX ".*/ui_.*\\..*$")
# 排除CMake生成的文件
list(FILTER ${PROJECT_NAME_UPPER}_SRC EXCLUDE REGEX ".*CMakeCXXCompilerId.*")
list(FILTER ${PROJECT_NAME_UPPER}_SRC EXCLUDE REGEX ".*CMakeCCompilerId.*")

message("${PROJECT_NAME_UPPER}_SRC: ${${PROJECT_NAME_UPPER}_SRC}")


file(GLOB ${PROJECT_NAME_UPPER}_UI "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui")
file(GLOB ${PROJECT_NAME_UPPER}_QRC "${CMAKE_CURRENT_SOURCE_DIR}/qrc/*.qrc")
file(GLOB ${PROJECT_NAME_UPPER}_TS "${CMAKE_CURRENT_SOURCE_DIR}/ts/*.ts")

if(MSVC)
        add_compile_options(/Zc:__cplusplus)
        # set_property(TARGET main PROPERTY WIN32_EXECUTABLE true)
endif()

set(PROJECT_SOURCES
    ${${PROJECT_NAME_UPPER}_SRC}
    ${${PROJECT_NAME_UPPER}_UI}
    ${${PROJECT_NAME_UPPER}_QRC}
    ${${PROJECT_NAME_UPPER}_TS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../share/layoutbuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../share/layoutbuilder.cpp
)


set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/logo.rc")
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        qt_add_executable(${PROJECT_NAME}
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            ${app_icon_resource_windows}
        )
    else()
        qt_add_executable(${PROJECT_NAME}
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            ${app_icon_resource_windows}
        )
    endif()
else()
    if(ANDROID)
        add_library(untitledqt5 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            add_executable(${PROJECT_NAME}
                ${PROJECT_SOURCES}
                ${app_icon_resource_windows}
            )
        else()
            add_executable(${PROJECT_NAME}
                ${PROJECT_SOURCES}
                ${app_icon_resource_windows}
            )
        endif()
    endif()
endif()

get_target_property(target_location ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
message("\t${PROJECT_NAME} target_location: ${target_location}")


target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../share/
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME})
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32 TRUE
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )
endif()


file(GLOB subdirectories_list LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/src/*")
foreach(subdirectory ${subdirectories_list})
    if(IS_DIRECTORY ${subdirectory})
        list(APPEND filtered_subdirectories_list ${subdirectory})
    endif()
endforeach()
list(APPEND filtered_subdirectories_list ${CMAKE_CURRENT_SOURCE_DIR}/src)

message(STATUS "${ESC}[34;1m3rdParty Include Dirs: ${ESC}[0m")

foreach(subdir ${3rdParty_Include})
    message(STATUS "\t\t${subdir}")
endforeach()
message(STATUS "\n")


include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()



get_target_property(target_RUNTIME_location ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
message("\t${PROJECT_NAME} target_RUNTIME_location: ${target_RUNTIME_location}")
get_target_property(target_ARCHIVE_location ${PROJECT_NAME} ARCHIVE_OUTPUT_DIRECTORY)
message("\t${PROJECT_NAME} target_ARCHIVE_location: ${target_ARCHIVE_location}")
get_target_property(target_LIBRARY_location ${PROJECT_NAME} LIBRARY_OUTPUT_DIRECTORY)
message("\t${PROJECT_NAME} target_LIBRARY_location: ${target_LIBRARY_location}")

set(CUSTOM_LIB_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_lib)
set(CUSTOM_BIN_DIR ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_bin)
if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME})

    set(EXACT_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CUSTOM_LIB_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})

    set(EXACT_OUTPUT_DIRECTORY ${CUSTOM_BIN_DIR}/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE})
endif()

message(STATUS "***** EXACT_OUTPUT_DIRECTORY: ${EXACT_OUTPUT_DIRECTORY} *****")

message(STATUS "***** CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR} *****")

message(STATUS "***** CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR} *****")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(LIB60870_DLLS_DIR "${3rdParty_DIR}/x64/lib60870-C-2.3.2/bin")
    file(GLOB LIB60870_DLL_FILES "${LIB60870_DLLS_DIR}/*.dll")
    file(COPY ${LIB60870_DLL_FILES} DESTINATION ${EXACT_OUTPUT_DIRECTORY})

    # file(COPY "${CMAKE_CURRENT_LIST_DIR}/../windeployqt6_x64.bat" DESTINATION ${EXACT_OUTPUT_DIRECTORY})
else()
    set(LIB60870_DLLS_DIR "${3rdParty_DIR}/win32/lib60870-C-2.3.2/bin")
    file(GLOB LIB60870_DLL_FILES "${LIB60870_DLLS_DIR}/*.dll")
    file(COPY ${LIB60870_DLL_FILES} DESTINATION ${EXACT_OUTPUT_DIRECTORY})

    # file(COPY "${CMAKE_CURRENT_LIST_DIR}/../windeployqt6_win32.bat" DESTINATION ${EXACT_OUTPUT_DIRECTORY})
endif()


target_compile_definitions(${PROJECT_NAME} PRIVATE QT_MESSAGELOGCONTEXT)
