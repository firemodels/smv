add_library(png_static STATIC
  png.c
  pngerror.c
  pngget.c
  pngmem.c
  pngpread.c
  pngread.c
  pngrio.c
  pngrtran.c
  pngrutil.c
  pngset.c
  pngtrans.c
  pngwio.c
  pngwrite.c
  pngwtran.c
  pngwutil.c
)
if (CMAKE_C_COMPILER_ID STREQUAL Clang)
  target_compile_options(png_static PRIVATE -Wno-deprecated-non-prototype -Wno-implicit-function-declaration)
endif()
target_compile_definitions(png_static PUBLIC
-DPNG_ARM_NEON_OPT=0 -DPNG_MIPS_MSA_OPT=0
-DPNG_POWERPC_VSX_OPT=0 -DPNG_INTEL_SSE_OPT=0
)
target_compile_definitions(png_static PRIVATE VER=1.1.4)
target_include_directories(png_static PRIVATE ../zlib131)
target_include_directories(png_static PUBLIC .)
target_link_directories(png_static PRIVATE ../zlib131)
