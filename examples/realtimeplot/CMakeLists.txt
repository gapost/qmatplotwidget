add_executable(realtimeplot
    main.cpp
    widget.h
    widget.cpp
)

target_include_directories(realtimeplot PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(realtimeplot PRIVATE
    ${PROJECT_NAME}
)