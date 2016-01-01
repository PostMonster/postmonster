cmake_minimum_required(VERSION 2.8.12)
project(postmonster)

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

# Qt dependencies
find_package(Qt5Widgets       REQUIRED)
find_package(Qt5Network       REQUIRED)
find_package(Qt5WebKit        REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)
find_package(Qt5Script        REQUIRED)

# All include paths now relative to src folder
include_directories(src public)

# Static plugin dependencies
add_subdirectory(plugins/httptool)

# Find sources and ui files
file(GLOB_RECURSE postmonster_SOURCES "src/*.cpp" "public/*.h")
file(GLOB_RECURSE postmonster_UI "src/*.ui")

set(postmonster_QRC src/postmonster.qrc)

# Call uic and rcc
qt5_wrap_ui(postmonster_UI_SOURCES ${postmonster_UI})
qt5_add_resources(postmonster_QRC_RCC ${postmonster_QRC})

# Build
add_executable(postmonster ${postmonster_SOURCES} ${postmonster_UI_SOURCES} ${postmonster_QRC_RCC})
target_link_libraries(postmonster Qt5::Widgets Qt5::Network Qt5::WebKit
                                  Qt5::WebKitWidgets Qt5::Script
                                  httptool)
