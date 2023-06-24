cmake_minimum_required(VERSION 3.17)
project(png LANGUAGES C VERSION 5.2.1)

set(FILES
    png.c pngset.c pngget.c pngrutil.c pngtrans.c pngwutil.c
    pngread.c pngrio.c pngwio.c pngwrite.c pngrtran.c
    pngwtran.c pngmem.c pngerror.c pngpread.c
)

add_library(png_static STATIC ${FILES})
target_include_directories(png_static PUBLIC .)
set_target_properties(png_static PROPERTIES OUTPUT_NAME "png")
set(LIBNAME libpng12)
set(PNGMAJ 0)
set(PNGMIN 1.2.5)
set(PNGVER ${PNGMAJ}.${PNGMIN})

if (ZLIB_FOUND)
    target_link_libraries(png_static PRIVATE ZLIB::ZLIB)
else()
    target_link_libraries(png_static PRIVATE zlib_static)
endif()

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()
# IntelLLVM warnings
target_compile_options(png_static PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-tautological-constant-out-of-range-compare>)
