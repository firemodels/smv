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
    c_api.c
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

target_link_libraries(smokeview PRIVATE libsmv)

# These include directories are an existing workaround that would be good to
# remove.Because of this custom code and the potential conflict with native libs
# this needs to be early in the include order.
if (WIN32)
    target_include_directories(smokeview PRIVATE ../glut_gl)
else()
    target_include_directories(smokeview PRIVATE ../glui_gl)
endif ()
# GLUI can be provided natively, but there are modifications in the code
# vendored with Smokeview that we rely on. Because of this custom code and the
# potential conflict with native libs this needs to be early in the include order.
target_link_libraries(smokeview PRIVATE glui_static)
target_include_directories(smokeview PRIVATE ../glui_v2_1_beta)

if(WIN32)
    target_link_libraries(smokeview PRIVATE PThreads4W::PThreads4W)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        install(DIRECTORY ${PThreads4W_DEBUG_DLL_DIR}/
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            FILES_MATCHING PATTERN "*.dll")
    else()
        install(DIRECTORY ${PThreads4W_DLL_DIR}/
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                FILES_MATCHING PATTERN "*.dll")
    endif()
endif()

# Selecting which GLUT version to use is the most platform-dependent part of the
# build.
if (MACOSX)
    target_link_libraries(smokeview PRIVATE "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
elseif (GLUT_FOUND)
    target_link_libraries(smokeview PRIVATE GLUT::GLUT)
else()
    target_link_libraries(smokeview PRIVATE glut32_static)
endif ()
target_link_libraries(smokeview PRIVATE GLEW::GLEW)
if (GLEW_FOUND)
    # This line is a hack to work around the fact the code includes "glew.h"
    # rather than <GL/glew.h> TODO: just change this
    target_include_directories(smokeview PRIVATE ${GLEW_INCLUDE_DIRS}/GL)
endif()
target_link_libraries(smokeview PRIVATE JPEG::JPEG)
target_link_libraries(smokeview PRIVATE PNG::PNG)
target_link_libraries(smokeview PRIVATE ZLIB::ZLIB)
target_link_libraries(smokeview PRIVATE OpenGL::GL OpenGL::GLU)

target_include_directories(smokeview PRIVATE
    .
    ../shared
)

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
if (LINUX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smokeview PRIVATE pthread X11 Xmu GLU GL m stdc++)
endif()

install(TARGETS smokeview)
