add_executable(simpleplot EXCLUDE_FROM_ALL
    main.cpp
)

target_include_directories(simpleplot PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(simpleplot PRIVATE
    ${PROJECT_NAME}
)