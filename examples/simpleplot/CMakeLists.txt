add_executable(simpleplot 
    main.cpp
)

target_link_libraries(simpleplot PRIVATE
    ${PROJECT_NAME}
)