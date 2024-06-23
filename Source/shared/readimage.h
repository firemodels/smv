#ifndef READIMAGE_H_DEFINED
#define READIMAGE_H_DEFINED
unsigned char *ReadJPEG(const char *filename, int *width, int *height,
                        int *is_transparent);
unsigned char *ReadPNG(const char *filename, int *width, int *height,
                       int *is_transparent);
unsigned char *ReadPicture(char *texturedir, char *filename, int *width,
                           int *height, int *is_transparent, int printflag);
#endif
