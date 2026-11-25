add_library(zlibstatic STATIC
    adler32.c
    compress.c
    crc32.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    uncompr.c
    deflate.c
    infback.c
    trees.c
    zutil.c
    inflate.c
    inftrees.c
    inffast.c
)
if (CMAKE_C_COMPILER_ID STREQUAL Clang)
  target_compile_options(zlibstatic PRIVATE
    -Wno-deprecated-non-prototype
    -Wno-implicit-function-declaration
    -Wno-deprecated-declarations
  )
endif()
target_include_directories(zlibstatic PUBLIC .)
