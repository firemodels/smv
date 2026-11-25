add_library(glew_static STATIC glew.c)
target_include_directories(glew_static PUBLIC .)
target_compile_definitions(glew_static PUBLIC GLEW_STATIC)
