cmake_minimum_required(VERSION 3.21)
project(png LANGUAGES C VERSION 5.2.1)

set(FILES
    png.c pngset.c pngget.c pngrutil.c pngtrans.c pngwutil.c
    pngread.c pngrio.c pngwio.c pngwrite.c pngrtran.c
    pngwtran.c pngmem.c pngerror.c pngpread.c
)

include_directories(".")
include_directories("../zlib128")
add_library(png_static STATIC ${FILES})
set_target_properties(png_static PROPERTIES OUTPUT_NAME "png")
set(LIBNAME libpng12)
set(PNGMAJ 0)
set(PNGMIN 1.2.5)
set(PNGVER ${PNGMAJ}.${PNGMIN})

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-shift-negative-value -Wno-tautological-constant-out-of-range-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif ()
