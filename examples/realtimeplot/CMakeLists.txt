add_executable(realtimeplot 
    main.cpp
    widget.h
    widget.cpp
)

target_link_libraries(realtimeplot PRIVATE
    ${PROJECT_NAME}
)