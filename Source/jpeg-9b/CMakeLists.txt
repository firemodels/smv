cmake_minimum_required(VERSION 3.17)
project(jpeg LANGUAGES C VERSION 5.2.1)

# Put here the object file name for the correct system-dependent memory
# manager file.  For Unix this is usually jmemnobs.o, but you may want
# to use jmemansi.o or jmemname.o if you have limited swap space.
set(SYSDEPMEM jmemnobs.c)

# library object files common to compression and decompression
set(COMOBJECTS jcomapi.c jutils.c jerror.c jmemmgr.c ${SYSDEPMEM})
# compression library object files
set(CLIBOBJECTS jaricom.c jdarith.c jcarith.c jcapimin.c jcapistd.c jctrans.c jcparam.c
        jdatadst.c jcinit.c jcmaster.c jcmarker.c jcmainct.c
        jcprepct.c jccoefct.c jccolor.c jcsample.c jchuff.c
        jcdctmgr.c jfdctfst.c jfdctflt.c
        jfdctint.c)
# decompression library object files
set(DLIBOBJECTS jdapimin.c jdapistd.c jdtrans.c jdatasrc.c
        jdmaster.c jdinput.c jdmarker.c jdhuff.c
        jdmainct.c jdcoefct.c jdpostct.c jddctmgr.c
        jidctfst.c jidctflt.c jidctint.c
        jdsample.c jdcolor.c jquant1.c jquant2.c jdmerge.c)
# These objectfiles are included in libjpeg.a
set(LIBOBJECTS ${CLIBOBJECTS} ${DLIBOBJECTS} ${COMOBJECTS})
set(JPEG_LIB_VERSION 62)

include_directories(".")
add_library(jpeg_static STATIC ${LIBOBJECTS})
set_target_properties(jpeg_static PROPERTIES OUTPUT_NAME "jpeg")
target_include_directories(jpeg_static PUBLIC .)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()
