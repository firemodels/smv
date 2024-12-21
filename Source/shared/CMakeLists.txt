# libsmv
add_library(libsmv STATIC
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
    readcad.c
    readlabel.c
    readtour.c
    scontour2d.c
)
target_include_directories(libsmv PUBLIC
    .
    #TODO: this is a bit of a hack, it shouldn't really use options.h
    ../smokeview
)
if(WIN32)
    target_link_libraries(libsmv PRIVATE PThreads4W::PThreads4W)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        install(DIRECTORY ${PThreads4W_DEBUG_DLL_DIR}/
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            FILES_MATCHING PATTERN "*.dll")
    else()
        install(DIRECTORY ${PThreads4W_DLL_DIR}/
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                FILES_MATCHING PATTERN "*.dll")
    endif()
endif()
if (JPEG_FOUND)
    target_link_libraries(libsmv PUBLIC JPEG::JPEG)
else()
    target_link_libraries(libsmv PUBLIC jpeg_static)
endif()
if (PNG_FOUND)
    target_link_libraries(libsmv PUBLIC PNG::PNG)
else()
    target_link_libraries(libsmv PUBLIC png_static)
endif()
if (ZLIB_FOUND)
    target_link_libraries(libsmv PUBLIC ZLIB::ZLIB)
else()
    target_link_libraries(libsmv PUBLIC zlib_static)
endif()
if (LIBGD_FOUND)
    target_link_libraries(libsmv PUBLIC PkgConfig::LIBGD)
else()
    target_link_libraries(libsmv PUBLIC gd_static)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(libsmv PUBLIC m)
endif()
target_link_libraries(libsmv PRIVATE OpenGL::GL OpenGL::GLU)
if (GLEW_FOUND)
    target_link_libraries(libsmv PUBLIC GLEW::GLEW)
    # This line is a hack to work around the fact the code includes "glew.h"
    # rather than <GL/glew.h>
    target_include_directories(libsmv PUBLIC ${GLEW_INCLUDE_DIRS}/GL)
else()
    target_sources(libsmv PUBLIC
        ../glew/glew.c
    )
    target_include_directories(libsmv PUBLIC ../glew)
endif()
