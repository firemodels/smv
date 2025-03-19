#include "options.h"
#include "glew.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "compress.h"

#define HEADER_SIZE 4
#define TRAILER_SIZE 4
#define FORTVOLSLICEREAD(var,size) FSEEK(SLICEFILE,HEADER_SIZE,SEEK_CUR);\
                           fread(var,4,size,SLICEFILE);\
                           FSEEK(SLICEFILE,TRAILER_SIZE,SEEK_CUR)

#ifndef VEC4EQCONS
#define VEC4EQCONS(y,x)\
  (y)[0]=(x);\
  (y)[1]=(x);\
  (y)[2]=(x);\
  (y)[3]=(x)
#endif

#define INTERP3D(data,value) \
    if(slicetype==SLICE_NODE_CENTER){\
      float dx, dy, dz;\
      float val000, val100, val010, val110;\
      float val001, val101, val011, val111;\
      float  val00, val01, val10, val11;\
      float val0, val1;\
      float *vv;\
\
      ijk = IJKNODE(i, j, k);\
\
      dx = (xyz[0]-xplt[i])/dxbar;               \
      dx = CLAMP(dx, 0.0, 1.0);                  \
      dy = (xyz[1]-yplt[j])/dybar;               \
      dy = CLAMP(dy, 0.0, 1.0);                  \
      dz = (xyz[2]-zplt[k])/dzbar;               \
      dz = CLAMP(dz, 0.0, 1.0);                  \
\
      vv = data+ijk;                             \
      val000 = vv[0];                 /* i,  j,k     */  \
      val100 = vv[1];                 /* i+1,j,k   */  \
\
      vv += nx;\
      val010 = vv[0];                 /* i,  j+1,k   */  \
      val110 = vv[1];                 /* i+1,j+1,k */  \
\
      vv += (nxy-nx);\
      val001 = vv[0];                 /* i,  j,k+1   */  \
      val101 = vv[1];                 /* i+1,j,k+1 */  \
\
      vv += nx;\
      val011 = vv[0];                /* i,  j+1,k+1  */ \
      val111 = vv[1];                /* i+1,j+1,k+1  */ \
\
      val00 = (1.0-dx)*val000 + dx*val100;\
      val10 = (1.0-dx)*val010 + dx*val110;\
      val01 = (1.0-dx)*val001 + dx*val101;\
      val11 = (1.0-dx)*val011 + dx*val111;\
       val0 = (1.0-dy)*val00  + dy*val10;\
       val1 = (1.0-dy)*val01  + dy*val11;\
      value = (1.0-dz)*val0   + dz*val1;\
    }\
    else{\
      float *vv;\
      vv = data+IJKNODE(i+1, j+1, k+1);\
      value = *vv;\
    }

// https://www.fourmilab.ch/documents/specrend/specrend.c

/* A colour system is defined by the CIE x and y coordinates of
   its three primary illuminants and the x and y coordinates of
   the white point. */

struct colourSystem{
  char *name;                     /* Colour system name */
  float xRed, yRed,              /* Red x, y */
    xGreen, yGreen,          /* Green x, y */
    xBlue, yBlue,            /* Blue x, y */
    xWhite, yWhite,          /* White point x, y */
    gamma;                   /* Gamma correction for system */
};

/* White point chromaticities. */

#define IlluminantC     0.3101, 0.3162          /* For NTSC television */
#define IlluminantD65   0.3127, 0.3291          /* For EBU and SMPTE */
#define IlluminantE     0.33333333, 0.33333333  /* CIE equal-energy illuminant */

/*  Gamma of nonlinear correction.

    See Charles Poynton's ColorFAQ Item 45 and GammaFAQ Item 6 at:

       http://www.poynton.com/ColorFAQ.html
       http://www.poynton.com/GammaFAQ.html

*/

#define GAMMA_REC709    0               /* Rec. 709 */

//static struct colourSystem SMPTEsystem = {"SMPTE",              0.630,  0.340,  0.310,  0.595,  0.155,  0.070,  IlluminantD65,  GAMMA_REC709};
//static struct colourSystem CIEsystem = {"CIE",                0.7355, 0.2645, 0.2658, 0.7243, 0.1669, 0.0085, IlluminantE,    GAMMA_REC709};
static struct colourSystem HDTVsystem = {"HDTV",               0.670,  0.330,  0.210,  0.710,  0.150,  0.060,  IlluminantD65,  GAMMA_REC709};
//static struct colourSystem NTSCsystem = {"NTSC",               0.67,   0.33,   0.21,   0.71,   0.14,   0.08,   IlluminantC,    GAMMA_REC709};

/*                             XYZ_TO_RGB

    Given an additive tricolour system CS, defined by the CIE x
    and y chromaticities of its three primaries (z is derived
    trivially as 1-(x+y)), and a desired chromaticity (XC, YC,
    ZC) in CIE space, determine the contribution of each
    primary in a linear combination which sums to the desired
    chromaticity.  If the  requested chromaticity falls outside
    the Maxwell  triangle (colour gamut) formed by the three
    primaries, one of the r, g, or b weights will be negative.

    Caller can use ConstrainRgb() to desaturate an
    outside-gamut colour to the closest representation within
    the available gamut and/or norm_rgb to normalise the RGB
    components so the largest nonzero component has value 1.

*/

/* ----------------------- Xyz2Rgb ----------------------------- */

void Xyz2Rgb(struct colourSystem *cs, float *xyz_c, float *rgb_arg){
  float xr, yr, zr, xg, yg, zg, xb, yb, zb;
  float xw, yw, zw;
  float rx, ry, rz, gx, gy, gz, bx, by, bz;
  float rw, gw, bw;

  xr = cs->xRed;    yr = cs->yRed;    zr = 1 - (xr + yr);
  xg = cs->xGreen;  yg = cs->yGreen;  zg = 1 - (xg + yg);
  xb = cs->xBlue;   yb = cs->yBlue;   zb = 1 - (xb + yb);

  xw = cs->xWhite;  yw = cs->yWhite;  zw = 1 - (xw + yw);

  /* xyz -> rgb matrix, before scaling to white. */

  rx = (yg*zb) - (yb*zg);  ry = (xb*zg) - (xg*zb);  rz = (xg*yb) - (xb*yg);
  gx = (yb*zr) - (yr*zb);  gy = (xr*zb) - (xb*zr);  gz = (xb*yr) - (xr*yb);
  bx = (yr*zg) - (yg*zr);  by = (xg*zr) - (xr*zg);  bz = (xr*yg) - (xg*yr);

  /* White scaling factors.
     Dividing by yw scales the white luminance to unity, as conventional. */

  rw = ((rx*xw) + (ry*yw) + (rz*zw)) / yw;
  gw = ((gx*xw) + (gy*yw) + (gz*zw)) / yw;
  bw = ((bx*xw) + (by*yw) + (bz*zw)) / yw;

  /* xyz -> rgb matrix, correctly scaled to white. */

  rx = rx / rw;  ry = ry / rw;  rz = rz / rw;
  gx = gx / gw;  gy = gy / gw;  gz = gz / gw;
  bx = bx / bw;  by = by / bw;  bz = bz / bw;

  /* rgb of the desired point */

  rgb_arg[0] = (rx*xyz_c[0]) + (ry*xyz_c[1]) + (rz*xyz_c[2]);
  rgb_arg[1] = (gx*xyz_c[0]) + (gy*xyz_c[1]) + (gz*xyz_c[2]);
  rgb_arg[2] = (bx*xyz_c[0]) + (by*xyz_c[1]) + (bz*xyz_c[2]);
}

/*                            INSIDE_GAMUT

     Test whether a requested colour is within the gamut
     achievable with the primaries of the current colour
     system.  This amounts simply to testing whether all the
     primary weights are non-negative. */

/* ----------------------- InsideGamut ----------------------------- */

int InsideGamut(float *rgb_arg){
  return (rgb_arg[0] >= 0.0) && (rgb_arg[1] >= 0.0) && (rgb_arg[2] >= 0.0);
}

/*                          CONSTRAIN_RGB

    If the requested RGB shade contains a negative weight for
    one of the primaries, it lies outside the colour gamut
    accessible from the given triple of primaries.  Desaturate
    it by adding white, equal quantities of R, G, and B, enough
    to make RGB all positive.  The function returns 1 if the
    components were modified, zero otherwise.

*/

/* ----------------------- ConstrainRgb ----------------------------- */

int ConstrainRgb(float *rgb_arg){
  float w;

  /* Amount of white needed is w = - min(0, *r, *g, *b) */

  w = (0 < rgb_arg[0])?0:rgb_arg[0];
  w = (w < rgb_arg[1])?w:rgb_arg[1];
  w = (w < rgb_arg[2])?w:rgb_arg[2];
  w = -w;

  /* Add just enough white to make r, g, b all positive. */

  if(w > 0){
    rgb_arg[0] += w;  rgb_arg[1] += w; rgb_arg[2] += w;
    return 1;                     /* Colour modified to fit RGB gamut */
  }
  return 0;                         /* Colour within RGB gamut */
}

/*                          GAMMA_CORRECT_RGB

    Transform linear RGB values to nonlinear RGB values. Rec.
    709 is ITU-R Recommendation BT. 709 (1990) ``Basic
    Parameter Values for the HDTV Standard for the Studio and
    for International Programme Exchange'', formerly CCIR Rec.
    709. For details see

       http://www.poynton.com/ColorFAQ.html
       http://www.poynton.com/GammaFAQ.html
*/

/*                            BB_SPECTRUM

    Calculate, by Planck's radiation law, the emittance of a black body
    of temperature bbTemp at the given wavelength (in metres).  */

    /* ----------------------- BlackBodySpectrum ----------------------------- */

float BlackBodySpectrum(float wavelength, float temperature){
  float wlm = wavelength*1e-9;   /* Wavelength in meters */

  return (3.74183e-16*pow(wlm, -5.0)) /
    (exp(1.4388e-2 / (wlm*temperature)) - 1.0);
}

/*                          SPECTRUM_TO_XYZ

    Calculate the CIE X, Y, and Z coordinates corresponding to
    a light source with spectral distribution given by  the
    function SPEC_INTENS, which is called with a series of
    wavelengths between 380 and 780 nm (the argument is
    expressed in meters), which returns emittance at  that
    wavelength in arbitrary units.  The chromaticity
    coordinates of the spectrum are returned in the x, y, and z
    arguments which respect the identity:

            x + y + z = 1.
*/

/* ----------------------- Spectrum2Xyz ----------------------------- */

void Spectrum2Xyz(float temperature, float *xyz){
  int i;
  float lambda, X = 0, Y = 0, Z = 0, XYZ;

  /* CIE colour matching functions xBar, yBar, and zBar for
     wavelengths from 380 through 780 nanometers, every 5
     nanometers.  For a wavelength lambda in this range:

          cie_colour_match[(lambda - 380) / 5][0] = xBar
          cie_colour_match[(lambda - 380) / 5][1] = yBar
          cie_colour_match[(lambda - 380) / 5][2] = zBar

      To save memory, this table can be declared as floats
      rather than doubles; (IEEE) float has enough
      significant bits to represent the values. It's declared
      as a double here to avoid warnings about "conversion
      between floating-point types" from certain persnickety
      compilers. */

  static float cie_colour_match[81][3] = {
      {0.0014,0.0000,0.0065}, {0.0022,0.0001,0.0105}, {0.0042,0.0001,0.0201},
      {0.0076,0.0002,0.0362}, {0.0143,0.0004,0.0679}, {0.0232,0.0006,0.1102},
      {0.0435,0.0012,0.2074}, {0.0776,0.0022,0.3713}, {0.1344,0.0040,0.6456},
      {0.2148,0.0073,1.0391}, {0.2839,0.0116,1.3856}, {0.3285,0.0168,1.6230},
      {0.3483,0.0230,1.7471}, {0.3481,0.0298,1.7826}, {0.3362,0.0380,1.7721},
      {0.3187,0.0480,1.7441}, {0.2908,0.0600,1.6692}, {0.2511,0.0739,1.5281},
      {0.1954,0.0910,1.2876}, {0.1421,0.1126,1.0419}, {0.0956,0.1390,0.8130},
      {0.0580,0.1693,0.6162}, {0.0320,0.2080,0.4652}, {0.0147,0.2586,0.3533},
      {0.0049,0.3230,0.2720}, {0.0024,0.4073,0.2123}, {0.0093,0.5030,0.1582},
      {0.0291,0.6082,0.1117}, {0.0633,0.7100,0.0782}, {0.1096,0.7932,0.0573},
      {0.1655,0.8620,0.0422}, {0.2257,0.9149,0.0298}, {0.2904,0.9540,0.0203},
      {0.3597,0.9803,0.0134}, {0.4334,0.9950,0.0087}, {0.5121,1.0000,0.0057},
      {0.5945,0.9950,0.0039}, {0.6784,0.9786,0.0027}, {0.7621,0.9520,0.0021},
      {0.8425,0.9154,0.0018}, {0.9163,0.8700,0.0017}, {0.9786,0.8163,0.0014},
      {1.0263,0.7570,0.0011}, {1.0567,0.6949,0.0010}, {1.0622,0.6310,0.0008},
      {1.0456,0.5668,0.0006}, {1.0026,0.5030,0.0003}, {0.9384,0.4412,0.0002},
      {0.8544,0.3810,0.0002}, {0.7514,0.3210,0.0001}, {0.6424,0.2650,0.0000},
      {0.5419,0.2170,0.0000}, {0.4479,0.1750,0.0000}, {0.3608,0.1382,0.0000},
      {0.2835,0.1070,0.0000}, {0.2187,0.0816,0.0000}, {0.1649,0.0610,0.0000},
      {0.1212,0.0446,0.0000}, {0.0874,0.0320,0.0000}, {0.0636,0.0232,0.0000},
      {0.0468,0.0170,0.0000}, {0.0329,0.0119,0.0000}, {0.0227,0.0082,0.0000},
      {0.0158,0.0057,0.0000}, {0.0114,0.0041,0.0000}, {0.0081,0.0029,0.0000},
      {0.0058,0.0021,0.0000}, {0.0041,0.0015,0.0000}, {0.0029,0.0010,0.0000},
      {0.0020,0.0007,0.0000}, {0.0014,0.0005,0.0000}, {0.0010,0.0004,0.0000},
      {0.0007,0.0002,0.0000}, {0.0005,0.0002,0.0000}, {0.0003,0.0001,0.0000},
      {0.0002,0.0001,0.0000}, {0.0002,0.0001,0.0000}, {0.0001,0.0000,0.0000},
      {0.0001,0.0000,0.0000}, {0.0001,0.0000,0.0000}, {0.0000,0.0000,0.0000}
  };

  for(i = 0, lambda = 380; lambda < 780.1; i++, lambda += 5){
    float Me;

    Me = BlackBodySpectrum(lambda, temperature);
    X += Me*cie_colour_match[i][0];
    Y += Me*cie_colour_match[i][1];
    Z += Me*cie_colour_match[i][2];
  }
  XYZ = (X + Y + Z);
  xyz[0] = X / XYZ;
  xyz[1] = Y / XYZ;
  xyz[2] = Z / XYZ;
}

/* ----------------------- Gaussian ----------------------------- */

float Gaussian(float x, float mu, float sigma1, float sigma2){
  float return_val, arg;

  if(x < mu){
    arg = (x - mu) / sigma1;
  }
  else{
    arg = (x - mu) / sigma2;
  }
  arg = -arg*arg / 2.0;
  return_val = exp(arg);
  return return_val;
}
// https://en.wikipedia.org/wiki/CIE_1931_color_space
/* ----------------------- ColorMatchRed ----------------------------- */

float ColorMatchRed(float lambda){
  float return_val;
  // units
  // lambda: nm

  return_val  = 1.056*Gaussian(lambda, 599.8, 37.9, 31.0);
  return_val += 0.362*Gaussian(lambda, 442.0, 16.0, 26.7);
  return_val -= 0.065*Gaussian(lambda, 501.1, 20.4, 26.2);
  return return_val;
}

/* ----------------------- ColorMatchGreen ----------------------------- */

float ColorMatchGreen(float lambda){
  float return_val;
  // units
  // lambda: nm

  return_val  = 0.821*Gaussian(lambda, 568.8, 46.9, 40.5);
  return_val += 0.286*Gaussian(lambda, 530.9, 16.3, 31.1);
  return return_val;
}


/* ----------------------- ColorMatchBlue ----------------------------- */

float ColorMatchBlue(float lambda){
  float return_val;
  // units
  // lambda: nm

  return_val  = 1.217*Gaussian(lambda, 437.0, 11.8, 36.0);
  return_val += 0.681*Gaussian(lambda, 459.0, 26.0, 13.8);
  return return_val;
}
/* ----------------------- GetPlankVal ----------------------------- */

float GetPlankVal(float lambda, float temp){
  // units
  // lambda: m
  // temp:   K
  // https://en.wikipedia.org/wiki/Planck%27s_law
  // https://www.fourmilab.ch/documents/specrend/
  float c1, c2;
  // (c1/lambda^5)/(exp(c2/(lambda*T)-1)
  //c1 = 2 pi hc^2 = 3.74183*10^-16 W m2
  //c2 = hc/k = 0.014388 m K
  c1 = 3.74183*pow(10.0, -16);
  c2 = 0.014388;
  float return_val = (c1 / pow(lambda, 5.0)) / (exp(c2 / (lambda*temp)) - 1.0);;
  return return_val;
}

/* ----------------------- GetRGBFireVal ----------------------------- */

void GetRGBFireVal(float temp, float *rgb_arg){
  // units
  // temp C
  int i, n;
  float valmin, valmax, dval, factor;

  static float cie_colour_match[81][3] = {
    {0.0014, 0.0000, 0.0065}, {0.0022, 0.0001, 0.0105}, {0.0042, 0.0001, 0.0201},
    {0.0076, 0.0002, 0.0362}, {0.0143, 0.0004, 0.0679}, {0.0232, 0.0006, 0.1102},
    {0.0435, 0.0012, 0.2074}, {0.0776, 0.0022, 0.3713}, {0.1344, 0.0040, 0.6456},
    {0.2148, 0.0073, 1.0391}, {0.2839, 0.0116, 1.3856}, {0.3285, 0.0168, 1.6230},
    {0.3483, 0.0230, 1.7471}, {0.3481, 0.0298, 1.7826}, {0.3362, 0.0380, 1.7721},
    {0.3187, 0.0480, 1.7441}, {0.2908, 0.0600, 1.6692}, {0.2511, 0.0739, 1.5281},
    {0.1954, 0.0910, 1.2876}, {0.1421, 0.1126, 1.0419}, {0.0956, 0.1390, 0.8130},
    {0.0580, 0.1693, 0.6162}, {0.0320, 0.2080, 0.4652}, {0.0147, 0.2586, 0.3533},
    {0.0049, 0.3230, 0.2720}, {0.0024, 0.4073, 0.2123}, {0.0093, 0.5030, 0.1582},
    {0.0291, 0.6082, 0.1117}, {0.0633, 0.7100, 0.0782}, {0.1096, 0.7932, 0.0573},
    {0.1655, 0.8620, 0.0422}, {0.2257, 0.9149, 0.0298}, {0.2904, 0.9540, 0.0203},
    {0.3597, 0.9803, 0.0134}, {0.4334, 0.9950, 0.0087}, {0.5121, 1.0000, 0.0057},
    {0.5945, 0.9950, 0.0039}, {0.6784, 0.9786, 0.0027}, {0.7621, 0.9520, 0.0021},
    {0.8425, 0.9154, 0.0018}, {0.9163, 0.8700, 0.0017}, {0.9786, 0.8163, 0.0014},
    {1.0263, 0.7570, 0.0011}, {1.0567, 0.6949, 0.0010}, {1.0622, 0.6310, 0.0008},
    {1.0456, 0.5668, 0.0006}, {1.0026, 0.5030, 0.0003}, {0.9384, 0.4412, 0.0002},
    {0.8544, 0.3810, 0.0002}, {0.7514, 0.3210, 0.0001}, {0.6424, 0.2650, 0.0000},
    {0.5419, 0.2170, 0.0000}, {0.4479, 0.1750, 0.0000}, {0.3608, 0.1382, 0.0000},
    {0.2835, 0.1070, 0.0000}, {0.2187, 0.0816, 0.0000}, {0.1649, 0.0610, 0.0000},
    {0.1212, 0.0446, 0.0000}, {0.0874, 0.0320, 0.0000}, {0.0636, 0.0232, 0.0000},
    {0.0468, 0.0170, 0.0000}, {0.0329, 0.0119, 0.0000}, {0.0227, 0.0082, 0.0000},
    {0.0158, 0.0057, 0.0000}, {0.0114, 0.0041, 0.0000}, {0.0081, 0.0029, 0.0000},
    {0.0058, 0.0021, 0.0000}, {0.0041, 0.0015, 0.0000}, {0.0029, 0.0010, 0.0000},
    {0.0020, 0.0007, 0.0000}, {0.0014, 0.0005, 0.0000}, {0.0010, 0.0004, 0.0000},
    {0.0007, 0.0002, 0.0000}, {0.0005, 0.0002, 0.0000}, {0.0003, 0.0001, 0.0000},
    {0.0002, 0.0001, 0.0000}, {0.0002, 0.0001, 0.0000}, {0.0001, 0.0000, 0.0000},
    {0.0001, 0.0000, 0.0000}, {0.0001, 0.0000, 0.0000}, {0.0000, 0.0000, 0.0000}
  };

  temp += 273.15;
  valmin = 380.0;
  valmax = 780.0;
  factor = pow(10.0, 9.0);
  n = 81;
  dval = (valmax - valmin)/(float)(n-1);;
  rgb_arg[0] = 0.0;
  rgb_arg[1] = 0.0;
  rgb_arg[2] = 0.0;
  for(i = 0;i < n;i++){
    float plank_val, lambda_nano, lambda_m;
    float rgb_val[3];

    lambda_nano = valmin + i*dval;
    lambda_m    = lambda_nano/factor;
    plank_val   = GetPlankVal(lambda_m, temp);
    rgb_val[0]  = plank_val*cie_colour_match[i][0];
    rgb_val[1]  = plank_val*cie_colour_match[i][1];
    rgb_val[2]  = plank_val*cie_colour_match[i][2];
    if(i == 0 || i == n - 1){
      rgb_arg[0] += rgb_val[0];
      rgb_arg[1] += rgb_val[1];
      rgb_arg[2] += rgb_val[2];
    }
    else{
      rgb_arg[0] += 2.0*rgb_val[0];
      rgb_arg[1] += 2.0*rgb_val[1];
      rgb_arg[2] += 2.0*rgb_val[2];
    }
  }
  rgb_arg[0] *= 0.5*dval;
  rgb_arg[1] *= 0.5*dval;
  rgb_arg[2] *= 0.5*dval;
}

/* ----------------------- MakeFireColors ----------------------------- */

void MakeFireColorsNew(float temp_min, float temp_max, int nfire_colors_arg){
  int i;
  float dtemp;

  dtemp = (temp_max - temp_min)/(float)(nfire_colors_arg -1);
  FREEMEMORY(fire_rgbs);
  NewMemory((void **)&fire_rgbs,3*nfire_colors_arg*sizeof(float));
  for(i = 0;i < nfire_colors_arg;i++){
    float temp, fire_rgb[3], xyz[3];

    temp = temp_min + (float)i*dtemp + 273.15;
    Spectrum2Xyz(temp, xyz);
    Xyz2Rgb(&HDTVsystem, xyz, fire_rgb);
// SMPTEsystem
// CIEsystem
// NTSCsystem
    ConstrainRgb(fire_rgb);
    memcpy(fire_rgbs + 3*i, fire_rgb, 3*sizeof(float));
  }
}

/* ----------------------- MakeFireColors ----------------------------- */

void MakeFireColors(float temp_min, float temp_max, int nfire_colors_arg){
  int i;
  float dtemp;

  dtemp = (temp_max - temp_min) / (float)(nfire_colors_arg - 1);
  FREEMEMORY(fire_rgbs);
  NewMemory((void **)&fire_rgbs, 3*nfire_colors_arg*sizeof(float));
  for(i = 0; i < nfire_colors_arg; i++){
    float temp, fire_emission[3];

    temp = temp_min + (float)i*dtemp;
    //float xyz[3];
    GetRGBFireVal(temp, fire_emission);
    //Xyz2Rgb(&HDTVsystem, xyz, fire_rgb);
    //ConstrainRgb(fire_rgb);
    memcpy(fire_rgbs + 3*i, fire_emission, 3*sizeof(float));
  }
}

/* ----------------------- Temperature2Emission ----------------------------- */

void Temperature2Emission(float temperature, float *emission){
  int index;
  float factor, *rgb_bef, *rgb_aft, temp_bef, temp_aft;

  if(temperature<=fire_temp_min){
    memcpy(emission, fire_rgbs, 3*sizeof(float));
    return;
  }
  if(temperature>=fire_temp_max){
    memcpy(emission, fire_rgbs+3*(nfire_colors-1), 3*sizeof(float));
    return;
  }
  index = nfire_colors*(temperature - fire_temp_min) / (fire_temp_max - fire_temp_min);
  rgb_bef = fire_rgbs+3*index;
  rgb_aft = rgb_bef + 3;
  temp_bef = fire_temp_min + (float)index*(fire_temp_max-fire_temp_min)/(float)(nfire_colors -1);
  temp_aft = fire_temp_min + (float)(index+1)*(fire_temp_max-fire_temp_min)/(float)(nfire_colors -1);
  factor = (temperature-temp_bef)/(temp_aft-temp_bef);
  emission[0] = (1.0-factor)*rgb_bef[0] + factor*rgb_aft[0];
  emission[1] = (1.0-factor)*rgb_bef[1] + factor*rgb_aft[1];
  emission[2] = (1.0-factor)*rgb_bef[2] + factor*rgb_aft[2];
}

/* ----------------------- GetFireEmission ----------------------------- */

void GetFireEmission(float *smoke_tran, float *fire_emission, float dlength, float xyz[3], meshdata *meshi, int *inobst, char *blank_local){
  int i, j, k;
  int ijk;

  int nx, ny, nz, nxy;

  float dxbar, dybar, dzbar;

  int ijkcell;
  float *xplt, *yplt, *zplt;
  int ibar, jbar, kbar;
  float *smokedata_local, *firedata_local;
  float black[] = {0.0,0.0,0.0,1.0};
  int slicetype;
  float soot_density, temperature;

  smokedata_local = meshi->volrenderinfo->smokedataptr;
  firedata_local  = meshi->volrenderinfo->firedataptr;
  slicetype = meshi->volrenderinfo->smokeslice->slice_filetype;

  if(slicetype==SLICE_NODE_CENTER){
    // check this
    xplt = meshi->xplt;
    yplt = meshi->yplt;
    zplt = meshi->zplt;
  }
  else{
    xplt = meshi->xplt_cen;
    yplt = meshi->yplt_cen;
    zplt = meshi->zplt_cen;
  }
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;

  dxbar = xplt[1]-xplt[0];
  dybar = yplt[1]-yplt[0];
  dzbar = zplt[1]-zplt[0];

  nx = ibar+1;
  ny = jbar+1;
  nz = kbar + 1;
  nxy = nx*ny;
  if(meshi->voltest_update == 1){
    float zmin, zmax;

    zmin = voltest_center[2] - voltest_r1;
    zmax = voltest_center[2] + voltest_r2;


    for(k=0;k<nz;k++){
      float dz;

      dz = voltest_center[2] - SMV2FDS_Z(zplt[k]);
      for(j = 0;j < ny;j++){
        float dy;

        dy = voltest_center[1] - SMV2FDS_Y(yplt[j]);
        for(i = 0;i < nx;i++){
          float dx, rad, rad_inner;
          float smoke_val, fire_val;

          dx = voltest_center[0] - SMV2FDS_X(xplt[i]);
          rad = sqrt(dx*dx + dy*dy + dz*dz);
          rad_inner = sqrt(dx*dx + dy*dy);
          if(rad < voltest_r1){
            smoke_val = voltest_soot1;
          }
          else if(rad >= voltest_r1 && rad <= voltest_r2){
            smoke_val = voltest_soot2;
          }
          else{
            smoke_val = 0.0;
          }
          if(rad_inner < voltest_r3){
            fire_val = (voltest_temp2 * (SMV2FDS_Z(zplt[k]) - zmin) + voltest_temp1 * (zmax - SMV2FDS_Z(zplt[k]))) / (zmax-zmin);
          }
          else{
            fire_val = voltest_temp1;
          }
          int ijk_index;
          int ii;

          ijk_index = IJKNODE(i, j, k);
          for(ii = 0;ii < meshi->volrenderinfo->ntimes;ii++){
            float *smokevals, *firevals;

            smokevals = meshi->volrenderinfo->smokedataptrs[ii];
            firevals = meshi->volrenderinfo->firedataptrs[ii];

            if(smokevals!=NULL)smokevals[ijk_index] = smoke_val;
            if(firevals!=NULL)firevals[ijk_index] = fire_val;
          }
        }
      }
    }
    meshi->voltest_update = 0;
  }

  i = GETINDEX(xyz[0], xplt[0], dxbar, ibar);
  j = GETINDEX(xyz[1], yplt[0], dybar, jbar);
  k = GETINDEX(xyz[2], zplt[0], dzbar, kbar);

  if(blank_local!=NULL){
    ijkcell = IJKCELL(i, j, k);
    if(blank_local[ijkcell]==SOLID){
      *inobst = 1;
      return;
    }
    *inobst = 0;
  }

  if(firedata_local!=NULL){
    INTERP3D(firedata_local, temperature);
    if(use_blackbody_colors == 1){
      Temperature2Emission(temperature, fire_emission);
    }
    else{
      if(temperature > global_temp_cb_min){
        int index;
        float temp_factor;

        temp_factor = (float)MAXSMOKERGB/(global_temp_cb_max - global_temp_min);
        index = CLAMP(temp_factor*(temperature - global_temp_min), 0, MAXSMOKERGB - 1);
        memcpy(fire_emission, rgb_volsmokecolormap + 4*index, 3*sizeof(float));
      }
      else{
        memcpy(fire_emission, black, 3*sizeof(float));
      }
    }
  }
  else{
    memcpy(fire_emission, black, 3*sizeof(float));
  }
  *smoke_tran = 1.0;
  if(smokedata_local!=NULL){
    INTERP3D(smokedata_local, soot_density);
    *smoke_tran = exp(-mass_extinct*soot_density*dlength);
    if(firedata_local!=NULL&&temperature<=global_temp_cb_min){
      memcpy(fire_emission, black, 3*sizeof(float));
    }
  }
  if(firedata_local!=NULL&&temperature>global_temp_cb_min){
    if(use_opacity_depth==1){
      *smoke_tran = pow(0.5, dlength/fire_halfdepth);
    }
    if(use_opacity_multiplier==1&&smokedata_local!=NULL){
      float absorb, trans;

      absorb = 1.0 - *smoke_tran;
      absorb *= emission_factor;
      trans = CLAMP(1.0 - absorb, 0.0, 1.0);
      *smoke_tran = trans;
    }
  }
}

/* ------------------ InitVolRenderSurface ------------------------ */

void InitVolRenderSurface(int flag){
  int i;

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    int ii;
    float dx, dy, dz;

    meshi = global_scase.meshescoll.meshinfo + i;
    meshi->ivolbar=meshi->ibar*nongpu_vol_factor;
    meshi->jvolbar=meshi->jbar*nongpu_vol_factor;
    meshi->kvolbar=meshi->kbar*nongpu_vol_factor;
    FREEMEMORY(meshi->xvolplt);
    FREEMEMORY(meshi->yvolplt);
    FREEMEMORY(meshi->zvolplt);
    NewMemory((void **)&meshi->xvolplt,(meshi->ivolbar+1)*sizeof(float));
    NewMemory((void **)&meshi->yvolplt,(meshi->jvolbar+1)*sizeof(float));
    NewMemory((void **)&meshi->zvolplt,(meshi->kvolbar+1)*sizeof(float));
    dx=(meshi->xplt[meshi->ibar]-meshi->xplt[0])/(float)meshi->ivolbar;
    dy=(meshi->yplt[meshi->jbar]-meshi->yplt[0])/(float)meshi->jvolbar;
    dz=(meshi->zplt[meshi->kbar]-meshi->zplt[0])/(float)meshi->kvolbar;
    for(ii=0;ii<=meshi->ivolbar;ii++){
      meshi->xvolplt[ii]=meshi->xplt[0]+(float)ii*dx;
    }
    for(ii=0;ii<=meshi->jvolbar;ii++){
      meshi->yvolplt[ii]=meshi->yplt[0]+(float)ii*dy;
    }
    for(ii=0;ii<=meshi->kvolbar;ii++){
      meshi->zvolplt[ii]=meshi->zplt[0]+(float)ii*dz;
    }
  }
  ijkbarmax=0;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    ijkbarmax=MAX(ijkbarmax,meshi->ivolbar);
    ijkbarmax=MAX(ijkbarmax,meshi->jvolbar);
    ijkbarmax=MAX(ijkbarmax,meshi->kvolbar);
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + i;
    vr = meshi->volrenderinfo;
    if(flag==FIRSTCALL){
      vr->smokecolor_yz0=NULL;
      vr->smokecolor_yz1=NULL;
      vr->smokecolor_xz0=NULL;
      vr->smokecolor_xz1=NULL;
      vr->smokecolor_xy0=NULL;
      vr->smokecolor_xy1=NULL;
    }
    else{
      FREEMEMORY(vr->smokecolor_yz0);
      FREEMEMORY(vr->smokecolor_yz1);
      FREEMEMORY(vr->smokecolor_xz0);
      FREEMEMORY(vr->smokecolor_xz1);
      FREEMEMORY(vr->smokecolor_xy0);
      FREEMEMORY(vr->smokecolor_xy1);
    }
    if(vr->smokeslice!=NULL){
      int nx, ny, nz;

      nx = ijkbarmax+1;
      ny = ijkbarmax+1;
      nz = ijkbarmax+1;
      NewMemory((void **)&vr->smokecolor_yz0,4*ny*nz*sizeof(float));
      NewMemory((void **)&vr->smokecolor_yz1,4*ny*nz*sizeof(float));
      NewMemory((void **)&vr->smokecolor_xz0,4*nx*nz*sizeof(float));
      NewMemory((void **)&vr->smokecolor_xz1,4*nx*nz*sizeof(float));
      NewMemory((void **)&vr->smokecolor_xy0,4*nx*ny*sizeof(float));
      NewMemory((void **)&vr->smokecolor_xy1,4*nx*ny*sizeof(float));
    }
  }
}

/* ------------------ InitVolsmokeSuperTexture ------------------------ */
#ifdef pp_GPU
void InitVolsmokeSuperTexture(supermeshdata *smesh){
  GLint border_size = 0;
  GLsizei nx, ny, nz;
  int i;

  nx = smesh->ibar+1;
  ny = smesh->jbar+1;
  nz = smesh->kbar+1;

  FFLUSH();

  glActiveTexture(GL_TEXTURE0);
  if(smesh->volsmoke_texture_id==0)glGenTextures(1, &smesh->volsmoke_texture_id);
  glBindTexture(GL_TEXTURE_3D, smesh->volsmoke_texture_id);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  if(smesh->volsmoke_texture_buffer==NULL){
    NewMemory((void **)&smesh->volsmoke_texture_buffer, nx*ny*nz*sizeof(float));
  }
  for(i = 0;i<nx*ny*nz;i++){
    smesh->volsmoke_texture_buffer[i] = 0.0;
  }
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F,
    nx, ny, nz, border_size,
    GL_RED, GL_FLOAT, smesh->volsmoke_texture_buffer);

  glActiveTexture(GL_TEXTURE1);
  if(smesh->volfire_texture_id==0)glGenTextures(1, &smesh->volfire_texture_id);
  glBindTexture(GL_TEXTURE_3D, smesh->volfire_texture_id);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  if(smesh->volfire_texture_buffer==NULL){
    NewMemory((void **)&smesh->volfire_texture_buffer, nx*ny*nz*sizeof(float));
  }
  for(i = 0;i<nx*ny*nz;i++){
    smesh->volfire_texture_buffer[i] = 0.0;
  }
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F,
    nx, ny, nz, border_size,
    GL_RED, GL_FLOAT, smesh->volfire_texture_buffer);

  if(volsmoke_colormap_id_defined==-1){
    volsmoke_colormap_id_defined = 1;
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &volsmoke_colormap_id);
    glBindTexture(GL_TEXTURE_1D, volsmoke_colormap_id);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, MAXSMOKERGB, 0, GL_RGBA, GL_FLOAT, rgb_volsmokecolormap);
  }

  nx = smesh->ibar;
  ny = smesh->jbar;
  nz = smesh->kbar;
  if(smesh->f_iblank_cell != NULL){
    glActiveTexture(GL_TEXTURE3);
    if(smesh->blockage_texture_id == 0)glGenTextures(1, &smesh->blockage_texture_id);
    glBindTexture(GL_TEXTURE_3D, smesh->blockage_texture_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, border_size, GL_RED, GL_FLOAT, smesh->f_iblank_cell);
  }
  glActiveTexture(GL_TEXTURE0);
  FFLUSH();
}
#endif

/* ------------------ IsBottomMesh ------------------------ */

int IsBottomMesh(meshdata *mesh_from){
  float xyz[3];
  float *boxmax, *boxmin;
  int return_val;

  boxmin = mesh_from->boxmin;
  boxmax = mesh_from->boxmax;
  xyz[0] = (boxmin[0]+boxmax[0])/2.0;
  xyz[1] = (boxmin[1]+boxmax[1])/2.0;
  xyz[2] = boxmin[2]-(boxmax[2]-boxmin[2])/100.0;
  return_val = 1-InMesh(xyz);
  return return_val;
}

/* ------------------ MeshConnect ------------------------ */

int MeshConnect(meshdata *mesh_from, int val, meshdata *mesh_to){
  float *eps;

  //  returns 1 if mesh_from  is 'val' of mesh_to (where val is MLEFT, MRIGHT, MFRONT, MBACK, MDOWN, MBACK )

  eps = mesh_from->boxeps;
  switch(val){
  case MLEFT:
  case MRIGHT:
    if(mesh_from->jbar!=mesh_to->jbar)return 0;
    if(mesh_from->kbar!=mesh_to->kbar)return 0;
    if(ABS(mesh_from->dbox[1]-mesh_to->dbox[1])>eps[1])return 0;
    if(ABS(mesh_from->dbox[2]-mesh_to->dbox[2])>eps[2])return 0;
    if(ABS(mesh_from->y0-mesh_to->y0)>eps[1])return 0;
    if(ABS(mesh_from->z0-mesh_to->z0)>eps[2])return 0;
    break;
  case MFRONT:
  case MBACK:
    if(mesh_from->ibar!=mesh_to->ibar)return 0;
    if(mesh_from->kbar!=mesh_to->kbar)return 0;
    if(ABS(mesh_from->dbox[0]-mesh_to->dbox[0])>eps[0])return 0;
    if(ABS(mesh_from->dbox[2]-mesh_to->dbox[2])>eps[2])return 0;
    if(ABS(mesh_from->x0-mesh_to->x0)>eps[0])return 0;
    if(ABS(mesh_from->z0-mesh_to->z0)>eps[2])return 0;
    break;
  case MDOWN:
  case MUP:
    if(mesh_from->ibar!=mesh_to->ibar)return 0;
    if(mesh_from->jbar!=mesh_to->jbar)return 0;
    if(ABS(mesh_from->dbox[0]-mesh_to->dbox[0])>eps[0])return 0;
    if(ABS(mesh_from->dbox[1]-mesh_to->dbox[1])>eps[1])return 0;
    if(ABS(mesh_from->x0-mesh_to->x0)>eps[0])return 0;
    if(ABS(mesh_from->y0-mesh_to->y0)>eps[1])return 0;
    break;
  default:
    break;
  }
  switch(val){
  case MLEFT:
    if(ABS(mesh_from->x1-mesh_to->x0)<eps[0])return 1;
    break;
  case MRIGHT:
    if(ABS(mesh_from->x0-mesh_to->x1)<eps[0])return 1;
    break;
  case MFRONT:
    if(ABS(mesh_from->y1-mesh_to->y0)<eps[1])return 1;
    break;
  case MBACK:
    if(ABS(mesh_from->y0-mesh_to->y1)<eps[1])return 1;
    break;
  case MDOWN:
    if(ABS(mesh_from->z1-mesh_to->z0)<eps[2])return 1;
    break;
  case MUP:
    if(ABS(mesh_from->z0-mesh_to->z1)<eps[2])return 1;
    break;
  default:
    break;
  }
  return 0;
}

/* ------------------ GetMinMesh ------------------------ */

meshdata *GetMinMesh(void){
  int i;
  float mindist = -1.0;
  meshdata *minmesh = NULL;

  // find mesh closes to origin that is not already in a supermesh

  for(i = 0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    float dist2;

    meshi = global_scase.meshescoll.meshinfo+i;
    if(meshi->super!=NULL)continue;
    dist2 = meshi->x0*meshi->x0+meshi->y0*meshi->y0+meshi->z0*meshi->z0;
    if(mindist<0.0||dist2<mindist){
      mindist = dist2;
      minmesh = meshi;
    }
  }
  return minmesh;
}

/* ------------------ ExtendMesh ------------------------ */

int ExtendMesh(supermeshdata *smesh, int direction){
  int i;
  int count = 0, nbefore;

  nbefore = smesh->nmeshes;
  for(i = 0;i<nbefore;i++){
    meshdata *nabor;

    nabor = smesh->meshes[i]->nabors[direction];
    if(nabor!=NULL&&nabor->super!=NULL)continue;
    if(nabor==NULL)return 0;
  }
  for(i = 0;i<nbefore;i++){
    meshdata *nabor;

    nabor = smesh->meshes[i]->nabors[direction];
    if(nabor->super!=NULL)continue;
    smesh->meshes[nbefore+count] = nabor;
    nabor->super = smesh;
    count++;
  }
  if(count==0)return 0;
  smesh->nmeshes = nbefore+count;
  return 1;
}

/* ------------------ MakeSMesh ------------------------ */

void MakeSMesh(supermeshdata *smesh, meshdata *firstmesh){
  meshdata **meshptrs;

  NewMemory((void **)&meshptrs, global_scase.meshescoll.nmeshes*sizeof(meshdata *));
  smesh->meshes = meshptrs;

  smesh->meshes[0] = firstmesh;
  firstmesh->super = smesh;
  smesh->nmeshes = 1;
  for(;;){
    int return_val, again;

    again = 0;
    return_val = ExtendMesh(smesh, MLEFT);
    again = MAX(again, return_val);
    return_val = ExtendMesh(smesh, MRIGHT);
    again = MAX(again, return_val);
    return_val = ExtendMesh(smesh, MFRONT);
    again = MAX(again, return_val);
    return_val = ExtendMesh(smesh, MBACK);
    again = MAX(again, return_val);
    return_val = ExtendMesh(smesh, MUP);
    again = MAX(again, return_val);
    return_val = ExtendMesh(smesh, MDOWN);
    again = MAX(again, return_val);
    if(again==0)break;
  }
}

/* ------------------ CompareSMeshes ------------------------ */

int CompareSMeshes(const void *arg1, const void *arg2){
  meshdata *meshi, *meshj;
  float dcell;

  meshi = *(meshdata **)arg1;
  meshj = *(meshdata **)arg2;
  dcell = MIN(meshi->dcell, meshj->dcell)/2.0;
  if(meshi->z0<meshj->z0-dcell)return -1;
  if(meshi->z0>meshj->z0+dcell)return 1;
  if(meshi->y0<meshj->y0-dcell)return -1;
  if(meshi->y0>meshj->y0+dcell)return 1;
  if(meshi->x0<meshj->x0-dcell)return -1;
  if(meshi->x0>meshj->x0+dcell)return 1;
  return 0;
}

/* ------------------ SetSuperIndex ------------------------ */

void SetSuperIndex(meshdata *meshi, int dir){
  meshdata *nab;
  int index;

  if(meshi->s_offset[dir]>=0)return;
  nab = meshi->nabors[dir];
  if(nab==NULL||nab->super!=meshi->super){
    meshi->s_offset[dir] = 0;
    return;
  }
  SetSuperIndex(nab, dir);
  index = nab->s_offset[dir];
  if(dir==MLEFT)index += nab->ibar;
  if(dir==MFRONT)index += nab->jbar;
  if(dir==MDOWN)index += nab->kbar;
  meshi->s_offset[dir] = index;
}

/* ------------------ InitNabors ------------------------ */

void *InitNabors(void *arg){
  int i;

  INIT_PRINT_TIMER(timer_init_nabors);
  if(global_scase.have_mesh_nabors == 0){
    for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      int j;

      meshi = global_scase.meshescoll.meshinfo + i;
      meshi->is_bottom = IsBottomMesh(meshi);
      for(j = 0;j < global_scase.meshescoll.nmeshes;j++){
        meshdata *meshj;

        if(i == j)continue;

        meshj = global_scase.meshescoll.meshinfo + j;

        if(MeshConnect(meshi, MLEFT, meshj) == 1){
          meshi->nabors[MRIGHT] = meshj;
          continue;
        }
        if(MeshConnect(meshi, MRIGHT, meshj) == 1){
          meshi->nabors[MLEFT] = meshj;
          continue;
        }
        if(MeshConnect(meshi, MFRONT, meshj) == 1){
          meshi->nabors[MBACK] = meshj;
          continue;
        }
        if(MeshConnect(meshi, MBACK, meshj) == 1){
          meshi->nabors[MFRONT] = meshj;
          continue;
        }
        if(MeshConnect(meshi, MDOWN, meshj) == 1){
          meshi->nabors[MUP] = meshj;
          continue;
        }
        if(MeshConnect(meshi, MUP, meshj) == 1){
          meshi->nabors[MDOWN] = meshj;
          continue;
        }
      }
    }
  }
  for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    float xyzmid[3], xyz[3];

    meshi = global_scase.meshescoll.meshinfo + i;
    memcpy(xyzmid, meshi->boxmiddle, 3*sizeof(float));

    memcpy(xyz, xyzmid, 3*sizeof(float));
    xyz[0] = meshi->boxmin[0]- meshi->boxeps_fds[0];
    meshi->skip_nabors[MLEFT] = GetMesh(xyz);

    memcpy(xyz, xyzmid, 3*sizeof(float));
    xyz[0] = meshi->boxmax[0] + meshi->boxeps_fds[0];
    meshi->skip_nabors[MRIGHT] = GetMesh(xyz);

    memcpy(xyz, xyzmid, 3*sizeof(float));
    xyz[1] = meshi->boxmin[1] - meshi->boxeps_fds[1];
    meshi->skip_nabors[MFRONT] = GetMesh(xyz);

    memcpy(xyz, xyzmid, 3*sizeof(float));
    xyz[1] = meshi->boxmax[0] + meshi->boxeps_fds[1];
    meshi->skip_nabors[MBACK] = GetMesh(xyz);

    memcpy(xyz, xyzmid, 3*sizeof(float));
    xyz[2] = meshi->boxmin[2] - meshi->boxeps_fds[2];
    meshi->skip_nabors[MDOWN] = GetMesh(xyz);

    memcpy(xyz, xyzmid, 3*sizeof(float));
    xyz[2] = meshi->boxmax[2] + meshi->boxeps_fds[2];
    meshi->skip_nabors[MUP] = GetMesh(xyz);
  }
  PRINT_TIMER(timer_init_nabors, "InitNabors");
  THREAD_EXIT(meshnabors_threads);
}

/* ------------------ InitSuperMesh ------------------------ */

void InitSuperMesh(void){
  int i;
  meshdata *thismesh;
  supermeshdata *smesh;

  // determine mesh connectivity

  THREADcontrol(meshnabors_threads, THREAD_JOIN);

  // merge connected meshes to form supermeshes

  global_scase.nsupermeshinfo = 0;
  for(smesh = global_scase.supermeshinfo, thismesh = GetMinMesh();thismesh!=NULL;thismesh = GetMinMesh(), smesh++){
    MakeSMesh(smesh, thismesh);
    global_scase.nsupermeshinfo++;
  }

  for(smesh = global_scase.supermeshinfo;smesh!=global_scase.supermeshinfo+global_scase.nsupermeshinfo;smesh++){
    meshdata *nab;
    float *smin, *smax;
    int nsize;

    smin = smesh->boxmin_scaled;
    smax = smesh->boxmax_scaled;

    for(i = 0;i<smesh->nmeshes;i++){
      int j;
      float *bmin, *bmax;

      bmin = smesh->meshes[i]->boxmin_scaled;
      bmax = smesh->meshes[i]->boxmax_scaled;
      if(i==0){
        memcpy(smin, bmin, 3*sizeof(float));
        memcpy(smax, bmax, 3*sizeof(float));
      }
      else{
        for(j = 0;j<3;j++){
          smin[j] = MIN(smin[j], bmin[j]);
          smax[j] = MAX(smax[j], bmax[j]);
        }
      }
    }

    smesh->volfire_texture_buffer = NULL;
    smesh->volsmoke_texture_buffer = NULL;

    smesh->volfire_texture_id = 0;
    smesh->volsmoke_texture_id = 0;

    smesh->blockage_texture_id = 0;

    // sort meshes in supermesh from lower front left to upper back right

    if(nvolrenderinfo>1){
      qsort((meshdata **)smesh->meshes, smesh->nmeshes, sizeof(meshdata *), CompareSMeshes);
    }

    // count meshes in supermesh in each direction

    smesh->ibar = smesh->meshes[0]->ibar;
    smesh->jbar = smesh->meshes[0]->jbar;
    smesh->kbar = smesh->meshes[0]->kbar;
    for(nab = smesh->meshes[0];nab->nabors[MRIGHT]!=NULL;nab = nab->nabors[MRIGHT]){
      smesh->ibar += nab->ibar;
    }
    for(nab = smesh->meshes[0];nab->nabors[MBACK]!=NULL;nab = nab->nabors[MBACK]){
      smesh->jbar += nab->jbar;
    }
    for(nab = smesh->meshes[0];nab->nabors[MUP]!=NULL;nab = nab->nabors[MUP]){
      smesh->kbar += nab->kbar;
    }

    // determine if a mesh side is exterior to a supermesh

    for(i = 0;i<smesh->nmeshes;i++){
      meshdata *meshi;
      int *extsides;
      int j;
      meshdata **nabors;

      meshi = smesh->meshes[i];
      extsides = meshi->extsides;
      nabors = meshi->nabors;
      for(j = 0;j<7;j++){
        extsides[j] = 0;
      }
      if(nabors[MLEFT]==NULL||nabors[MLEFT]->super!=meshi->super)extsides[2] = 1;
      if(nabors[MRIGHT]==NULL||nabors[MRIGHT]->super!=meshi->super)extsides[4] = 1;
      if(nabors[MFRONT]==NULL||nabors[MFRONT]->super!=meshi->super)extsides[1] = 1;
      if(nabors[MBACK]==NULL||nabors[MBACK]->super!=meshi->super)extsides[5] = 1;
      if(nabors[MDOWN]==NULL||nabors[MDOWN]->super!=meshi->super)extsides[0] = 1;
      if(nabors[MUP]==NULL||nabors[MUP]->super!=meshi->super)extsides[6] = 1;
      SetSuperIndex(meshi, MLEFT);
      SetSuperIndex(meshi, MFRONT);
      SetSuperIndex(meshi, MDOWN);
    }
    nsize = (smesh->ibar+1)*(smesh->jbar+1)*(smesh->kbar+1);
    NEWMEMORY(smesh->f_iblank_cell, nsize*sizeof(float));
    for(i = 0;i<nsize;i++){
      smesh->f_iblank_cell[i] = (float)GAS;
    }
  }
#ifdef pp_GPU
  if(gpuactive==1){
    for(i = 0;i<global_scase.nsupermeshinfo;i++){
      smesh = global_scase.supermeshinfo+i;
      InitVolsmokeSuperTexture(smesh);
    }
  }
#endif
}

/* ------------------ GetVolsmokeNFrames ------------------------ */

int GetVolsmokeNFrames(volrenderdata *vr){
  slicedata *smokeslice;
  FILE *volstream = NULL;
  int framesize;
  LINT skip_local;
  int nframes;
  FILE_SIZE filesize;

  smokeslice = vr->smokeslice;
  if(load_volcompressed==1&&vr->smokeslice->vol_file!=NULL){
    volstream = fopen(vr->smokeslice->vol_file, "rb");
  }
  if(volstream==NULL){
    framesize = smokeslice->nslicei*smokeslice->nslicej*smokeslice->nslicek;
    framesize *= 4; // convert to bytes
    framesize += HEADER_SIZE+TRAILER_SIZE;

    skip_local = (HEADER_SIZE+30+TRAILER_SIZE); // long label
    skip_local += (HEADER_SIZE+30+TRAILER_SIZE); // short label
    skip_local += (HEADER_SIZE+30+TRAILER_SIZE); // unit label
    skip_local += (HEADER_SIZE+24+TRAILER_SIZE); // is1, is2, js1, js2, ks1, ks2

                                                 // nframes = (totalsize - skip_local)/(12 + framesize);

    nframes = 0;
    filesize = GetFileSizeSMV(smokeslice->reg_file);
    if(filesize>0){
      nframes = (int)(filesize-skip_local)/(int)(12+framesize);
    }
  }
  else{
    unsigned char buffer[32];
    // 1,completion,version
    // 1,version,n_data_compressedm32,nbytes,n_data_in,time,valmin,valmax,data ....
    FSEEK(volstream, 12, SEEK_SET);
    for(nframes = 0;;nframes++){
      int ncompressed;

      if(fread(buffer, 1, 32, volstream)!=32)break;
      ncompressed = *(int *)(buffer+8)-32;
      if(FSEEK(volstream, ncompressed, SEEK_CUR)!=0)break;
    }
    fclose(volstream);
  }
  return nframes;
}

/* ------------------ InitVolRender ------------------------ */

void InitVolRender(void){
  int i;

  nvolrenderinfo=0;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + i;
    vr = meshi->volrenderinfo;
    vr->rendermeshlabel=meshi->label;

    vr->fireslice=NULL;
    vr->smokeslice=NULL;

    vr->firepos = NULL;
    vr->smokepos = NULL;

    vr->timeslist=NULL;

    vr->loaded=0;
    vr->display=0;
    vr->is_compressed=0;
    vr->times_defined=0;
  }
  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *slicei;
    char *shortlabel, *longlabel;
    int blocknumber;
    meshdata *meshi;
    volrenderdata *vr;

    slicei = global_scase.slicecoll.sliceinfo + i;
    blocknumber = slicei->blocknumber;
    if(blocknumber<0||blocknumber>=global_scase.meshescoll.nmeshes)continue;
    shortlabel = slicei->label.shortlabel;
    longlabel = slicei->label.longlabel;
    if(STRCMP(shortlabel, "temp")!=0&&IsSootFile(shortlabel, longlabel)==0&&STRCMP(shortlabel, "frac")!=0&&STRCMP(shortlabel, "X_CO2")!=0)continue;
    if(slicei->full_mesh==NO)continue;
    if(FILE_EXISTS(slicei->reg_file)==NO)continue;

    meshi = global_scase.meshescoll.meshinfo + blocknumber;

    vr = meshi->volrenderinfo;

    if(STRCMP(shortlabel,"temp")==0){
      vr->fireslice=slicei;
     continue;
    }
    if(IsSootFile(shortlabel, longlabel)==1){
      vr->smokeslice=slicei;
      continue;
    }
  }
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + i;
    vr = meshi->volrenderinfo;
    vr->ntimes=0;

    vr->firedata_full=NULL;
    vr->smokedata_full=NULL;

    vr->c_firedata_view=NULL;
    vr->c_smokedata_view=NULL;

    if(vr->smokeslice!=NULL){
      int j;

      nvolrenderinfo++;
      vr->ntimes=GetVolsmokeNFrames(vr);
      if(vr->ntimes>0){
        NewMemory((void **)&vr->times,vr->ntimes*sizeof(float));
        NewMemory((void **)&vr->dataready, vr->ntimes*sizeof(int));

        NewMemory((void **)&vr->firepos,vr->ntimes*sizeof(LINT));
        NewMemory((void **)&vr->smokepos,vr->ntimes*sizeof(LINT));

        NewMemory((void **)&vr->firedataptrs,vr->ntimes*sizeof(float *));
        NewMemory((void **)&vr->smokedataptrs,vr->ntimes*sizeof(float *));

        NewMemory((void **)&vr->nfiredata_compressed,vr->ntimes*sizeof(int));
        NewMemory((void **)&vr->nsmokedata_compressed,vr->ntimes*sizeof(int));

        vr->firedataptr = NULL;
        vr->smokedataptr = NULL;

        for(j=0;j<vr->ntimes;j++){
          vr->firedataptrs[j]  = NULL;
          vr->smokedataptrs[j] = NULL;

          vr->nfiredata_compressed[j]  = 0;
          vr->nsmokedata_compressed[j] = 0;

          vr->dataready[j]=0;
        }
      }
    }
  }
  if(nvolrenderinfo>0){
    NewMemory((void **)&volfacelistinfo,6*global_scase.meshescoll.nmeshes*sizeof(volfacelistdata));
    NewMemory((void **)&volfacelistinfoptrs,6*global_scase.meshescoll.nmeshes*sizeof(volfacelistdata *));
  }
  if(nvolrenderinfo>0){
    InitSuperMesh();
  }
}

/* ------------------ GetMeshInSmesh ------------------------ */

meshdata *GetMeshInSmesh(meshdata *mesh_guess, supermeshdata *smesh, float *xyz){
  int i;
  float *smin, *smax;

  smin = smesh->boxmin_scaled;
  smax = smesh->boxmax_scaled;

  if(xyz[0]<smin[0]||xyz[1]<smin[1]||xyz[2]<smin[2])return NULL;
  if(xyz[0]>smax[0]||xyz[1]>smax[1]||xyz[2]>smax[2])return NULL;
  for(i = -1; i<smesh->nmeshes; i++){
    meshdata *meshi;
    float *bmin, *bmax;

    if(i==-1){
      if(mesh_guess==NULL)continue;
      meshi = mesh_guess;
    }
    else{
      meshi = smesh->meshes[i];
      if(meshi==mesh_guess)continue;
    }

    bmin = meshi->boxmin_scaled;
    bmax = meshi->boxmax_scaled;

    if(
      bmin[0]<=xyz[0]&&xyz[0]<=bmax[0]&&
      bmin[1]<=xyz[1]&&xyz[1]<=bmax[1]&&
      bmin[2]<=xyz[2]&&xyz[2]<=bmax[2]){
      return meshi;
    }
  }
  assert(FFALSE);
  return NULL;
}

/* ------------------ IntegrateFireColors ------------------------ */

void IntegrateFireColors(float *integrated_firecolor, float *xyzvert, float dlength, meshdata *meshi, int iwall){
  float t_intersect_min, *boxmin, *boxmax;
  int i;
  int nsteps;
  float dxyz[3];
  float distseg, dxseg, dyseg, dzseg;
  float xyz[3];
  float *vert_beg=NULL, *vert_end=NULL;
  int iwall_min=0;
  float xyzvals[3];
  char *blank_local;
  float taun, alphan;
  meshdata *xyz_mesh=NULL;

  if(combine_meshes==1){
    boxmin = meshi->super->boxmin_scaled;
    boxmax = meshi->super->boxmax_scaled;
  }
  else{
    boxmin = meshi->boxmin_scaled;
    boxmax = meshi->boxmax_scaled;
  }

  if(meshi->inside==1){
    vert_beg=eye_position_smv;
    vert_end=xyzvert;
  }
  else{
    int first;

    vert_beg=xyzvert;
    vert_end=xyzvals;

    first = 1;
    dxyz[0] = xyzvert[0] - eye_position_smv[0];
    dxyz[1] = xyzvert[1] - eye_position_smv[1];
    dxyz[2] = xyzvert[2] - eye_position_smv[2];
    for(i=1;i<4;i++){
      int ii;

      // xyz(t) = xyzvert + t*(xyzvert - eye_position_smv )
      // xyzvert_ii + t*(xyzvert_ii - eye_position_smv_ii) = boxmin_ii
      // t = (xyzvert_ii - boxmin_ii)/(xyzvert_ii-eye_position_smv_ii)
      //
      // integrate from t=0 to t=t_intersect_min  (if outside mesh)
      //     ie from vertex to nearest wall along a line from the eye position
      //        intersecting the vertex position
      // integrate from t=-1 to t=0 (if inside mesh)
      //     ie from the eye position to the vertex position
      ii=i-1;
      float t_intersect;

      if(iwall!=-i&&dxyz[ii]<0.0){
        t_intersect = (boxmin[ii]-xyzvert[ii])/dxyz[ii];
        if(first==1||t_intersect<t_intersect_min){
          first = 0;
          t_intersect_min=t_intersect;
          iwall_min=-i;
        }
      }
      if(iwall!=i&&dxyz[ii]>0.0){
        t_intersect = (boxmax[ii]-xyzvert[ii])/dxyz[ii];
        if(first==1||t_intersect<t_intersect_min){
          first = 0;
          t_intersect_min=t_intersect;
          iwall_min=i;
        }
      }
    }
    vert_end[0] = 0.0;
    vert_end[1] = 0.0;
    vert_end[2] = 0.0;
    switch(iwall_min){
      case XWALLMIN:
        vert_end[0] = boxmin[0];
        vert_end[1] = CLAMP(xyzvert[1] + t_intersect_min*dxyz[1], boxmin[1], boxmax[1]);
        vert_end[2] = CLAMP(xyzvert[2] + t_intersect_min*dxyz[2], boxmin[2], boxmax[2]);
        break;
      case XWALLMAX:
        vert_end[0] = boxmax[0];
        vert_end[1] = CLAMP(xyzvert[1] + t_intersect_min* dxyz[1],boxmin[1],boxmax[1]);
        vert_end[2] = CLAMP(xyzvert[2] + t_intersect_min* dxyz[2],boxmin[2],boxmax[2]);
        break;
      case YWALLMIN:
        vert_end[0] = CLAMP(xyzvert[0] + t_intersect_min* dxyz[0],boxmin[0],boxmax[0]);
        vert_end[1] = boxmin[1];
        vert_end[2] = CLAMP(xyzvert[2] + t_intersect_min* dxyz[2],boxmin[2],boxmax[2]);
        break;
      case YWALLMAX:
        vert_end[0] = CLAMP(xyzvert[0] + t_intersect_min* dxyz[0],boxmin[0],boxmax[0]);
        vert_end[1] = boxmax[1];
        vert_end[2] = CLAMP(xyzvert[2] + t_intersect_min* dxyz[2],boxmin[2],boxmax[2]);
        break;
      case ZWALLMIN:
        vert_end[0] = CLAMP(xyzvert[0] + t_intersect_min* dxyz[0],boxmin[0],boxmax[0]);
        vert_end[1] = CLAMP(xyzvert[1] + t_intersect_min* dxyz[1],boxmin[1],boxmax[1]);
        vert_end[2] = boxmin[2];
        break;
      case ZWALLMAX:
        vert_end[0] = CLAMP(xyzvert[0] + t_intersect_min* dxyz[0],boxmin[0],boxmax[0]);
        vert_end[1] = CLAMP(xyzvert[1] + t_intersect_min* dxyz[1],boxmin[1],boxmax[1]);
        vert_end[2] = boxmax[2];
        break;
      default:
        assert(FFALSE);
        break;
    }
  }

  dxseg = vert_end[0] - vert_beg[0];
  dyseg = vert_end[1] - vert_beg[1];
  dzseg = vert_end[2] - vert_beg[2];
  distseg = sqrt(dxseg*dxseg+dyseg*dyseg+dzseg*dzseg);
  integrated_firecolor[0] = 0.0;
  integrated_firecolor[1] = 0.0;
  integrated_firecolor[2] = 0.0;
  integrated_firecolor[3] = 0.0;

  if(distseg<0.001)return;

  nsteps = 2*distseg/dlength;
  if(nsteps<1)nsteps=1;
  dlength=SCALE2FDS(distseg/(float)nsteps);
  taun=1.0;
  alphan=0.0;
  for(i=0;i<nsteps;i++){
    int inobst;
    float factor;
    float fire_emission[3];
    float alphai, taui, xi;

    xi = 0.5 + (float)i;
    factor = xi/(float)nsteps;
    xyz[0] = factor*vert_end[0] + (1.0 - factor)*vert_beg[0];
    xyz[1] = factor*vert_end[1] + (1.0 - factor)*vert_beg[1];
    xyz[2] = factor*vert_end[2] + (1.0 - factor)*vert_beg[2];

    if(combine_meshes==1){
      xyz_mesh = GetMeshInSmesh(xyz_mesh,meshi->super,xyz);
      if(xyz_mesh==NULL)break;
      blank_local = NULL;
      if(block_volsmoke==1)blank_local=xyz_mesh->c_iblank_cell;
      GetFireEmission(&taui, fire_emission, dlength, xyz, xyz_mesh, &inobst, blank_local);
    }
    else{
      blank_local = NULL;
      if(block_volsmoke==1)blank_local=meshi->c_iblank_cell;
      GetFireEmission(&taui, fire_emission, dlength, xyz, meshi, &inobst, blank_local);
    }
    if(blank_local!=NULL&&inobst==1)break; // terminate ray when a blockage is encountered

//         taui = exp(-mass_extinct*soot_val*dstep);
//         new_color = taui*current_color + old_color

    taun *= taui;
    alphai = 1.0 - taui;
    alphan = 1.0 - taun;

    if(hrrpuv_max_blending==1){
      integrated_firecolor[0] = MAX(integrated_firecolor[0], fire_emission[0]);
      integrated_firecolor[1] = MAX(integrated_firecolor[1], fire_emission[1]);
      integrated_firecolor[2] = MAX(integrated_firecolor[2], fire_emission[2]);
    }
    // https://developer.nvidia.com/sites/all/modules/custom/gpugems/books/GPUGems/gpugems_ch39.html
    // equation 6 - integrate forward
    else{
      integrated_firecolor[0] += taun*alphai*fire_emission[0];
      integrated_firecolor[1] += taun*alphai*fire_emission[1];
      integrated_firecolor[2] += taun*alphai*fire_emission[2];
    }
  }

  if(alphan>0.0){
    alphan = CLAMP(alphan, 0.0, 1.0);
    float max_rgb;

    max_rgb = MAX(integrated_firecolor[0], MAX(integrated_firecolor[1], integrated_firecolor[2]));
    if(max_rgb>1.0){
      integrated_firecolor[0] /= max_rgb;
      integrated_firecolor[1] /= max_rgb;
      integrated_firecolor[2] /= max_rgb;
    }

 //   float fire_rgb_from[3], fire_rgb_to[3];
 //   memcpy(fire_rgb_from, integrated_firecolor, 3*sizeof(float));
 //   Xyz2Rgb(&HDTVsystem, fire_rgb_from, fire_rgb_to);
 //   ConstrainRgb(fire_rgb_to);

 //    memcpy(integrated_firecolor, fire_rgb_to, 3*sizeof(float));
    integrated_firecolor[3] = alphan;
    if(volbw==1){
      float gray;

      gray = TOBW(integrated_firecolor);
      integrated_firecolor[0] = gray;
      integrated_firecolor[1] = gray;
      integrated_firecolor[2] = gray;
    }
  }
  else{
    VEC4EQCONS(integrated_firecolor,0.0);
  }
}

/* ------------------ ComputeAllSmokecolors ------------------------ */

void ComputeAllSmokecolors(void){
  int ii;

  if(freeze_volsmoke==1)return;
  for(ii=0;ii<global_scase.meshescoll.nmeshes;ii++){
    meshdata *meshi;
    volrenderdata *vr;
    int iwall;
    float dlength;
    float dx, dy, dz;
    float *x, *y, *z;
    int ibar, jbar, kbar;
    float *smokecolor;

    meshi = global_scase.meshescoll.meshinfo + ii;
    vr = meshi->volrenderinfo;
    if(vr->loaded==0||vr->display==0)continue;

    x = meshi->xvolplt;
    y = meshi->yvolplt;
    z = meshi->zvolplt;
    ibar = meshi->ivolbar;
    jbar = meshi->jvolbar;
    kbar = meshi->kvolbar;
    dx = x[1] - x[0];
    dy = y[1] - y[0];
    dz = z[1] - z[0];
    dlength = sqrt(dx*dx+dy*dy+dz*dz)/2.0;

    if(vr->smokeslice==NULL)continue;
    for(iwall=-3;iwall<=3;iwall++){
      float *xyz,xyzarray[3];
      int i, j;

      xyz = xyzarray;
      if(iwall==0||meshi->drawsides[iwall+3]==0)continue;
      switch(iwall){
        case XWALLMIN:
        case XWALLMAX:
          if(iwall<0){
            smokecolor=vr->smokecolor_yz0;
            xyz[0] = meshi->x0;
          }
          else{
            smokecolor=vr->smokecolor_yz1;
            xyz[0] = meshi->x1;
          }
          if(vr->firedataptr==NULL||vr->smokedataptr==NULL){
            for(i=0;i<=jbar;i++){
              for(j=0;j<=kbar;j++){
                smokecolor[0]=0.0;
                smokecolor[1]=0.0;
                smokecolor[2]=0.0;
                smokecolor[3]=0.0;
                smokecolor+=4;
              }
            }
          }
          else{
            for(i=0;i<=jbar;i++){
              xyz[1] = y[i];
              for(j=0;j<=kbar;j++){
                xyz[2] = z[j];
                IntegrateFireColors(smokecolor,xyz,dlength,meshi,iwall);
                smokecolor+=4;
              }
            }
          }
          break;
        case YWALLMIN:
        case YWALLMAX:
          if(iwall<0){
            smokecolor=vr->smokecolor_xz0;
            xyz[1] = meshi->y0;
          }
          else{
            smokecolor=vr->smokecolor_xz1;
            xyz[1] = meshi->y1;
          }
          if(vr->firedataptr==NULL||vr->smokedataptr==NULL){
            for(i=0;i<=ibar;i++){
              for(j=0;j<=kbar;j++){
                smokecolor[0]=0.0;
                smokecolor[1]=0.0;
                smokecolor[2]=0.0;
                smokecolor[3]=0.0;
                smokecolor+=4;
              }
            }
          }
          else{
            for(i=0;i<=ibar;i++){
              xyz[0] = x[i];
              for(j=0;j<=kbar;j++){
                xyz[2] = z[j];
                IntegrateFireColors(smokecolor,xyz,dlength,meshi,iwall);
                smokecolor+=4;
              }
            }
          }
          break;
        case ZWALLMIN:
        case ZWALLMAX:
          if(iwall<0){
            smokecolor=vr->smokecolor_xy0;
            xyz[2]=meshi->z0;
          }
          else{
            smokecolor=vr->smokecolor_xy1;
            xyz[2]=meshi->z1;
          }
          if(vr->firedataptr==NULL||vr->smokedataptr==NULL){
            for(i=0;i<=ibar;i++){
              for(j=0;j<=jbar;j++){
                smokecolor[0]=0.0;
                smokecolor[1]=0.0;
                smokecolor[2]=0.0;
                smokecolor[3]=0.0;
                smokecolor+=4;
              }
            }
          }
          else{
            for(i=0;i<=ibar;i++){
              xyz[0] = x[i];
              for(j=0;j<=jbar;j++){
                xyz[1] = y[j];
                IntegrateFireColors(smokecolor,xyz,dlength,meshi,iwall);
                smokecolor+=4;
              }
            }
          }
          break;
        default:
          assert(FFALSE);
          break;
      }
    }
  }
}

  /* ------------------ DrawSmoke3dVolDebug ------------------------ */

void DrawSmoke3dVolDebug(void){
  int ii;

  for(ii=0;ii<nvolfacelistinfo;ii++){
    volfacelistdata *vi;
    meshdata *meshi;
    float x[2], y[2], z[2];
    float *xplt, *yplt, *zplt;
    int ibar, jbar, kbar;
    char label[256];
    int iwall;


    vi = volfacelistinfoptrs[ii];
    sprintf(label,"*** %i %2.1f ***",ii,vi->dist2);
    iwall=vi->iwall;
    meshi = vi->facemesh;
    xplt = meshi->xplt;
    yplt = meshi->yplt;
    zplt = meshi->zplt;
    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;

    if(iwall==0||meshi->drawsides[iwall+3]==0)continue;
    switch(iwall){
      case XWALLMIN:
      case XWALLMAX:
        if(iwall<0){
          x[0] = meshi->x0;
          x[1] = x[0];
        }
        else{
          x[0]=meshi->x1;
          x[1]=x[0];
        }
        y[0] = yplt[0];
        y[1] = yplt[jbar];
        z[0] = zplt[0];
        z[1] = zplt[kbar];
        Output3Text(foregroundcolor, (x[0]+x[1])/2.0,(y[0]+y[1])/2.0,(z[0]+z[1])/2.0, label);
        break;
      case YWALLMIN:
      case YWALLMAX:
        if(iwall<0){
          y[0] = meshi->y0;
          y[1] = y[0];
        }
        else{
          y[0] = meshi->y1;
          y[1] = y[0];
        }
        x[0] = xplt[0];
        x[1] = xplt[ibar];
        z[0] = zplt[0];
        z[1] = zplt[kbar];
        Output3Text(foregroundcolor, (x[0]+x[1])/2.0,(y[0]+y[1])/2.0,(z[0]+z[1])/2.0, label);
        break;
      case ZWALLMIN:
      case ZWALLMAX:
        if(iwall<0){
          z[0] = meshi->z0;
          z[1] = z[0];
        }
        else{
          z[0] = meshi->z1;
          z[1] = z[0];
        }
        x[0] = xplt[0];
        x[1] = xplt[ibar];
        y[0] = yplt[0];
        y[1] = yplt[jbar];
        Output3Text(foregroundcolor, (x[0]+x[1])/2.0,(y[0]+y[1])/2.0,(z[0]+z[1])/2.0, label);
        break;
      default:
        assert(FFALSE);
        break;
    }
  }
  glBegin(GL_LINES);
  for(ii=0;ii<nvolfacelistinfo;ii++){
    volfacelistdata *vi;
    meshdata *meshi;
    float x[2], y[2], z[2];
    float *xplt, *yplt, *zplt;
    int ibar, jbar, kbar;
    char label[256];
    int iwall;

    sprintf(label,"*** %i ***",ii);

    vi = volfacelistinfoptrs[ii];
    iwall=vi->iwall;
    meshi = vi->facemesh;
    xplt = meshi->xplt;
    yplt = meshi->yplt;
    zplt = meshi->zplt;
    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;

    if(iwall==0||meshi->drawsides[iwall+3]==0)continue;
    switch(iwall){
      case XWALLMIN:
      case XWALLMAX:
        if(iwall<0){
          x[0] = meshi->x0;
          glColor3f(1.0,0.0,0.0);
        }
        else{
          x[0]=meshi->x1;
          glColor3f(0.0,0.0,1.0);
        }
        y[0] = yplt[0];
        y[1] = yplt[jbar];
        z[0] = zplt[0];
        z[1] = zplt[kbar];
        glVertex3f(x[0],y[0],z[0]);
        glVertex3f(x[0],y[1],z[1]);
        glVertex3f(x[0],y[1],z[0]);
        glVertex3f(x[0],y[0],z[1]);
        break;
      case YWALLMIN:
      case YWALLMAX:
        if(iwall<0){
          y[0] = meshi->y0;
          glColor3f(1.0,0.0,0.0);
        }
        else{
          y[0] = meshi->y1;
          glColor3f(0.0,0.0,1.0);
        }
        x[0] = xplt[0];
        x[1] = xplt[ibar];
        z[0] = zplt[0];
        z[1] = zplt[kbar];
        glVertex3f(x[0],y[0],z[0]);
        glVertex3f(x[1],y[0],z[1]);
        glVertex3f(x[0],y[0],z[1]);
        glVertex3f(x[1],y[0],z[0]);
        break;
      case ZWALLMIN:
      case ZWALLMAX:
        if(iwall<0){
          z[0] = meshi->z0;
          glColor3f(1.0,0.0,0.0);
        }
        else{
          z[0] = meshi->z1;
          glColor3f(0.0,0.0,1.0);
        }
        x[0] = xplt[0];
        x[1] = xplt[ibar];
        y[0] = yplt[0];
        y[1] = yplt[jbar];
        glVertex3f(x[0],y[0],z[0]);
        glVertex3f(x[1],y[1],z[0]);
        glVertex3f(x[0],y[1],z[0]);
        glVertex3f(x[1],y[0],z[0]);
        break;
      default:
        assert(FFALSE);
        break;
    }
  }
  glEnd();
}

#define GETSMOKECOLORPTR(valptr, i, j, nj) (valptr + 4*((i)*(nj) + (j)))

#define DRAWXPLANE(v1,v2,v3) \
                glColor4fv(smokecolor + val_index[v1]); glVertex3f(xx, y[yindex[v1]], z[zindex[v1]]);\
                glColor4fv(smokecolor + val_index[v2]); glVertex3f(xx, y[yindex[v2]], z[zindex[v2]]);\
                glColor4fv(smokecolor + val_index[v3]); glVertex3f(xx, y[yindex[v3]], z[zindex[v3]])

#define DRAWYPLANE(v1,v2,v3) \
                glColor4fv(smokecolor + val_index[v1]); glVertex3f(x[xindex[v1]], yy, z[zindex[v1]]);\
                glColor4fv(smokecolor + val_index[v2]); glVertex3f(x[xindex[v2]], yy, z[zindex[v2]]);\
                glColor4fv(smokecolor + val_index[v3]); glVertex3f(x[xindex[v3]], yy, z[zindex[v3]])

#define DRAWZPLANE(v1,v2,v3) \
                glColor4fv(smokecolor + val_index[v1]); glVertex3f(x[xindex[v1]], y[yindex[v1]], zz);\
                glColor4fv(smokecolor + val_index[v2]); glVertex3f(x[xindex[v2]], y[yindex[v2]], zz);\
                glColor4fv(smokecolor + val_index[v3]); glVertex3f(x[xindex[v3]], y[yindex[v3]], zz)
/* ------------------ DrawSmoke3dVol ------------------------ */

void DrawSmoke3DVol(void){
  int iwall;
  int ii;

  if(use_transparency_data==1)TransparentOn();
  if(show_blackbody_colormap == 1){
    int i;

    glBegin(GL_QUADS);
    for(i = 0;i < nfire_colors;i++){
      float fire_rgb[3], zmin, zmax, maxrgb;

      memcpy(fire_rgb, fire_rgbs + 3*i, 3*sizeof(float));
      maxrgb = MAX(fire_rgb[0], fire_rgb[1]);
      maxrgb = MAX(maxrgb, fire_rgb[2]);
      if(maxrgb>0.0){
        fire_rgb[0] /= maxrgb;
        fire_rgb[1] /= maxrgb;
        fire_rgb[2] /= maxrgb;
      }

      glColor3fv(fire_rgb);
      zmin = (float)i / (float)nfire_colors;
      zmax = (float)(i + 1) / (float)nfire_colors;
      glVertex3f(0.9, 0.5, zmin);
      glVertex3f(1.0, 0.5, zmin);
      glVertex3f(1.0, 0.5, zmax);
      glVertex3f(0.9, 0.5, zmax);

      glVertex3f(0.9, 0.5, zmin);
      glVertex3f(0.9, 0.5, zmax);
      glVertex3f(1.0, 0.5, zmax);
      glVertex3f(1.0, 0.5, zmin);
    }
    glEnd();
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    glColor3fv(foregroundcolor);
    glVertex3f(0.9, 0.5, 0.0);
    glVertex3f(1.0, 0.5, 0.0);

    glVertex3f(1.0, 0.5, 0.0);
    glVertex3f(1.0, 0.5, 1.0);

    glVertex3f(1.0, 0.5, 1.0);
    glVertex3f(0.9, 0.5, 1.0);

    glVertex3f(0.9, 0.5, 1.0);
    glVertex3f(0.9, 0.5, 0.0);
    glEnd();
  }
  for(ii=0;ii<nvolfacelistinfo;ii++){
    volfacelistdata *vi;
    meshdata *meshi;
    volrenderdata *vr;
    int i,j;
    float xx, yy, zz;
    float x[2], y[2], z[2];
    int n00, n01, n10, n11;
    float *xplt, *yplt, *zplt;
    int ibar, jbar, kbar;
    float *smokecolor;
    int val_index[4];
    int xindex[4], yindex[4], zindex[4];

    vi = volfacelistinfoptrs[ii];
    iwall=vi->iwall;
    meshi = vi->facemesh;
    if(meshvisptr[meshi-global_scase.meshescoll.meshinfo]==0)continue;
    xplt = meshi->xvolplt;
    yplt = meshi->yvolplt;
    zplt = meshi->zvolplt;
    ibar = meshi->ivolbar;
    jbar = meshi->jvolbar;
    kbar = meshi->kvolbar;
    vr = meshi->volrenderinfo;

    if(iwall==0||meshi->drawsides[iwall+3]==0)continue;
    if(vr->firedataptr==NULL&&vr->smokedataptr==NULL)continue;

    glBegin(GL_TRIANGLES);
    switch(iwall){
      float *smokecolor_base;

      case XWALLMIN:
      case XWALLMAX:
        if(iwall<0){
          xx = meshi->x0;
          smokecolor_base = vr->smokecolor_yz0;
        }
        else{
          xx=meshi->x1;
          smokecolor_base = vr->smokecolor_yz1;
        }
        n00 = 0;
        n01 = 4;
        n10 = 4*(kbar+1);
        n11 = 4*(1 + kbar+1);
        val_index[0]=n00;
        val_index[1]=n10;
        val_index[2]=n11;
        val_index[3]=n01;
        yindex[0] = 0;
        yindex[1] = 1;
        yindex[2] = 1;
        yindex[3] = 0;
        zindex[0] = 0;
        zindex[1] = 0;
        zindex[2] = 1;
        zindex[3] = 1;
        for(i=0;i<jbar;i++){
          y[0] = yplt[i];
          y[1] = yplt[i+1];
          for(j=0;j<kbar;j++){
            smokecolor = GETSMOKECOLORPTR(smokecolor_base, i, j, kbar + 1);
            z[0] = zplt[j];
            z[1] = zplt[j+1];
            float *c00, *c01, *c10, *c11;
            c00 = smokecolor + n00;
            c01 = smokecolor + n01;
            c10 = smokecolor + n10;
            c11 = smokecolor + n11;
            if((meshi->inside==0&&iwall>0)||(meshi->inside!=0&&iwall<0)){
              if(ABS(c00[0] - c11[0]) <  ABS(c01[0] - c10[0])){
                DRAWXPLANE(0,2,3);
                DRAWXPLANE(0,1,2);
              }
              else{
                DRAWXPLANE(0,1,3);
                DRAWXPLANE(1,2,3);
              }
            }
            else{
              if(ABS(c00[0] - c11[0]) > ABS(c01[0] - c10[0])){
                DRAWXPLANE(0,3,1);
                DRAWXPLANE(1,3,2);
              }
              else{
                DRAWXPLANE(0,2,1);
                DRAWXPLANE(0,3,2);
              }
            }
          }
        }
        break;
      case YWALLMIN:
      case YWALLMAX:
        n00 = 0;
        n01 = 4;
        n10 = 4*(kbar+1);
        n11 = 4*(1 + kbar+1);
        val_index[0]=n00;
        val_index[1]=n10;
        val_index[2]=n11;
        val_index[3]=n01;
        xindex[0] = 0;
        xindex[1] = 1;
        xindex[2] = 1;
        xindex[3] = 0;
        zindex[0] = 0;
        zindex[1] = 0;
        zindex[2] = 1;
        zindex[3] = 1;
        if(iwall<0){
          smokecolor_base = vr->smokecolor_xz0;
          yy=meshi->y0;
        }
        else{
          smokecolor_base = vr->smokecolor_xz1;
          yy=meshi->y1;
        }
        for(i=0;i<ibar;i++){
          x[0] = xplt[i];
          x[1] = xplt[i+1];
          for(j=0;j<kbar;j++){
            smokecolor = GETSMOKECOLORPTR(smokecolor_base, i, j, kbar + 1);
            z[0] = zplt[j];
            z[1] = zplt[j+1];
            float *c00, *c01, *c10, *c11;
            c00 = smokecolor + n00;
            c01 = smokecolor + n01;
            c10 = smokecolor + n10;
            c11 = smokecolor + n11;
            if((meshi->inside==0&&iwall>0)||(meshi->inside!=0&&iwall<0)){
              if(ABS(c00[0] - c11[0]) >  ABS(c01[0] - c10[0])){
                DRAWYPLANE(0,3,1);
                DRAWYPLANE(1,3,2);
              }
              else{
                DRAWYPLANE(0,3,2);
                DRAWYPLANE(0,2,1);
              }
            }
            else{
              if(ABS(c00[0] - c11[0]) <  ABS(c01[0] - c10[0])){
                DRAWYPLANE(0,1,2);
                DRAWYPLANE(0,2,3);
              }
              else{
                DRAWYPLANE(0,1,3);
                DRAWYPLANE(1,2,3);
              }
            }
          }
        }
        break;
      case ZWALLMIN:
      case ZWALLMAX:
        n00 = 0;
        n01 = 4;
        n10 = 4*(jbar+1);
        n11 = 4*(1 + jbar+1);
        val_index[0]=n00;
        val_index[1]=n10;
        val_index[2]=n11;
        val_index[3]=n01;
        xindex[0] = 0;
        xindex[1] = 1;
        xindex[2] = 1;
        xindex[3] = 0;
        yindex[0] = 0;
        yindex[1] = 0;
        yindex[2] = 1;
        yindex[3] = 1;
       if(iwall<0){
          smokecolor_base = vr->smokecolor_xy0;
          zz=meshi->z0;
        }
        else{
          smokecolor_base = vr->smokecolor_xy1;
          zz=meshi->z1;
        }
        for(i=0;i<ibar;i++){
          x[0] = xplt[i];
          x[1] = xplt[i+1];
          for(j=0;j<jbar;j++){
            smokecolor = GETSMOKECOLORPTR(smokecolor_base, i, j, jbar + 1);
            y[0] = yplt[j];
            y[1] = yplt[j+1];
            float *c00, *c01, *c10, *c11;
            c00 = smokecolor + n00;
            c01 = smokecolor + n01;
            c10 = smokecolor + n10;
            c11 = smokecolor + n11;
            if((meshi->inside==0&&iwall>0)||(meshi->inside!=0&&iwall<0)){
              if(ABS(c00[0] - c11[0]) >  ABS(c01[0] - c10[0])){
                DRAWZPLANE(0,1,3);
                DRAWZPLANE(1,2,3);
              }
              else{
                DRAWZPLANE(0,1,2);
                DRAWZPLANE(0,2,3);
              }
            }
            else{
              if(ABS(c00[0] - c11[0]) >  ABS(c01[0] - c10[0])){
                DRAWZPLANE(0,3,1);
                DRAWZPLANE(1,3,2);
              }
              else{
                DRAWZPLANE(0,3,2);
                DRAWZPLANE(0,2,1);
              }
            }
          }
        }
        break;
      default:
        assert(FFALSE);
        break;
    }
    glEnd();
  }
  if(use_transparency_data==1)TransparentOff();
}

/* ------------------ UpdateVolsmokeSupertexture ------------------------ */

void UpdateVolsmokeSupertexture(supermeshdata *smesh){
  GLsizei ni, nj, nk;
  int i;

  // smoke texture

  glActiveTexture(GL_TEXTURE0);
  for(i=0;i<smesh->nmeshes;i++){
    meshdata *meshi;
    int *s_offset;
    float *smokedataptr;

    meshi = smesh->meshes[i];
    smokedataptr = meshi->volrenderinfo->smokedataptr;

    s_offset = meshi->s_offset;

    ni = meshi->ibar+1;
    nj = meshi->jbar+1;
    nk = meshi->kbar+1;
#ifdef pp_GPU
    GPUnframes += 3*ni*nj*nk;
#endif
    glTexSubImage3D(GL_TEXTURE_3D,0,s_offset[0],s_offset[1],s_offset[2],ni,nj,nk,GL_RED, GL_FLOAT, smokedataptr);
  }

  // fire texture

  glActiveTexture(GL_TEXTURE1);
  for(i=0;i<smesh->nmeshes;i++){
    meshdata *meshi;
    int *s_offset;
    float *firedataptr;

    meshi = smesh->meshes[i];
    firedataptr = meshi->volrenderinfo->firedataptr;
    if(firedataptr==NULL)continue;

    s_offset = meshi->s_offset;

    ni = meshi->ibar+1;
    nj = meshi->jbar+1;
    nk = meshi->kbar+1;
#ifdef pp_GPU
    GPUnframes += 3*ni*nj*nk;
#endif

    glTexSubImage3D(GL_TEXTURE_3D,0,s_offset[0],s_offset[1],s_offset[2],ni,nj,nk,GL_RED, GL_FLOAT, firedataptr);
  }

  // light texture

  glActiveTexture(GL_TEXTURE5);

  // blockage texture

  glActiveTexture(GL_TEXTURE3);
  for(i=0;i<smesh->nmeshes;i++){
    meshdata *meshi;
    int *s_offset;

    meshi = smesh->meshes[i];
    s_offset = meshi->s_offset;

    ni = meshi->ibar;
    nj = meshi->jbar;
    nk = meshi->kbar;
#ifdef pp_GPU
    GPUnframes += 3*ni*nj*nk;
#endif

    if(meshi->f_iblank_cell != NULL){
      glTexSubImage3D(GL_TEXTURE_3D, 0, s_offset[0], s_offset[1], s_offset[2], ni, nj, nk, GL_RED, GL_FLOAT, meshi->f_iblank_cell);
    }
  }
  glActiveTexture(GL_TEXTURE0);
}

/* ------------------ UpdateVolsmokeTexture ------------------------ */

void UpdateVolsmokeTexture(meshdata *meshi){
  GLsizei ni, nj, nk;
  int ijk_offset[3]={0,0,0};
  volrenderdata *vr;
  float *smokedata_local, *firedata_local;

  vr = meshi->volrenderinfo;
  smokedata_local = vr->smokedataptr;
  firedata_local  = vr->firedataptr;

  //  glGetIntegerv(GL_MAX_TEXTURE_COORDS,&ntextures);
  ni = meshi->ibar+1;
  nj = meshi->jbar+1;
  nk = meshi->kbar+1;
#ifdef pp_GPU
  GPUnframes += 3*ni*nj*nk;
#endif
  glActiveTexture(GL_TEXTURE0);
  glTexSubImage3D(GL_TEXTURE_3D,0,ijk_offset[0],ijk_offset[1],ijk_offset[2],ni,nj,nk,GL_RED, GL_FLOAT, smokedata_local);

  if(firedata_local!=NULL){
    glActiveTexture(GL_TEXTURE1);
    glTexSubImage3D(GL_TEXTURE_3D,0,ijk_offset[0],ijk_offset[1],ijk_offset[2],ni,nj,nk,GL_RED, GL_FLOAT, firedata_local);
  }

  if(meshi->f_iblank_cell!=NULL){
    ni = meshi->ibar;
    nj = meshi->jbar;
    nk = meshi->kbar;
    glActiveTexture(GL_TEXTURE3);
    glTexSubImage3D(GL_TEXTURE_3D, 0, ijk_offset[0], ijk_offset[1], ijk_offset[2], ni, nj, nk, GL_RED, GL_FLOAT, meshi->f_iblank_cell);
  }

  glActiveTexture(GL_TEXTURE0);
}

/* ------------------ DrawSmoke3dGpuVol ------------------------ */

void DrawSmoke3DGPUVol(void){

  int iwall;
  meshdata *meshold=NULL;
  int ii;
  int inside;
  int *drawsides;
  int newmesh;
  float dcell;

//  SVEXTERN int GPUload[30],GPUtime[30],SVDECL(nGPUframes,0),SVDECL(iGPUframes,0);
#ifdef pp_GPU
  START_TIMER(thisGPUtime);
  if(thisGPUtime>lastGPUtime+0.25){
#ifdef _DEBUG
    PRINTF("CPU->GPU %4.1f Mbytes/s\n",4.0*GPUnframes/(thisGPUtime-lastGPUtime)/(1024.0*1024.0));
#endif
    lastGPUtime=thisGPUtime;
    GPUnframes=0;
  }
#endif
  if(mouse_down==1&&show_volsmoke_moving==0){
    return;
  }
  glUniform3f(GPUvol_eyepos,eye_position_smv[0],eye_position_smv[1],eye_position_smv[2]);
  glUniform1f(GPUvol_xyzmaxdiff,xyzmaxdiff);
  glUniform1f(GPUvol_gpu_vol_factor,gpu_vol_factor);
  glUniform1f(GPUvol_fire_opacity_factor,fire_opacity_factor);
  glUniform1f(GPUvol_mass_extinct,mass_extinct);
  glUniform1i(GPUvol_volbw,volbw);
  glUniform1f(GPUvol_temperature_min, global_temp_min);
  glUniform1f(GPUvol_temperature_cutoff, global_temp_cb_min);
  glUniform1f(GPUvol_temperature_max, global_temp_cb_max);
  glUniform1i(GPUvol_block_volsmoke,block_volsmoke);

  SNIFF_ERRORS("after DrawSmoke3dGpuVol before update textures");
  if(use_transparency_data==1)TransparentOn();
  for(ii=0;ii<nvolfacelistinfo;ii++){
    volrenderdata *vr;
    volfacelistdata *vi;
    meshdata *meshi;
    float x1, x2, yy1, yy2, z1, z2;
    float xx, yy, zz;

    vi = volfacelistinfoptrs[ii];
    iwall=vi->iwall;
    meshi = vi->facemesh;

    if(meshvisptr[meshi-global_scase.meshescoll.meshinfo]==0)continue;
    if(iwall==0||meshi->drawsides[iwall+3]==0)continue;

    vr = meshi->volrenderinfo;
    if(vr->firedataptr==NULL&&vr->smokedataptr==NULL)continue;

    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    // define parameters for smoke drawing

    if(iwall<0){
      xx = meshi->x0;
      yy = meshi->y0;
      zz = meshi->z0;
    }
    else{
      xx = meshi->x1;
      yy = meshi->y1;
      zz = meshi->z1;
    }
    x1 = meshi->x0;
    x2 = meshi->x1;
    yy1 = meshi->y0;
    yy2 = meshi->y1;
    z1 = meshi->z0;
    z2 = meshi->z1;
    dcell = meshi->dcell;
    inside = meshi->inside;
    newmesh=0;
    if(combine_meshes==1){
      if(meshold==NULL||meshi->super!=meshold->super)newmesh=1;
      drawsides=meshi->super->drawsides;
    }
    else{
      if(meshi!=meshold)newmesh=1;
      drawsides=meshi->drawsides;
    }
    if(newmesh==1){
      if(combine_meshes==1){
        UpdateVolsmokeSupertexture(meshi->super);
      }
      else{
        UpdateVolsmokeTexture(meshi);
      }
    }

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


    SNIFF_ERRORS("before DrawSmoke3dGpuVol gpu defines");
    if(newmesh==1){
      glUniform1i(GPUvol_inside,inside);
      if(combine_meshes==1){
        float *smin, *smax;
        int *sdrawsides;

        smin = meshi->super->boxmin_scaled;
        smax = meshi->super->boxmax_scaled;
        sdrawsides = meshi->super->drawsides;
        glUniform3f(GPUvol_boxmin,smin[0],smin[1],smin[2]);
        glUniform3f(GPUvol_boxmax,smax[0],smax[1],smax[2]);
        glUniform1iv(GPUvol_drawsides,7,sdrawsides);
      }
      else{
        glUniform3f(GPUvol_boxmin,x1,yy1,z1);
        glUniform3f(GPUvol_boxmax,x2,yy2,z2);
        glUniform1iv(GPUvol_drawsides,7,drawsides);
      }
      if(vr->firedataptr!=NULL){
        glUniform1i(GPUvol_havefire,1);
      }
      else{
        glUniform1i(GPUvol_havefire,0);
      }
      glUniform1i(GPUvol_slicetype,vr->smokeslice->slice_filetype);
      glUniform3f(GPUvol_dcell3,meshi->dcell3[0],meshi->dcell3[1],meshi->dcell3[2]);
      glUniform1i(GPUvol_soot_density, 0);  // smokedata_local
      glUniform1i(GPUvol_fire,         1);  // firedata_local
      glUniform1i(GPUvol_smokecolormap,2);  // rgb_volsmokecolormap
      glUniform1i(GPUvol_blockage,     3);  // meshi->f_iblank_cell
      if(mouse_down==1){
        glUniform1f(GPUvol_dcell,8.0*dcell);
      }
      else{
        glUniform1f(GPUvol_dcell,dcell);
      }

      meshold=meshi;
    }
    SNIFF_ERRORS("after DrawSmoke3dGpuVol before loop");
    glBegin(GL_TRIANGLES);

    switch(iwall){
      case XWALLMIN:
      case XWALLMAX:
        if((inside==0&&iwall>0)||(inside!=0&&iwall<0)){
          glVertex3f(xx,yy1,z1);
          glVertex3f(xx,yy2,z1);
          glVertex3f(xx,yy2,z2);

          glVertex3f(xx,yy1,z1);
          glVertex3f(xx,yy2,z2);
          glVertex3f(xx,yy1,z2);
        }
        else{
          glVertex3f(xx,yy1,z1);
          glVertex3f(xx,yy2,z2);
          glVertex3f(xx,yy2,z1);

          glVertex3f(xx,yy1,z1);
          glVertex3f(xx,yy1,z2);
          glVertex3f(xx,yy2,z2);
        }
        break;
      case YWALLMIN:
      case YWALLMAX:
        if((inside==0&&iwall>0)||(inside!=0&&iwall<0)){
          glVertex3f(x1,yy,z1);
          glVertex3f(x2,yy,z2);
          glVertex3f(x2,yy,z1);

          glVertex3f(x1,yy,z1);
          glVertex3f(x1,yy,z2);
          glVertex3f(x2,yy,z2);
        }
        else{
          glVertex3f(x1,yy,z1);
          glVertex3f(x2,yy,z1);
          glVertex3f(x2,yy,z2);

          glVertex3f(x1,yy,z1);
          glVertex3f(x2,yy,z2);
          glVertex3f(x1,yy,z2);
        }
        break;
      case ZWALLMIN:
      case ZWALLMAX:
        if((inside==0&&iwall>0)||(inside!=0&&iwall<0)){
          glVertex3f(x1,yy1,zz);
          glVertex3f(x2,yy1,zz);
          glVertex3f(x2,yy2,zz);

          glVertex3f(x1,yy1,zz);
          glVertex3f(x2,yy2,zz);
          glVertex3f(x1,yy2,zz);
        }
        else{
          glVertex3f(x1,yy1,zz);
          glVertex3f(x2,yy2,zz);
          glVertex3f(x2,yy1,zz);

          glVertex3f(x1,yy1,zz);
          glVertex3f(x1,yy2,zz);
          glVertex3f(x2,yy2,zz);
        }
        break;
      default:
        assert(FFALSE);
        break;
    }
    glEnd();
  }
  SNIFF_ERRORS("after DrawSmoke3dGpuVol after loop");
  if(use_transparency_data==1)TransparentOff();
}

/* ------------------ GetVolsmokeFrameTime ------------------------ */

float GetVolsmokeFrameTime(volrenderdata *vr, int framenum){
  slicedata *smokeslice;
  FILE *SLICEFILE;
  int framesize;
  float time_local=0.0;
  LINT skip_local;

  if(framenum<0||framenum>=vr->ntimes)return time_local;
  smokeslice=vr->smokeslice;
  framesize = smokeslice->nslicei*smokeslice->nslicej*smokeslice->nslicek;

  skip_local =           (HEADER_SIZE+30        +TRAILER_SIZE); // long label
  skip_local +=          (HEADER_SIZE+30        +TRAILER_SIZE); // short label
  skip_local +=          (HEADER_SIZE+30        +TRAILER_SIZE); // unit label
  skip_local +=          (HEADER_SIZE+24        +TRAILER_SIZE); // is1, is2, js1, js2, ks1, ks2
  skip_local += framenum*(HEADER_SIZE +4        +TRAILER_SIZE); // framenum time's
  skip_local += (LINT)framenum*(LINT)(HEADER_SIZE +4*framesize+TRAILER_SIZE); // framenum slice data's

  SLICEFILE=fopen(smokeslice->reg_file,"rb");
  if(SLICEFILE==NULL)return time_local;

  FSEEK(SLICEFILE,skip_local,SEEK_SET); // skip from beginning of file

  FORTVOLSLICEREAD(&time_local,1);
  fclose(SLICEFILE);
  return time_local;
}

/* ------------------ GetVolsmokeAllTimes ------------------------ */

void GetVolsmokeAllTimes(volrenderdata *vr){
  int i;
  FILE *volstream=NULL;

  if(load_volcompressed==1&&vr->smokeslice->vol_file!=NULL){
    volstream=fopen(vr->smokeslice->vol_file,"rb");
  }

  if(volstream==NULL){
    for(i=0;i<vr->ntimes;i++){
      vr->times[i]= GetVolsmokeFrameTime(vr,i);
    }
  }
  else{
    unsigned char buffer[32];
    int ii;
// 1,completion,version
// 1,version,n_data_compressedm32,nbytes,n_data_in,time,valmin,valmax,data ....

// smoke positions

    FSEEK(volstream,12,SEEK_SET);
    for(ii=0;ii<vr->ntimes;ii++){
      int ncompressed;
      float *time_local;

      vr->smokepos[ii]=FTELL(volstream);
      if(fread(buffer,1,32,volstream)!=32)break;
      ncompressed=*(int *)(buffer+8)-32;
      time_local=(float *)(buffer+20);
      if(FSEEK(volstream,ncompressed,SEEK_CUR)!=0)break;
      vr->times[ii]=*time_local;
    }
    fclose(volstream);

// fire positions

    volstream=NULL;
    if(vr->fireslice->vol_file!=NULL)volstream=fopen(vr->fireslice->vol_file,"rb");
    if(volstream!=NULL){
      FSEEK(volstream,12,SEEK_SET);
      for(ii=0;ii<vr->ntimes;ii++){
        int ncompressed;

        vr->firepos[ii]=FTELL(volstream);
        if(fread(buffer,1,32,volstream)!=32)break;
        ncompressed=*(int *)(buffer+8)-32;
        if(FSEEK(volstream,ncompressed,SEEK_CUR)!=0)break;
      }
      fclose(volstream);
    }
  }
}

/* ------------------ FreeVolsmokeFrame ------------------------ */

void FreeVolsmokeFrame(volrenderdata *vr, int framenum){
  int i;

  for(i=0;i<framenum;i++){
    void *smokedataptr, *firedataptr;

    smokedataptr=vr->smokedataptrs[i];
    FREEMEMORY(smokedataptr);
    vr->smokedataptrs[i]=NULL;

    firedataptr=vr->firedataptrs[i];
    FREEMEMORY(firedataptr);
    vr->firedataptrs[i]=NULL;
  }
}

/* ------------------ ReadVolsmokeFrame ------------------------ */
#define VOL_OFFSET 32
void ReadVolsmokeFrame(volrenderdata *vr, int framenum, int *first){
  slicedata *fireslice, *smokeslice;
  FILE *SLICEFILE;
  int framesize,framesize2;
  LINT skip_local;
  float time_local, *smokeframe_data=NULL, *fireframe_data=NULL;
  unsigned char *c_smokedata_compressed=NULL, *c_firedata_compressed=NULL;
  unsigned char *c_firedata_compressed2=NULL;
  uLongf              n_smokedata_compressed,     n_firedata_compressed;
  unsigned int size_before=0, size_after=0;
  FILE *volstream=NULL;
  int print = 0;

  if(framenum<0||framenum>=vr->ntimes)return;

  smokeslice = vr->smokeslice;
  fireslice  = vr->fireslice;

  framesize = smokeslice->nslicei*smokeslice->nslicej*smokeslice->nslicek;
  framesize2 = framesize+VOL_OFFSET;
  if(compress_volsmoke==1){
    vr->is_compressed=1;
  }
  else{
    vr->is_compressed=0;
  }
  if(vr->is_compressed==1||load_volcompressed==1){
    n_smokedata_compressed=1.01*framesize2+600;
    if(vr->smokedata_full==NULL){
      NewMemory((void **)&vr->smokedata_full,framesize*sizeof(float));
      NewMemory((void **)&vr->smokedata_view,framesize*sizeof(float));
      NewMemory((void **)&vr->c_smokedata_view,framesize2);
    }
    smokeframe_data=vr->smokedata_full;

    if(fireslice!=NULL){
      n_firedata_compressed=1.01*framesize2+600;
      if(vr->firedata_full==NULL){
        NewMemory((void **)&vr->firedata_full,framesize*sizeof(float));
        NewMemory((void **)&vr->firedata_view,framesize*sizeof(float));
        NewMemory((void **)&vr->c_firedata_view,framesize2);
      }
      NewMemory((void **)&c_firedata_compressed,n_firedata_compressed);
      NewMemory((void **)&c_firedata_compressed2,n_firedata_compressed);
      fireframe_data=vr->firedata_full;
    }
  }
  else{
    NewMemory((void **)&smokeframe_data, framesize*sizeof(float));
    NewMemory((void **)&fireframe_data,  framesize*sizeof(float));
  }

  if(load_volcompressed==1&&vr->smokeslice->vol_file!=NULL){
    volstream=fopen(vr->smokeslice->vol_file,"rb");
  }

  skip_local = (HEADER_SIZE + 30 + TRAILER_SIZE); // long label
  skip_local += (HEADER_SIZE + 30 + TRAILER_SIZE); // short label
  skip_local += (HEADER_SIZE + 30 + TRAILER_SIZE); // unit label
  skip_local += (HEADER_SIZE + 24 + TRAILER_SIZE); // is1, is2, js1, js2, ks1, ks2
  skip_local += framenum * (HEADER_SIZE + 4 + TRAILER_SIZE); // framenum time's
  skip_local += ( LINT )framenum * ( LINT )(HEADER_SIZE + 4 * framesize + TRAILER_SIZE); // framenum slice data's
  if(volstream==NULL){
    SLICEFILE=fopen(smokeslice->reg_file,"rb");
    if(SLICEFILE==NULL)return;

    FSEEK(SLICEFILE,skip_local,SEEK_SET); // skip from beginning of file

    FORTVOLSLICEREAD(&time_local,1);
    if(global_times!=NULL&&global_times[itimes]>time_local)restart_time=1;
    if(*first==1){
      *first=0;
      print=1;
      PRINTF("time=%.2f ",time_local);
    }

    vr->times[framenum]=time_local;
    FORTVOLSLICEREAD(smokeframe_data,framesize);
    CheckMemory;
    size_before+=sizeof(float)*framesize;
    if(vr->is_compressed==1){
      float valmin=0.0;

    // one,file version,ndata_compressed,nbytes 1/2/4,ndata_uncompressed,time_local,valmin,valmax,data ....
      CompressVolSliceFrame(smokeframe_data, framesize, time_local, &valmin, NULL,
                  &c_smokedata_compressed, &n_smokedata_compressed);
      size_after+=n_smokedata_compressed;
      vr->smokedataptrs[framenum]=c_smokedata_compressed;
    }
    else{
      vr->smokedataptrs[framenum]=smokeframe_data;
    }
    vr->smokedataptr = vr->smokedataptrs[framenum];
    CheckMemory;
    if(print==1)PRINTF("smoke");
    fclose(SLICEFILE);
  }
  else{
    unsigned char buffer[32];
    int ncompressed;

// 1,completion,version
// 1,version,n_data_compressedm32,nbytes,n_data_in,time,valmin,valmax,data ....
    FSEEK(volstream,vr->smokepos[framenum],SEEK_SET);
    fread(buffer,8,4,volstream);
    ncompressed=*(int *)(buffer+8);
    time_local = *(float *)(buffer+20);
    FSEEK(volstream,vr->smokepos[framenum],SEEK_SET);
    NewMemory((void **)&c_smokedata_compressed,ncompressed);
    fread(c_smokedata_compressed,1,ncompressed,volstream);
    vr->smokedataptrs[framenum]=c_smokedata_compressed;

    if(*first==1){
      *first=0;
      print=1;
      PRINTF("time=%.2f ",time_local);
    }

    vr->times[framenum]=time_local;
    fclose(volstream);
    volstream=NULL;
  }

  if(fireslice!=NULL){
    if(load_volcompressed==1&&vr->fireslice->vol_file!=NULL){
      volstream=fopen(vr->fireslice->vol_file,"rb");
    }
    if(volstream==NULL){
      SLICEFILE=fopen(fireslice->reg_file,"rb");
      if(SLICEFILE!=NULL){
        FSEEK(SLICEFILE,skip_local,SEEK_SET); // skip from beginning of file

        FORTVOLSLICEREAD(&time_local,1);
        vr->times[framenum]=time_local;
        FORTVOLSLICEREAD(fireframe_data,framesize);
        CheckMemory;
        size_before+=sizeof(float)*framesize;
        if(vr->is_compressed==1){
          float valmin=20.0, valmax=1400.0;

          CompressVolSliceFrame(fireframe_data, framesize,  time_local, &valmin, &valmax,
                  &c_firedata_compressed, &n_firedata_compressed);
          size_after+=n_firedata_compressed;
          vr->firedataptrs[framenum]=c_firedata_compressed;
          vr->nfiredata_compressed[framenum]=n_firedata_compressed;
        }
        else{
          vr->firedataptrs[framenum]=fireframe_data;
        }
        vr->firedataptr = vr->firedataptrs[framenum];
        if(print==1)PRINTF(", fire");
        fclose(SLICEFILE);
      }
    }
    else{
      unsigned char buffer[32];
      int ncompressed;

// 1,completion,version
// 1,version,n_data_compressedm32,nbytes,n_data_in,time_local,valmin,valmax,data ....
      FSEEK(volstream,vr->firepos[framenum],SEEK_SET);
      fread(buffer,8,4,volstream);
      ncompressed=*(int *)(buffer+8);
      time_local = *(float *)(buffer+20);
      FSEEK(volstream,vr->firepos[framenum],SEEK_SET);
      NewMemory((void **)&c_firedata_compressed,ncompressed);
      fread(c_firedata_compressed,1,ncompressed,volstream);
      vr->firedataptrs[framenum]=c_firedata_compressed;
      vr->firedataptr = vr->firedataptrs[framenum];

      vr->times[framenum]=time_local;
      if(print==1)PRINTF(", fire");
      fclose(volstream);
      volstream=NULL;
    }
  }
  CheckMemory;
  vr->dataready[framenum]=1;
  if(vr->is_compressed==1&&load_volcompressed==0){
    PRINTF(" (%4.1f%s)",(float)size_before/(float)size_after,"X");
  }
  if(print==1)PRINTF("\n");
}

/* ------------------ UnloadVolsmokeFrameAllMeshes ------------------------ */

void UnloadVolsmokeFrameAllMeshes(int framenum){
  int i;

  PRINTF("Unloading smoke frame: %i\n",framenum);
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + i;
    vr = meshi->volrenderinfo;
    if(vr->smokeslice==NULL||vr->fireslice==NULL||vr->loaded==0)continue;
    FREEMEMORY(vr->firedataptrs[framenum]);
    FREEMEMORY(vr->smokedataptrs[framenum]);
//    vr->loaded=0;
//    vr->display=0;
  }
}

/* ------------------ UnloadVolsmokeAllFrames ------------------------ */

void UnloadVolsmokeAllFrames(volrenderdata *vr){
  int i;

  PRINTF("Unloading smoke %s - ",vr->rendermeshlabel);
  for(i=0;i<vr->ntimes;i++){
    FREEMEMORY(vr->firedataptrs[i]);
    FREEMEMORY(vr->smokedataptrs[i]);
    vr->dataready[i]=0;
  }
  vr->loaded=0;
  vr->display=0;
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  UpdateTimes();
  PRINTF("completed");
  PRINTF("\n");
}

/* ------------------ ReadVolsmokeAllFrames ------------------------ */

void ReadVolsmokeAllFrames(volrenderdata *vr){
  int nframes;
  int i;
  int first=1;

  nframes = vr->ntimes;
  for(i=0;i<nframes;i++){
    ReadVolsmokeFrame(vr, i, &first);
  }
  if(vr->is_compressed==1||load_volcompressed==1){//xyz BEGIN
    vr->smokedataptr = vr->smokedata_view;
    vr->firedataptr = vr->firedata_view;
  }
  else{
    vr->smokedataptr = vr->smokedataptrs[0];  //*** hack
    vr->firedataptr = vr->firedataptrs[0];
  }
  vr->loaded=1;
  vr->display=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  stept=1;
  UpdateTimes();
}

/* ------------------ ReadVolsmokeFrameAllMeshes ------------------------ */

void ReadVolsmokeFrameAllMeshes(int framenum, supermeshdata *smesh){
  int i;
  int first=1;
  int nm;

  if(smesh==NULL){
    nm=global_scase.meshescoll.nmeshes;
  }
  else{
    nm=smesh->nmeshes;
  }
  for(i=0;i<nm;i++){
    meshdata *meshi;
    volrenderdata *vr;

    if(smesh==NULL){
      meshi = global_scase.meshescoll.meshinfo + i;
    }
    else{
      meshi = smesh->meshes[i];
    }
    vr = meshi->volrenderinfo;
    if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
    if(read_vol_mesh==i||read_vol_mesh==VOL_READALL){
      ReadVolsmokeFrame(vr,framenum,&first);
    }
  }
  for(i=0;i<nm;i++){
    meshdata *meshi;
    volrenderdata *vr;

    if(smesh==NULL){
      meshi = global_scase.meshescoll.meshinfo + i;
    }
    else{
      meshi = smesh->meshes[i];
    }
    vr = meshi->volrenderinfo;
    if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
    if(read_vol_mesh!=i&&read_vol_mesh!=VOL_READALL)continue;
    if(framenum==0){
      if(vr->is_compressed==1||load_volcompressed==1){
        vr->smokedataptr = vr->smokedata_view;  //*** hack
        vr->firedataptr  = vr->firedata_view;    //*** hack
      }
      else{
        vr->smokedataptr = vr->smokedataptrs[0];  //*** hack
        vr->firedataptr  = vr->firedataptrs[0];
      }
    }
    vr->loaded=1;
    vr->display=1;
  }
}

/* ------------------ ReadVolsmokeAllFramesAllMeshes2 ------------------------ */

void *ReadVolsmokeAllFramesAllMeshes2(void *arg){
  int i;
  int nframes=0;

  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + i;
    vr = meshi->volrenderinfo;
    if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
    if(read_vol_mesh!=VOL_READALL&&read_vol_mesh!=i)continue;
    if(vr->ntimes>0){
      nframes=vr->ntimes;
      break;
    }
  }
  for(i=0;i<nframes;i++){
    ReadVolsmokeFrameAllMeshes(i,NULL);
  }
  read_vol_mesh = VOL_READNONE;
  return NULL;
}

/* ------------------ UnloadVolsmokeSuperTextures ------------------------ */

void UnloadVolsmokeSuperTextures(void){
  int i, doit;

  doit = 0;
  for(i = 0;i<global_scase.nsupermeshinfo;i++){
    supermeshdata *smesh;

    smesh = global_scase.supermeshinfo+i;
    if(smesh->volsmoke_texture_buffer!=NULL||smesh->volfire_texture_buffer!=NULL){
      doit = 1;
      break;
    }
  }
  if(doit==0)return;
  PRINTF("Unloading smoke and fire textures for each supermesh\n");
  for(i = 0;i<global_scase.nsupermeshinfo;i++){
    supermeshdata *smesh;

    smesh = global_scase.supermeshinfo+i;
    FREEMEMORY(smesh->volfire_texture_buffer);
    FREEMEMORY(smesh->volsmoke_texture_buffer);
  }
  PRINTF("complete");
  PRINTF("\n");
}

/* ------------------ InitVolsmokeTexture ------------------------ */

void InitVolsmokeTexture(meshdata *meshi){
  GLint border_size = 0;
  GLsizei nx, ny, nz;
  int i;

  //UnloadVolsmokeSuperTextures();
  if(verbose_output==1)PRINTF("defining smoke and fire textures for %s - ", meshi->label);
  FFLUSH();

  nx = meshi->ibar+1;
  ny = meshi->jbar+1;
  nz = meshi->kbar+1;

// define smoke texture

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &meshi->volsmoke_texture_id);
  glBindTexture(GL_TEXTURE_3D, meshi->volsmoke_texture_id);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  if(meshi->volsmoke_texture_buffer==NULL){
    NewMemory((void **)&meshi->volsmoke_texture_buffer, nx*ny*nz*sizeof(float));
  }
  for(i = 0;i<nx*ny*nz;i++){
    meshi->volsmoke_texture_buffer[i] = 0.0;
  }
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F,
    nx, ny, nz, border_size,
    GL_RED, GL_FLOAT, meshi->volsmoke_texture_buffer);

// define fire texture

  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &meshi->volfire_texture_id);
  glBindTexture(GL_TEXTURE_3D, meshi->volfire_texture_id);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  if(meshi->volfire_texture_buffer==NULL){
    NewMemory((void **)&meshi->volfire_texture_buffer, nx*ny*nz*sizeof(float));
  }
  for(i = 0;i<nx*ny*nz;i++){
    meshi->volfire_texture_buffer[i] = 0.0;
  }
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F,
    nx, ny, nz, border_size,
    GL_RED, GL_FLOAT, meshi->volfire_texture_buffer);

// define light texture

  if(volsmoke_colormap_id_defined==-1){
    volsmoke_colormap_id_defined = 1;
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &volsmoke_colormap_id);
    glBindTexture(GL_TEXTURE_1D, volsmoke_colormap_id);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, MAXSMOKERGB, 0, GL_RGBA, GL_FLOAT, rgb_volsmokecolormap);
  }

// define blockage texture

  nx = meshi->ibar;
  ny = meshi->jbar;
  nz = meshi->kbar;

  if(meshi->f_iblank_cell != NULL){
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &meshi->blockage_texture_id);
    glBindTexture(GL_TEXTURE_3D, meshi->blockage_texture_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, border_size, GL_RED, GL_FLOAT, meshi->f_iblank_cell);
  }

  glActiveTexture(GL_TEXTURE0);
  if(verbose_output==1)PRINTF("complete");
  if(verbose_output==1)PRINTF("\n");
  FFLUSH();
}

#ifdef pp_GPU
/* ------------------ DefineVolsmokeTextures ------------------------ */

void DefineVolsmokeTextures(void){
  int i;

  if(combine_meshes==1&&gpuactive==1){
#ifdef pp_GPU
    for(i=0;i<global_scase.nsupermeshinfo;i++){
      supermeshdata *smesh;

      smesh = global_scase.supermeshinfo + i;
      InitVolsmokeSuperTexture(smesh);
    }
#endif
  }
  else{
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo  + i;
      InitVolsmokeTexture(meshi);
    }
  }
}

/* ------------------ ReadVolsmokeAllFramesAllMeshes ------------------------ */

void ReadVolsmokeAllFramesAllMeshes(void){
  int i;

  compress_volsmoke=glui_compress_volsmoke;
  load_volcompressed=glui_load_volcompressed;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + i;
    vr = meshi->volrenderinfo;
    if(vr->fireslice==NULL||vr->smokeslice==NULL)continue;
    if(read_vol_mesh!=VOL_READALL&&read_vol_mesh!=i)continue;
    GetVolsmokeAllTimes(vr);
    vr->loaded=1;
    vr->display=1;
    if(gpuactive==1){
      if(combine_meshes==1&&gpuactive==1){
#ifdef pp_GPU
        InitVolsmokeSuperTexture(meshi->super);
#endif
      }
      else{
        InitVolsmokeTexture(meshi);
      }
    }
  }
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  stept=1;
  UpdateTimes();

  if(volsmokeload_threads == NULL){
    volsmokeload_threads = THREADinit(&n_volsmokeload_threads, &use_volsmokeload_threads, ReadVolsmokeAllFramesAllMeshes2);
  }
  THREADrun(volsmokeload_threads);
}

/* ------------------ UnloadVolsmokeTextures ------------------------ */

void UnloadVolsmokeTextures(void){
  int  i;

  PRINTF("Unloading smoke and fire textures for each mesh\n");
  FFLUSH();
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + i;
    FREEMEMORY(meshi->volsmoke_texture_buffer);
    FREEMEMORY(meshi->volfire_texture_buffer);
  }
}

#endif
