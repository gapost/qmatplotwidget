add_library(${PROJECT_NAME} STATIC
    QMatPlotWidget.cpp
    QMatPlotWidget.h
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Qt::Core
    Qt::Widgets
  PRIVATE
    Qwt::Qwt
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)