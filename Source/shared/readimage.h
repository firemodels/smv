#ifndef READIMAGE_H_DEFINED
#define READIMAGE_H_DEFINED
unsigned char *ReadJPEG(const char *filename, int *width, int *height,
                        int *is_transparent);
unsigned char *ReadPNG(const char *filename, int *width, int *height,
                       int *is_transparent);
unsigned char *ReadPicture(char *texturedir, char *filename, int *width,
                           int *height, int *is_transparent, int printflag);
void           EncodePNGData(unsigned char *buffer, int nbuffer, unsigned char *data, int ndata, int skip, int channel);
unsigned char *DecodePNGData(unsigned char *buffer, int nbuffer, int *ndataptr, int skip, int channel);
#endif
