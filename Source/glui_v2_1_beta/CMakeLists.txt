cmake_minimum_required(VERSION 3.17)

project(glui)
set(PROJECT_VERSION 2.37)

find_package(OpenGL REQUIRED)

set(GLUI_SRCS
  glui_add_controls.cpp
  glui.cpp
  glui_bitmap_img_data.cpp
  glui_bitmaps.cpp
  glui_button.cpp
  glui_edittext.cpp
  glui_checkbox.cpp
  glui_node.cpp
  glui_radio.cpp
  glui_statictext.cpp
  glui_panel.cpp
  glui_separator.cpp
  glui_spinner.cpp
  glui_control.cpp
  glui_column.cpp
  glui_translation.cpp
  glui_rotation.cpp
  glui_mouse_iaction.cpp
  glui_listbox.cpp
  glui_rollout.cpp
  arcball.cpp
  algebra3.cpp
  quaternion.cpp
  glui.h
  glut_hel20.c
  glut_hel24.c
  glut_hel36.c
)
add_library(glui_static STATIC ${GLUI_SRCS})
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(MACOSX TRUE)
  add_definitions(-Dpp_OSX)
else ()
  target_include_directories(glui_static PRIVATE ../glui_gl)
endif()

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()
if (MACOSX)
  add_compile_options($<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations>)
endif()

# Warnings for everything except MSVC
target_compile_options(glui_static PRIVATE $<$<NOT:$<C_COMPILER_ID:MSVC>>: -Wno-write-strings>)
