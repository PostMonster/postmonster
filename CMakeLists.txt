cmake_minimum_required(VERSION 2.8.12)
project(postmonster)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
