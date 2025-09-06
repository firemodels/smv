cmake_minimum_required(VERSION 3.17)
project(glut32)
set(PROJECT_VERSION 2.37)

set(COMMON_FILES
  glut_8x13.c
  glut_9x15.c
  glut_bitmap.c
  glut_bwidth.c
  glut_cindex.c
  glut_cmap.c
  glut_cursor.c
  glut_dials.c
  glut_dstr.c
  glut_event.c
  glut_ext.c
  glut_fullscrn.c
  glut_gamemode.c
  glut_get.c
  glut_glxext.c
  glut_hel10.c
  glut_hel12.c
  glut_hel18.c
  glut_init.c
  glut_input.c
  glut_joy.c
  glut_key.c
  glut_keyctrl.c
  glut_keyup.c
  glut_modifier.c
  glut_mroman.c
  glut_overlay.c
  glut_roman.c
  glut_shapes.c
  glut_space.c
  glut_stroke.c
  glut_swap.c
  glut_swidth.c
  glut_tablet.c
  glut_teapot.c
  glut_tr10.c
  glut_tr24.c
  glut_util.c
  glut_win.c
  glut_winmisc.c
  glut_mesa.c
  glut_warp.c
  glut_vidresize.c
)

set(UNIX_ONLY_FILES
  layerutil.c
  glut_menu.c
  glut_menu2.c
)

set(WINDOWS_ONLY_FILES
  win32_glx.c
  win32_menu.c
  win32_util.c
  win32_x11.c
  win32_winproc.c
)
if(BUILD_SHARED_LIBS)
  add_library(glut32 SHARED ${COMMON_FILES})
else()
  add_library(glut32 STATIC ${COMMON_FILES})
endif()
if (WIN32)
  target_link_libraries(glut32 PRIVATE winmm.lib)
endif ()
if (UNIX)
  target_sources(glut32 PRIVATE ${UNIX_ONLY_FILES})
elseif (WIN32)
  target_sources(glut32 PRIVATE ${WINDOWS_ONLY_FILES})
endif ()
if(LINUX)
  target_link_libraries(glut32 PUBLIC X11 Xmu GLU GL m stdc++ Xi)
endif()
target_include_directories(glut32 PUBLIC ../glut_gl)
set_target_properties(glut32 PROPERTIES OUTPUT_NAME "glut32")
if (WIN32)
  find_package(OpenGL REQUIRED)
  target_link_libraries(glut32 PUBLIC OpenGL::GL OpenGL::GLU)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
if (MSVC)
    target_compile_options(glut32 PRIVATE /wd4311)
endif ()
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    add_definitions(-Dpp_OSX)
endif()
# IntelLLVM warnings
target_compile_options(glut32 PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-bitwise-conditional-parentheses>)
target_compile_options(glut32 PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-pointer-to-int-cast>)
target_compile_options(glut32 PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-deprecated-declarations>)
target_compile_options(glut32 PRIVATE $<$<C_COMPILER_ID:IntelLLVM>: -Wno-pointer-sign>)
if (LINUX)
  # TODO: we shouldn't be ignoring these
  target_compile_options(glut32 PRIVATE -Wno-implicit-int -Wno-implicit-function-declaration -Wno-int-to-pointer-cast -Wno-dangling-else)
endif()

install(TARGETS glut32)
