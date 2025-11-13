# smokeview
add_executable(smokeview
    main.c
    menus.c
    IOscript.c
    IOshooter.c
    colortimebar.c
    camera.c
    IOgeometry.c
    IOwui.c
    IOobjects.c
    IOtour.c
    getdatacolors.c
    smokeview.c
    output.c
    renderimage.c
    renderhtml.c
    getdatabounds.c
    readsmv.c
    IOvolsmoke.c
    IOsmoke.c
    IOplot3d.c
    IOplot2d.c
    IOslice.c
    IOhvac.c
    IOboundary.c
    IOpart.c
    IOzone.c
    IOiso.c
    callbacks.c
    drawGeometry.c
    skybox.c
    update.c
    viewports.c
    smv_geometry.c
    showscene.c
    infoheader.c
    startup.c
    shaders.c
    unit.c
    colortable.c
    command_args.c
)

# GLUI sources are separate here. In some branches GLUI is conditional.
target_sources(smokeview PRIVATE
    glui_smoke.cpp
    glui_clip.cpp
    glui_stereo.cpp
    glui_geometry.cpp
    glui_motion.cpp
    glui_bounds.cpp
    glui_colorbar.cpp
    glui_display.cpp
    glui_tour.cpp
    glui_trainer.cpp
    glui_objects.cpp
    glui_shooter.cpp
)

target_link_libraries(smokeview PRIVATE smv)
# These include directories are an existing workaround that would be good to
# remove.Because of this custom code and the potential conflict with native libs
# this needs to be early in the include order.
if (WIN32)
    target_include_directories(smokeview PRIVATE ../glut_gl)
else()
    target_include_directories(smokeview PRIVATE ../glui_gl)
endif ()

if(WIN32)
    target_link_libraries(smokeview PRIVATE wsock32 ws2_32)
endif()
if (VENDORED_UI_LIBS)
    target_include_directories(smokeview PRIVATE ../glui_v2_1_beta)
    target_link_libraries(smokeview PRIVATE glui_static)
else()
    FetchContent_MakeAvailable(glui-git)
    target_link_libraries(smokeview PRIVATE glui)
endif()

# Selecting which GLUT version to use is the most platform-dependent part of the
# build.
if (MACOSX)
    add_definitions(-Dpp_NOQUARTZ)
    target_link_libraries(smokeview PRIVATE "-framework OpenGL" "-framework GLUT" "-lc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
elseif (GLUT_FOUND)
    target_link_libraries(smokeview PRIVATE GLUT::GLUT)
else()
    target_link_libraries(smokeview PRIVATE glut32)
endif ()
if (GLEW_FOUND)
    target_link_libraries(smokeview PRIVATE GLEW::GLEW)
    # This line is a hack to work around the fact the code includes "glew.h"
    # rather than <GL/glew.h> TODO: just change this
    target_include_directories(smokeview PRIVATE ${GLEW_INCLUDE_DIRS}/GL)
else()
    target_link_libraries(smokeview PRIVATE glew_static)
endif()

target_include_directories(smokeview PRIVATE .)

if (BETA)
    add_compile_definitions(pp_BETA)
endif ()

if (SMV_ROOT_OVERRIDE)
    target_compile_definitions(smokeview PRIVATE SMV_ROOT_OVERRIDE="${SMV_ROOT_OVERRIDE}")
endif()

if (SMOKEVIEW_CONFIG_PATH)
    target_compile_definitions(smokeview PRIVATE SMOKEVIEW_CONFIG_PATH="${SMOKEVIEW_CONFIG_PATH}")
endif()

if (SMOKEVIEW_OBJECT_DEFS_PATH)
    target_compile_definitions(smokeview PRIVATE SMOKEVIEW_OBJECT_DEFS_PATH="${SMOKEVIEW_OBJECT_DEFS_PATH}")
endif()

if (MSVC)
    # The headers in Source/glut_gl redefine some macros. Ideally those headers
    # could be removed but for now we just ignore the warning.
    target_compile_options(smokeview PRIVATE /wd4005)
endif()

target_link_libraries(smokeview PRIVATE smv)

install(TARGETS smokeview)
