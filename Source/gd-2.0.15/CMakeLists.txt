cmake_minimum_required(VERSION 3.21)
project(gd LANGUAGES C VERSION 5.2.1)

set(FILES
    gd.c gd_gd.c gd_gd2.c gd_io.c gd_io_dp.c
    gd_io_file.c gd_ss.c gd_io_ss.c gd_png.c gd_jpeg.c gdxpm.c
    gdfontt.c gdfonts.c gdfontmb.c gdfontl.c gdfontg.c
    gdtables.c gdft.c gdcache.c gdkanji.c wbmp.c
    gd_wbmp.c gdhelpers.c gd_topal.c
)

if(UNIX)
    set(STDINT "-DHAVE_STDINT_H")
endif()

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32)
endif ()

include_directories(".")
include_directories("../zlib128")
include_directories("../jpeg-9b")
include_directories("../png-1.6.21")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBJPEG -Wno-sizeof-pointer-memaccess -Wno-absolute-value")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBJPEG")
add_library(gd_static STATIC ${FILES})
set_target_properties(gd_static PROPERTIES OUTPUT_NAME "gd")
target_include_directories(gd_static PUBLIC "include")

target_link_libraries(gd_static jpeg_static )

set(MAJOR_VERSION 2)
set(VERSION 2.0.4)
