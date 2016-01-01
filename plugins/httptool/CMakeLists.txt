cmake_minimum_required(VERSION 2.8.12)
project(httptool) 

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Enable C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

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
