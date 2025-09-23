if (WIN32)
    # set_path
    add_executable(set_path main.c )
    target_link_libraries(set_path PRIVATE smv)
    install(TARGETS set_path)
endif ()
