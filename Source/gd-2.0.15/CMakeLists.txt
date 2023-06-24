cmake_minimum_required(VERSION 3.17)
project(gd LANGUAGES C VERSION 5.2.1)

set(FILES
    gd.c gd_gd.c gd_gd2.c gd_io.c gd_io_dp.c
    gd_io_file.c gd_ss.c gd_io_ss.c gd_png.c gd_jpeg.c gdxpm.c
    gdfontt.c gdfonts.c gdfontmb.c gdfontl.c gdfontg.c
    gdtables.c gdft.c gdcache.c gdkanji.c wbmp.c
    gd_wbmp.c gdhelpers.c gd_topal.c
)

include_directories(".")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DHAVE_LIBPNG -DHAVE_LIBZ -DHAVE_LIBJPEG")
add_library(gd_static STATIC ${FILES})
set_target_properties(gd_static PROPERTIES OUTPUT_NAME "gd")
target_include_directories(gd_static PUBLIC .)

if(UNIX)
    set(STDINT "-DHAVE_STDINT_H")
endif()

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32)
endif()
if (MSVC)
    target_compile_options(gd_static PRIVATE /wd4133)
endif()

if (JPEG_FOUND)
    target_link_libraries(gd_static PRIVATE JPEG::JPEG)
else()
    target_link_libraries(gd_static PRIVATE jpeg_static)
endif()

if (PNG_FOUND)
    target_link_libraries(gd_static PRIVATE PNG::PNG)
else()
    target_link_libraries(gd_static PRIVATE png_static)
endif()

if (ZLIB_FOUND)
    target_link_libraries(gd_static PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(gd_static PRIVATE zlib_static)
endif()

set(MAJOR_VERSION 2)
set(VERSION 2.0.4)

# IntelLLVM warnings, the IntelLLVM target needs CMake 3.20
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20")
    target_compile_options(gd_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-incompatible-pointer-types>)
    target_compile_options(gd_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-sizeof-pointer-memaccess>)
    target_compile_options(gd_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-absolute-value>)
else()
    target_compile_options(gd_static PRIVATE -Wno-incompatible-pointer-types)
endif()
