
# smvq
add_executable(smvq smvq.c
    ../smokeview/menus.c
    ../smokeview/IOscript.c
    ../smokeview/IOshooter.c
    ../smokeview/colortimebar.c
    ../smokeview/camera.c
    ../smokeview/IOgeometry.c
    ../smokeview/IOwui.c
    ../smokeview/IOobjects.c
    ../smokeview/IOtour.c
    ../smokeview/getdatacolors.c
    ../smokeview/smokeview.c
    ../smokeview/output.c
    ../smokeview/renderimage.c
    ../smokeview/renderhtml.c
    ../smokeview/getdatabounds.c
    ../smokeview/readsmv.c
    ../smokeview/IOvolsmoke.c
    ../smokeview/IOsmoke.c
    ../smokeview/IOplot3d.c
    ../smokeview/IOplot2d.c
    ../smokeview/IOslice.c
    ../smokeview/IOhvac.c
    ../smokeview/IOboundary.c
    ../smokeview/IOpart.c
    ../smokeview/IOzone.c
    ../smokeview/IOiso.c
    ../smokeview/callbacks.c
    ../smokeview/drawGeometry.c
    ../smokeview/skybox.c
    ../smokeview/startup.c
    ../smokeview/shaders.c
    ../smokeview/unit.c
    ../smokeview/update.c
    ../smokeview/viewports.c
    ../smokeview/smv_geometry.c
    ../smokeview/showscene.c
    ../smokeview/infoheader.c
    ../smokeview/colortable.c
    ../smokeview/command_args.c

    ../smokeview/glui_smoke.cpp
    ../smokeview/glui_clip.cpp
    ../smokeview/glui_stereo.cpp
    ../smokeview/glui_geometry.cpp
    ../smokeview/glui_motion.cpp
    ../smokeview/glui_bounds.cpp
    ../smokeview/glui_colorbar.cpp
    ../smokeview/glui_display.cpp
    ../smokeview/glui_tour.cpp
    ../smokeview/glui_trainer.cpp
    ../smokeview/glui_objects.cpp
    ../smokeview/glui_shooter.cpp
)

target_include_directories(smvq PRIVATE
    ../shared
    ../glew
    ../smokeview
    ../glui_v2_1_beta
    ../glui_gl
)
target_link_libraries(smvq PRIVATE libsmv)

find_package(json-c CONFIG)
if (NOT json-c_FOUND)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(json-c json-c)
    endif()
    if (NOT json-c_FOUND)
        FetchContent_Declare(
            json-c-git
            EXCLUDE_FROM_ALL
            SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/json-c/json-c
            GIT_REPOSITORY https://github.com/json-c/json-c.git
            GIT_TAG        33337523f5cff82e659ecbe94b2fac19c7761a42
            FIND_PACKAGE_ARGS CONFIG NAMES json-c
        )
        FetchContent_MakeAvailable(json-c-git)
        target_link_libraries(smvq PRIVATE json-c-static)
        target_include_directories(smvq PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/json-c)
    else()
        message("${json-c_INCLUDE_DIRS}")
        target_link_libraries(smvq  PRIVATE ${json-c_LIBRARIES})
        target_include_directories(smvq  PRIVATE ${json-c_INCLUDE_DIRS})
        target_compile_options(smvq PUBLIC ${json-c_CFLAGS_OTHER})
    endif()
else()
    target_link_libraries(smvq PRIVATE json-c::json-c)
endif()

# These include directories are an existing workaround that would be good to
# remove.Because of this custom code and the potential conflict with native libs
# this needs to be early in the include order.
if (WIN32)
    target_include_directories(smvq PRIVATE ../glut_gl)
else()
    target_include_directories(smvq PRIVATE ../glui_gl)
endif ()
# GLUI can be provided natively, but there are modifications in the code
# vendored with Smokeview that we rely on. Because of this custom code and the
# potential conflict with native libs this needs to be early in the include order.
target_link_libraries(smvq PRIVATE glui_static)
target_include_directories(smvq PRIVATE ../glui_v2_1_beta)
if(WIN32)
    if (PThreads4W_FOUND)
        target_link_libraries(smvq PRIVATE PThreads4W::PThreads4W)
    else()
        target_include_directories(smvq PRIVATE ../pthreads)
        target_link_libraries(smvq PRIVATE pthread_static)
    endif()
endif()
# Selecting which GLUT version to use is the most platform-dependent part of the
# build.
if (MACOSX)
    target_link_libraries(smvq PRIVATE "-framework OpenGL" "-framework GLUT")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
elseif (GLUT_FOUND)
    target_link_libraries(smvq PRIVATE GLUT::GLUT)
else()
    target_link_libraries(smvq PRIVATE glut32_static)
endif ()

if (WIN32)
    target_include_directories(smvq PRIVATE ../pthreads)
    find_package(unofficial-getopt-win32 REQUIRED)
    target_link_libraries(smvq PRIVATE unofficial::getopt-win32::getopt)
endif()
if ((NOT MACOSX) AND UNIX)
    target_link_libraries(smvq PRIVATE m)
endif()
if (LINUX)
    add_definitions(-Dpp_LINUX)
    target_link_libraries(smvq PRIVATE pthread X11 Xmu GLU GL m stdc++)
endif()

install(TARGETS smvq)
