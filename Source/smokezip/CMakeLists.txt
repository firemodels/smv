# smokezip
add_executable(smokezip
    main.c
    CNV3dsmoke.c
    CNVboundary.c
    CNVpart.c
    CNVslice.c
    readfiles.c
    utilities.c
    threaderzip.c
)
target_include_directories(smokezip PRIVATE .)
target_link_libraries(smokezip PRIVATE smv)

install(TARGETS smokezip)
