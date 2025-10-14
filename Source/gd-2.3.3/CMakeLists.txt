cmake_minimum_required(VERSION 3.17)
project(gd LANGUAGES C VERSION 5.2.1)

set(FILES
    gd.c
    gd_avif.c
    gd_bmp.c
    gd_color.c
    gd_color_map.c
    gd_color_match.c
    gd_crop.c
    gd_filename.c
    gd_filter.c
    gd_gd.c
    gd_gd2.c
    gd_gif_in.c
    gd_gif_out.c
    gd_heif.c
    gd_interpolation.c
    gd_io.c
    gd_io_dp.c
    gd_io_file.c
    gd_io_ss.c
    gd_matrix.c
    gd_nnquant.c
    gd_jpeg.c
    gd_png.c
    gd_rotate.c
    gd_security.c
    gd_ss.c
    gd_tga.c
    gd_tiff.c
    gd_topal.c
    gd_transform.c
    gd_version.c
    gd_wbmp.c
    gd_webp.c
    gd_xbm.c
    gdcache.c
    gdfontg.c
    gdfontl.c
    gdfontmb.c
    gdfonts.c
    gdfontt.c
    gdft.c
    gdfx.c
    gdhelpers.c
    gdkanji.c
    gdtables.c
    gdxpm.c
    wbmp.c
)

include_directories(".")
add_library(gd_static STATIC ${FILES})
target_compile_definitions(gd_static PUBLIC
    -DHAVE_LIBPNG
    -DHAVE_LIBZ
    -DHAVE_LIBJPEG
    -DBGDWIN32
    -DNONDLL
    -DWINVER=0x0500
    -D_WIN32_WINNT=0x0500
    -D_WIN32_IE=0x0600
)
if (WIN32)
    target_compile_definitions(gd_static PUBLIC HAVE_BOOLEAN)
endif()
target_include_directories(gd_static PUBLIC .)

if(UNIX)
    target_compile_definitions(gd_static PUBLIC -DHAVE_STDINT_H)
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
    target_link_libraries(gd_static PRIVATE zlibstatic)
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
if (CMAKE_C_COMPILER_ID STREQUAL Clang)
    target_compile_options(gd_static PRIVATE
        -Wno-deprecated-non-prototype
        -Wno-implicit-function-declaration
        -Wno-deprecated-declarations
        -Wno-ignored-pragmas
    )
endif()
