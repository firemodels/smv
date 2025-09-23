# smokediff
add_executable(smokediff
    main.c
    IOdboundary.c
    IOdplot.c
    IOdslice.c
    readsmv.c
    utilities.c
)
target_include_directories(smokediff PRIVATE .)
target_link_libraries(smokediff PRIVATE smv)

install(TARGETS smokediff)
