
#ifndef COLORBARS_H_DEFINED
#define COLORBARS_H_DEFINED

#include "options.h"

#define FILE_UPDATE 6

#define CB_RAINBOW 0
#define CB_ORIGINAL 1
#define CB_LINEAR 2
#define CB_DIVERGENT 3
#define CB_CIRCULAR 4
#define CB_DEPRECATED 5
#define CB_USER 6
#define CB_OTHER 7
#define INTERP_RGB 0
#define INTERP_LAB 1

/* --------------------------  colordata ------------------------------------ */

typedef struct _colordata {
  float color[4], full_color[4], bw_color[4];
  struct _colordata *nextcolor;
} colordata;

typedef struct _colorbardata {
  /// @brief The label used in GUI menus
  char menu_label[1024];
  /// @brief The category of colorbar: rainbow, linear, divergent, etc.
  char colorbar_type[256];
  int nnodes, nnodes_orig, node_index_orig[1024], nodehilight, type;
  unsigned char node_rgb_orig[3 * 1024], node_rgb[3 * 1024],
      colorbar_alpha[1024];
  unsigned char node_index[1024]; // colorbar index
  float node_dist[1024];
  float colorbar_dist[256];
  int dist_ind[256];
  float colorbar_dist_delta[1024];
  /// @brief Is the interpolation RGB (INTERP_RGB) or LAB (INTERP_LAB)?
  int interp;
  int can_adjust, adjusted;
  float colorbar_rgb[3 * 1024], colorbar_lab[3 * 1024];
} colorbardata;

typedef struct {
  /// @brief The capacity of the colorbarinfo array.
  int capacity;
  /// @brief The number of colorbars in the collection
  int ncolorbars;
  /// @brief An array of colorbars of length ncolorbars
  colorbardata *colorbarinfo;
  /// @brief How many of the colobars in colorbarinfo are defaults?
  int ndefaultcolorbars;

  int bw_colorbar_index;
  int fire_colorbar_index;
  colorbardata *fire_colorbar;

  colorbardata *levelset_colorbar;

  int split_colorbar_index;
  colorbardata *split_colorbar;
  int co2_colorbar_index;
  int iso_colorbar_index;
} colorbar_collection;

/**
 * @brief Get a colorbar given a label.
 *
 * @param[in] colorbars The colorbar collection to search through.
 * @param[in] label The label of the colorbar.
 * @return A pointer to the colorbar, or NULL if no matching colorbar was found.
 */
EXTERNCPP colorbardata *GetColorbar(colorbar_collection *colorbars,
                                    const char *label);

/**
 * @brief Append a new (blank) colorbar to the colorbar collection.
 *
 * @param[inout] colorbars The colorbar collection.
 * @return A pointer to the new colorbar.
 */
EXTERNCPP colorbardata *NewColorbar(colorbar_collection *colorbars);
/**
 * @brief Read a colorbar from a CSV file into a colorbar struct.
 *
 * @param[out] colorbar The colorbar to read the data into.
 * @param[in] filepath The filepath of the CSV file.
 * @param[in] colorbar_type The type of the colorbar (string label).
 * @param[in] type The type of the colorbar (enum).
 * @return 0 on success, non-zero on failure.
 */
EXTERNCPP int ReadCSVColorbar(colorbardata *colorbar, const char *filepath,
                     const char *colorbar_type, int type);
/**
 * @brief Initialize the default colorbars. This includes:
 *    - Initializing the colorbars which are hardcoded.
 *    - Reading and initializing from the SMV_ROOT_DIR.
 *    - Reading and initializing from the use config directory.
 *
 * @param colorbars The colorbar collection to read into.
 * @param nini
 * @param show_extreme_mindata Should this colorbar color data below it's
 * bounds?
 * @param rgb_below_min What color should data below the minimum bound be
 * colored.
 * @param show_extreme_maxdata Should this colorbar color data above it's
 * bounds?
 * @param rgb_above_max What color should data above the maximum bound be
 * colored.
 * @param colorbarcopyinfo
 */
EXTERNCPP void InitDefaultColorbars(colorbar_collection *colorbars, int nini,
                                    int show_extreme_mindata,
                                    unsigned char rgb_below_min[3],
                                    int show_extreme_maxdata,
                                    unsigned char rgb_above_max[3],
                                    colorbardata **colorbarcopyinfoptr);



EXTERNCPP void AdjustColorBar(colorbardata *cbi);
EXTERNCPP void RemapColorbar(colorbardata *cbi, int show_extreme_mindata,
                             unsigned char rgb_below_min[3],
                             int show_extreme_maxdata,
                             unsigned char rgb_above_max[3]);
EXTERNCPP void Lab2XYZ(float *xyz, float *lab);
EXTERNCPP void CheckLab(void);
EXTERNCPP void FRgb2Lab(float *rgb_arg, float *lab);
EXTERNCPP void GetColorDist(colorbardata *cbi, int option, float *min, float *max);

#endif
