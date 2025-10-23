cmake_minimum_required(VERSION 3.16)

project(QMatPlotWidget VERSION 0.2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(QMATPLOTWIDGETT_BUILD_EXAMPLES "Build the examples" OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qwt REQUIRED)
message(STATUS "Found Qwt ${QWT_VERSION_STRING} at ${QWT_INCLUDE_DIR}")

# Library.
add_subdirectory(src)

# Examples.
if(QMATPLOTWIDGETT_BUILD_EXAMPLES)
    add_subdirectory(examples/realtimeplot)
    add_subdirectory(examples/simpleplot)
    # add_subdirectory(examples/qwt)
endif()

