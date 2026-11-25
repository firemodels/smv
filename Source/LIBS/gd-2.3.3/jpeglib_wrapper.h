#ifndef JPEGLIB_WRAPPER_H
#define JPEGLIB_WRAPPER_H

/* Rename 'boolean' for libjpeg to avoid conflicts */
typedef int jpeg_boolean;
#define boolean jpeg_boolean

/* Include the actual libjpeg header */
#include "jpeglib.h"

/* Restore the original 'boolean' so system headers still see it correctly */
#undef boolean

#endif /* JPEGLIB_WRAPPER_H */
