# smv
add_library(smv STATIC
    isobox.c
    dmalloc.c
    compress.c
    file_util.c
    string_util.c
    threader.c
    histogram.c
    translate.c
    csphere.c
    md5.c
    sha1.c
    sha256.c
    stdio_m.c
    stdio_buffer.c
    getdata.c
    color2rgb.c
    colorbars.c
    colorbar_defs.c
    readimage.c
    readcad.c
    readgeom.c
    readhvac.c
    readslice.c
    readsmoke.c
    readobject.c
    readlabel.c
    readtour.c
    scontour2d.c
    readsmvfile.c
    "paths.c"
)
set_source_files_properties(dmalloc.c PROPERTIES SKIP_LINTING ON)
target_include_directories(smv PUBLIC .)
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(smv PRIVATE PThreads4W::PThreads4W)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            install(DIRECTORY ${PThreads4W_DEBUG_DLL_DIR}/
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                FILES_MATCHING PATTERN "*.dll")
        else()
            install(DIRECTORY ${PThreads4W_DLL_DIR}/
                    DESTINATION ${CMAKE_INSTALL_BINDIR}
                    FILES_MATCHING PATTERN "*.dll")
        endif()
    else()
        target_link_libraries(smv PUBLIC pthread_static)
    endif()
endif()

if (JPEG_FOUND)
    target_link_libraries(smv PUBLIC JPEG::JPEG)
else()
    target_link_libraries(smv PUBLIC jpeg_static)
endif()

if (PNG_FOUND)
    target_link_libraries(smv PUBLIC PNG::PNG)
else()
    target_link_libraries(smv PUBLIC png_static)
endif()

if (ZLIB_FOUND)
    target_link_libraries(smv PUBLIC ZLIB::ZLIB)
else()
    target_link_libraries(smv PUBLIC zlibstatic)
endif()

if (LIBGD_FOUND AND (NOT VENDORED_LIBS))
    target_link_libraries(smv PUBLIC PkgConfig::LIBGD)
else()
    target_link_libraries(smv PUBLIC gd_static)
endif()

target_link_libraries(smv PUBLIC OpenGL::GL OpenGL::GLU)

if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smv PUBLIC m)
endif()
if (SMV_ROOT_OVERRIDE)
    target_compile_definitions(smv PUBLIC SMV_ROOT_OVERRIDE="${SMV_ROOT_OVERRIDE}")
endif()
