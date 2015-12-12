cmake_minimum_required(VERSION 2.8.12)
project(httptool) 

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Project dependencies
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

file(GLOB_RECURSE httptool_SOURCES "*.cpp")
file(GLOB_RECURSE httptool_UI "*.ui")

set(httptool_QRC httptool.qrc)

qt5_wrap_ui(httptool_UI_HEADERS ${httptool_UI})
qt5_add_resources(httptool_QRC_RCC ${httptool_QRC})

add_definitions(-DQT_STATICPLUGIN)
include_directories(../../public)

add_library(httptool STATIC ${httptool_SOURCES} ${httptool_UI_HEADERS} ${httptool_QRC_RCC})
target_link_libraries(httptool Qt5::Widgets Qt5::Network)