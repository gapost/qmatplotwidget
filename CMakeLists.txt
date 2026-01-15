cmake_minimum_required(VERSION 3.16)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(QMatPlotWidget 
  VERSION 0.2.0 
  LANGUAGES CXX
  DESCRIPTION "A Qt plot widget with MATLAB-style interface"
  HOMEPAGE_URL https://gitlab.com/qdaq/qmatplotwidget
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build the examples
option(QMATPLOTWIDGETT_BUILD_EXAMPLES "Build the examples" OFF)

# Default install prefix (if not set by user)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if((CMAKE_SYSTEM_NAME STREQUAL "Windows") AND (NOT CMAKE_CROSSCOMPILING))
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${PROJECT_NAME}" CACHE PATH "Default install path" FORCE)
  else()
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Default install path" FORCE)
  endif()
endif()

# Install folder support
include(GNUInstallDirs)

# Find dependencies.
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


message(STATUS "----------------------------------------")
message(STATUS "CMake configuration summary for ${PROJECT_NAME}")
message(STATUS "  CMake version:        ${CMAKE_VERSION} (Generator: ${CMAKE_GENERATOR})")
message(STATUS "  Project version:      ${PROJECT_VERSION}")
message(STATUS "  C++ standard:         ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build type:           ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix:       ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  CMAKE_PREFIX_PATH:    ${CMAKE_PREFIX_PATH}")
message(STATUS "  Module path:          ${CMAKE_MODULE_PATH}")
message(STATUS "  C++ compiler:         ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Qt version (found):   ${Qt5_VERSION}")
message(STATUS "  Qwt version (found):  ${QWT_VERSION_STRING}")
message(STATUS "  Qwt include dir:      ${QWT_INCLUDE_DIR}")
message(STATUS "  Install dirs:         lib=${CMAKE_INSTALL_LIBDIR} bin=${CMAKE_INSTALL_BINDIR} include=${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "----------------------------------------")

