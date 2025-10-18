add_library(${PROJECT_NAME} STATIC
    QMatPlotWidget.cpp
    QMatPlotWidget.h
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    Qt::Core
    Qt::Widgets
    Qwt::Qwt
)