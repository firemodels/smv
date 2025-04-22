#include <string.h>

#include "dmalloc.h"
#include "colorbars.h"

/* ------------------ CreateColorbarRainbow ------------------------ */

void CreateColorbarRainbow(colorbardata *cbi){
// rainbow colorbar
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "Rainbow");
  cbi->nnodes = 5;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 64;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 192;
  cbi->node_rgb[5] = 192;

  cbi->node_index[2] = 128;
  cbi->node_rgb[6] = 0;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 192;
  cbi->node_rgb[9] = 192;
  cbi->node_rgb[10] = 192;
  cbi->node_rgb[11] = 0;

  cbi->node_index[4] = 255;
  cbi->node_rgb[12] = 255;
  cbi->node_rgb[13] = 0;
  cbi->node_rgb[14] = 0;
  strcpy(cbi->colorbar_type, "rainbow");
}

/* ------------------ CreateColorbarOriginalRainbow ------------------------ */

void CreateColorbarOriginalRainbow(colorbardata *cbi){
// original rainbow colorbar
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "Rainbow_orig");
  cbi->nnodes = 5;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 64;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 255;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 128;
  cbi->node_rgb[6] = 0;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 192;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 255;
  cbi->node_rgb[11] = 0;

  cbi->node_index[4] = 255;
  cbi->node_rgb[12] = 255;
  cbi->node_rgb[13] = 0;
  cbi->node_rgb[14] = 0;
  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarRainbow2 ------------------------ */

void CreateColorbarRainbow2(colorbardata *cbi){
// Rainbow 2 colorbar
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "Rainbow 2");
  cbi->nnodes = 12;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 4;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 108;

  cbi->node_index[1] = 20;
  cbi->node_rgb[3] = 6;
  cbi->node_rgb[4] = 3;
  cbi->node_rgb[5] = 167;

  cbi->node_index[2] = 60;
  cbi->node_rgb[6] = 24;
  cbi->node_rgb[7] = 69;
  cbi->node_rgb[8] = 240;

  cbi->node_index[3] = 70;
  cbi->node_rgb[9] = 31;
  cbi->node_rgb[10] = 98;
  cbi->node_rgb[11] = 214;

  cbi->node_index[4] = 80;
  cbi->node_rgb[12] = 5;
  cbi->node_rgb[13] = 125;
  cbi->node_rgb[14] = 170;

  cbi->node_index[5] = 96;
  cbi->node_rgb[15] = 48;
  cbi->node_rgb[16] = 155;
  cbi->node_rgb[17] = 80;

  cbi->node_index[6] = 112;
  cbi->node_rgb[18] = 82;
  cbi->node_rgb[19] = 177;
  cbi->node_rgb[20] = 8;

  cbi->node_index[7] = 163;
  cbi->node_rgb[21] = 240;
  cbi->node_rgb[22] = 222;
  cbi->node_rgb[23] = 3;

  cbi->node_index[8] = 170;
  cbi->node_rgb[24] = 249;
  cbi->node_rgb[25] = 214;
  cbi->node_rgb[26] = 7;

  cbi->node_index[9] = 200;
  cbi->node_rgb[27] = 252;
  cbi->node_rgb[28] = 152;
  cbi->node_rgb[29] = 22;

  cbi->node_index[10] = 230;
  cbi->node_rgb[30] = 254;
  cbi->node_rgb[31] = 67;
  cbi->node_rgb[32] = 13;

  cbi->node_index[11] = 255;
  cbi->node_rgb[33] = 215;
  cbi->node_rgb[34] = 5;
  cbi->node_rgb[35] = 13;
  strcpy(cbi->colorbar_type, "deprecated");
}

/* ------------------ CreateColorbarYellowRed ------------------------ */

void CreateColorbarYellowRed(colorbardata *cbi){
// yellow/red
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "yellow->red");
  cbi->nnodes = 2;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 255;
  cbi->node_rgb[1] = 255;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 255;
  cbi->node_rgb[3] = 255;
  cbi->node_rgb[4] = 0;
  cbi->node_rgb[5] = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarBlueRedGreen ------------------------ */

void CreateColorbarBlueRedGreen(colorbardata *cbi){
// blue/green/red
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "blue->green->red");
  cbi->nnodes = 3;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 128;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 255;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 255;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 0;
  cbi->node_rgb[8] = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarBlueYellowWhite ------------------------ */

void CreateColorbarBlueYellowWhite(colorbardata *cbi){
// blue/yellow/white
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "blue->yellow->white");
  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 151;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 113;
  cbi->node_rgb[3] = 255;
  cbi->node_rgb[4] = 0;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 212;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 255;
  cbi->node_rgb[11] = 255;
  strcpy(cbi->colorbar_type, "deprecated");
}

/* ------------------ CreateColorbarBlueRedSplit ------------------------ */

void CreateColorbarBlueRedSplit(colorbardata *cbi){
// blue->red split
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "blue->red split");
  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 127;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 255;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 128;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 0;
  cbi->node_rgb[11] = 0;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "divergent");
}

/* ------------------ CreateColorbarAfacSplit ------------------------ */

void CreateColorbarAfacSplit(colorbardata *cbi){
// AFAC split
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "AFAC split");
  cbi->nnodes = 8;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 178;
  cbi->node_rgb[2] = 90;

  cbi->node_index[1] = 80;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 178;
  cbi->node_rgb[5] = 90;

  cbi->node_index[2] = 81;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 243;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 100;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 243;
  cbi->node_rgb[11] = 0;

  cbi->node_index[4] = 101;
  cbi->node_rgb[12] = 250;
  cbi->node_rgb[13] = 150;
  cbi->node_rgb[14] = 38;

  cbi->node_index[5] = 140;
  cbi->node_rgb[15] = 250;
  cbi->node_rgb[16] = 150;
  cbi->node_rgb[17] = 38;

  cbi->node_index[6] = 141;
  cbi->node_rgb[18] = 209;
  cbi->node_rgb[19] = 34;
  cbi->node_rgb[20] = 41;

  cbi->node_index[7] = 255;
  cbi->node_rgb[21] = 209;
  cbi->node_rgb[22] = 34;
  cbi->node_rgb[23] = 41;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "divergent");
}

/* ------------------ CreateColorbarBlackWhite ------------------------ */

void CreateColorbarBlackWhite(colorbardata *cbi){
// black->white
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "black->white");

  cbi->nnodes = 2;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 255;
  cbi->node_rgb[3] = 255;
  cbi->node_rgb[4] = 255;
  cbi->node_rgb[5] = 255;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarFed ------------------------ */

void CreateColorbarFed(colorbardata *cbi){
// FED
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "FED");

  cbi->nnodes = 6;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 96;
  cbi->node_rgb[1] = 96;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 26; // 0.295276,0.307087
  cbi->node_rgb[3] = 96;
  cbi->node_rgb[4] = 96;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 26;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 85; // 0.992126,1.003937
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 255;
  cbi->node_rgb[11] = 0;

  cbi->node_index[4] = 85;
  cbi->node_rgb[12] = 255;
  cbi->node_rgb[13] = 155;
  cbi->node_rgb[14] = 0;

  cbi->node_index[5] = 255;
  cbi->node_rgb[15] = 255;
  cbi->node_rgb[16] = 155;
  cbi->node_rgb[17] = 0;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarFireOriginal ------------------------ */

void CreateColorbarFireOriginal(colorbardata *cbi){
// fire (original)
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "fire");

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 127;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 0;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 128;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 128;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 128;
  cbi->node_rgb[11] = 0;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarFire2 ------------------------ */

void CreateColorbarFire2(colorbardata *cbi){
// fire 2
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "fire 2");

  cbi->nnodes = 10;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 127;
  cbi->node_rgb[3] = 38;
  cbi->node_rgb[4] = 0;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 128;
  cbi->node_rgb[6] = 219;
  cbi->node_rgb[7] = 68;
  cbi->node_rgb[8] = 21;

  cbi->node_index[3] = 160;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 125;
  cbi->node_rgb[11] = 36;

  cbi->node_index[4] = 183;
  cbi->node_rgb[12] = 255;
  cbi->node_rgb[13] = 157;
  cbi->node_rgb[14] = 52;

  cbi->node_index[5] = 198;
  cbi->node_rgb[15] = 255;
  cbi->node_rgb[16] = 170;
  cbi->node_rgb[17] = 63;

  cbi->node_index[6] = 214;
  cbi->node_rgb[18] = 255;
  cbi->node_rgb[19] = 198;
  cbi->node_rgb[20] = 93;

  cbi->node_index[7] = 229;
  cbi->node_rgb[21] = 255;
  cbi->node_rgb[22] = 208;
  cbi->node_rgb[23] = 109;

  cbi->node_index[8] = 244;
  cbi->node_rgb[24] = 255;
  cbi->node_rgb[25] = 234;
  cbi->node_rgb[26] = 161;

  cbi->node_index[9] = 255;
  cbi->node_rgb[27] = 255;
  cbi->node_rgb[28] = 255;
  cbi->node_rgb[29] = 238;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarFire3 ------------------------ */

void CreateColorbarFire3(colorbardata *cbi){
// fire 3
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "fire 3");

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 108;
  cbi->node_rgb[3] = 255;
  cbi->node_rgb[4] = 127;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 156;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 255;
  cbi->node_rgb[11] = 255;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarCool ------------------------ */

void CreateColorbarCool(colorbardata *cbi){
// cool
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "cool");

  cbi->nnodes = 7;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 90;
  cbi->node_rgb[3] = 64;
  cbi->node_rgb[4] = 64;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 110;
  cbi->node_rgb[6] = 155;
  cbi->node_rgb[7] = 35;
  cbi->node_rgb[8] = 33;

  cbi->node_index[3] = 120;
  cbi->node_rgb[9] = 108;
  cbi->node_rgb[10] = 19;
  cbi->node_rgb[11] = 43;

  cbi->node_index[4] = 130;
  cbi->node_rgb[12] = 208;
  cbi->node_rgb[13] = 93;
  cbi->node_rgb[14] = 40;

  cbi->node_index[5] = 160;
  cbi->node_rgb[15] = 255;
  cbi->node_rgb[16] = 178;
  cbi->node_rgb[17] = 0;

  cbi->node_index[6] = 255;
  cbi->node_rgb[18] = 255;
  cbi->node_rgb[19] = 255;
  cbi->node_rgb[20] = 255;
  strcpy(cbi->colorbar_type, "deprecated");
}

/* ------------------ CreateColorbarFireLineLevelSet ------------------------ */

void CreateColorbarFireLineLevelSet(colorbardata *cbi){
// fire line (level set)
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "fire line (level set)");

  cbi->nnodes = 6;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 1;
  cbi->node_rgb[2] = 2;

  cbi->node_index[1] = 120;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 1;
  cbi->node_rgb[5] = 2;

  cbi->node_index[2] = 120;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 0;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 136;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 0;
  cbi->node_rgb[11] = 0;

  cbi->node_index[4] = 136;
  cbi->node_rgb[12] = 64;
  cbi->node_rgb[13] = 64;
  cbi->node_rgb[14] = 64;

  cbi->node_index[5] = 255;
  cbi->node_rgb[15] = 64;
  cbi->node_rgb[16] = 64;
  cbi->node_rgb[17] = 64;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarFireLineWallThickness ------------------------ */

void CreateColorbarFireLineWallThickness(colorbardata *cbi){
// fire line (wall thickness)
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "fire line (wall thickness)");

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 32;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 0;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 32;
  cbi->node_rgb[6] = 253;
  cbi->node_rgb[7] = 254;
  cbi->node_rgb[8] = 255;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 253;
  cbi->node_rgb[10] = 254;
  cbi->node_rgb[11] = 255;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarSplit ------------------------ */

void CreateColorbarSplit(colorbardata *cbi){
// split
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "split");
  // TODO: It would seem this would be fine, but not sure.
  int colorsplit[12] = {0, 0, 0, 64, 64, 255, 0, 192, 0, 255, 0, 0};

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_index[1] = 127;
  cbi->node_index[2] = 128;
  cbi->node_index[3] = 255;
  for (int i = 0; i < 12; i++){
    cbi->node_rgb[i] = colorsplit[i];
  }

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarMethanol ------------------------ */

void CreateColorbarMethanol(colorbardata *cbi){
// Methanol
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "Methanol");

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 9;
  cbi->node_rgb[1] = 190;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 192;
  cbi->node_rgb[3] = 9;
  cbi->node_rgb[4] = 190;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 200;
  cbi->node_rgb[6] = 9;
  cbi->node_rgb[7] = 190;
  cbi->node_rgb[8] = 255;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 9;
  cbi->node_rgb[10] = 190;
  cbi->node_rgb[11] = 255;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarPropane ------------------------ */

void CreateColorbarPropane(colorbardata *cbi){
// Propane
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "Propane");

  cbi->nnodes = 5;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 140;
  cbi->node_rgb[3] = 235;
  cbi->node_rgb[4] = 120;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 160;
  cbi->node_rgb[6] = 250;
  cbi->node_rgb[7] = 180;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 190;
  cbi->node_rgb[9] = 252;
  cbi->node_rgb[10] = 248;
  cbi->node_rgb[11] = 70;

  cbi->node_index[4] = 255;
  cbi->node_rgb[12] = 255;
  cbi->node_rgb[13] = 255;
  cbi->node_rgb[14] = 255;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
}

/* ------------------ CreateColorbarCo2 ------------------------ */

void CreateColorbarCo2(colorbardata *cbi){
// CO2
  memset(cbi, 0, sizeof(colorbardata));
  strcpy(cbi->menu_label, "CO2");

  cbi->nnodes = 3;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 192;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 0;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 255;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 255;
  strcpy(cbi->colorbar_type, "original");
  cbi->can_adjust = 0;
}
