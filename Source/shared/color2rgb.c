#include <string.h>
#include "getdata.h"

/* ------------------ color2rgb ------------------------ */

// Translate character string of a color name to RGB value
void color2rgb(int rgb[3], const char *color){

  if(strcmp(color, "ALICE BLUE") == 0){
    rgb[0] = 240;
    rgb[1] = 248;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "ANTIQUE WHITE") == 0){
    rgb[0] = 250;
    rgb[1] = 235;
    rgb[2] = 215;
    return;
  }
  if(strcmp(color, "ANTIQUE WHITE 1") == 0){
    rgb[0] = 255;
    rgb[1] = 239;
    rgb[2] = 219;
    return;
  }
  if(strcmp(color, "ANTIQUE WHITE 2") == 0){
    rgb[0] = 238;
    rgb[1] = 223;
    rgb[2] = 204;
    return;
  }
  if(strcmp(color, "ANTIQUE WHITE 3") == 0){
    rgb[0] = 205;
    rgb[1] = 192;
    rgb[2] = 176;
    return;
  }
  if(strcmp(color, "ANTIQUE WHITE 4") == 0){
    rgb[0] = 139;
    rgb[1] = 131;
    rgb[2] = 120;
    return;
  }
  if(strcmp(color, "AQUAMARINE") == 0){
    rgb[0] = 127;
    rgb[1] = 255;
    rgb[2] = 212;
    return;
  }
  if(strcmp(color, "AQUAMARINE 1") == 0){
    rgb[0] = 118;
    rgb[1] = 238;
    rgb[2] = 198;
    return;
  }
  if(strcmp(color, "AQUAMARINE 2") == 0){
    rgb[0] = 102;
    rgb[1] = 205;
    rgb[2] = 170;
    return;
  }
  if(strcmp(color, "AQUAMARINE 3") == 0){
    rgb[0] = 69;
    rgb[1] = 139;
    rgb[2] = 116;
    return;
  }
  if(strcmp(color, "AZURE") == 0){
    rgb[0] = 240;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "AZURE 1") == 0){
    rgb[0] = 224;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "AZURE 2") == 0){
    rgb[0] = 193;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "AZURE 3") == 0){
    rgb[0] = 131;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "BANANA") == 0){
    rgb[0] = 227;
    rgb[1] = 207;
    rgb[2] = 87;
    return;
  }
  if(strcmp(color, "BEIGE") == 0){
    rgb[0] = 245;
    rgb[1] = 245;
    rgb[2] = 220;
    return;
  }
  if(strcmp(color, "BISQUE") == 0){
    rgb[0] = 255;
    rgb[1] = 228;
    rgb[2] = 196;
    return;
  }
  if(strcmp(color, "BISQUE 1") == 0){
    rgb[0] = 238;
    rgb[1] = 213;
    rgb[2] = 183;
    return;
  }
  if(strcmp(color, "BISQUE 2") == 0){
    rgb[0] = 205;
    rgb[1] = 183;
    rgb[2] = 158;
    return;
  }
  if(strcmp(color, "BISQUE 3") == 0){
    rgb[0] = 139;
    rgb[1] = 125;
    rgb[2] = 107;
    return;
  }
  if(strcmp(color, "BLACK") == 0){
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "BLANCHED ALMOND") == 0){
    rgb[0] = 255;
    rgb[1] = 235;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "BLUE") == 0){
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "BLUE 2") == 0){
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "BLUE 3") == 0){
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "BLUE 4") == 0){
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "BLUE VIOLET") == 0){
    rgb[0] = 138;
    rgb[1] = 43;
    rgb[2] = 226;
    return;
  }
  if(strcmp(color, "BRICK") == 0){
    rgb[0] = 156;
    rgb[1] = 102;
    rgb[2] = 31;
    return;
  }
  if(strcmp(color, "BROWN") == 0){
    rgb[0] = 165;
    rgb[1] = 42;
    rgb[2] = 42;
    return;
  }
  if(strcmp(color, "BROWN 1") == 0){
    rgb[0] = 255;
    rgb[1] = 64;
    rgb[2] = 64;
    return;
  }
  if(strcmp(color, "BROWN 2") == 0){
    rgb[0] = 238;
    rgb[1] = 59;
    rgb[2] = 59;
    return;
  }
  if(strcmp(color, "BROWN 3") == 0){
    rgb[0] = 205;
    rgb[1] = 51;
    rgb[2] = 51;
    return;
  }
  if(strcmp(color, "BROWN 4") == 0){
    rgb[0] = 139;
    rgb[1] = 35;
    rgb[2] = 35;
    return;
  }
  if(strcmp(color, "BURLY WOOD") == 0){
    rgb[0] = 222;
    rgb[1] = 184;
    rgb[2] = 135;
    return;
  }
  if(strcmp(color, "BURLY WOOD 1") == 0){
    rgb[0] = 255;
    rgb[1] = 211;
    rgb[2] = 155;
    return;
  }
  if(strcmp(color, "BURLY WOOD 2") == 0){
    rgb[0] = 238;
    rgb[1] = 197;
    rgb[2] = 145;
    return;
  }
  if(strcmp(color, "BURLY WOOD 3") == 0){
    rgb[0] = 205;
    rgb[1] = 170;
    rgb[2] = 125;
    return;
  }
  if(strcmp(color, "BURLY WOOD 4") == 0){
    rgb[0] = 139;
    rgb[1] = 115;
    rgb[2] = 85;
    return;
  }
  if(strcmp(color, "BURNT ORANGE") == 0){
    rgb[0] = 204;
    rgb[1] = 85;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "BURNT SIENNA") == 0){
    rgb[0] = 138;
    rgb[1] = 54;
    rgb[2] = 15;
    return;
  }
  if(strcmp(color, "BURNT UMBER") == 0){
    rgb[0] = 138;
    rgb[1] = 51;
    rgb[2] = 36;
    return;
  }
  if(strcmp(color, "CADET BLUE") == 0){
    rgb[0] = 95;
    rgb[1] = 158;
    rgb[2] = 160;
    return;
  }
  if(strcmp(color, "CADET BLUE 1") == 0){
    rgb[0] = 152;
    rgb[1] = 245;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "CADET BLUE 2") == 0){
    rgb[0] = 142;
    rgb[1] = 229;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "CADET BLUE 3") == 0){
    rgb[0] = 122;
    rgb[1] = 197;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "CADET BLUE 4") == 0){
    rgb[0] = 83;
    rgb[1] = 134;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "CADMIUM ORANGE") == 0){
    rgb[0] = 255;
    rgb[1] = 97;
    rgb[2] = 3;
    return;
  }
  if(strcmp(color, "CADMIUM YELLOW") == 0){
    rgb[0] = 255;
    rgb[1] = 153;
    rgb[2] = 18;
    return;
  }
  if(strcmp(color, "CARROT") == 0){
    rgb[0] = 237;
    rgb[1] = 145;
    rgb[2] = 33;
    return;
  }
  if(strcmp(color, "CHARTREUSE") == 0){
    rgb[0] = 127;
    rgb[1] = 255;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "CHARTREUSE 1") == 0){
    rgb[0] = 118;
    rgb[1] = 238;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "CHARTREUSE 2") == 0){
    rgb[0] = 102;
    rgb[1] = 205;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "CHARTREUSE 3") == 0){
    rgb[0] = 69;
    rgb[1] = 139;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "CHOCOLATE") == 0){
    rgb[0] = 210;
    rgb[1] = 105;
    rgb[2] = 30;
    return;
  }
  if(strcmp(color, "CHOCOLATE 1") == 0){
    rgb[0] = 255;
    rgb[1] = 127;
    rgb[2] = 36;
    return;
  }
  if(strcmp(color, "CHOCOLATE 2") == 0){
    rgb[0] = 238;
    rgb[1] = 118;
    rgb[2] = 33;
    return;
  }
  if(strcmp(color, "CHOCOLATE 3") == 0){
    rgb[0] = 205;
    rgb[1] = 102;
    rgb[2] = 29;
    return;
  }
  if(strcmp(color, "CHOCOLATE 4") == 0){
    rgb[0] = 139;
    rgb[1] = 69;
    rgb[2] = 19;
    return;
  }
  if(strcmp(color, "COBALT") == 0){
    rgb[0] = 61;
    rgb[1] = 89;
    rgb[2] = 171;
    return;
  }
  if(strcmp(color, "COBALT GREEN") == 0){
    rgb[0] = 61;
    rgb[1] = 145;
    rgb[2] = 64;
    return;
  }
  if(strcmp(color, "COLD GREY") == 0){
    rgb[0] = 128;
    rgb[1] = 138;
    rgb[2] = 135;
    return;
  }
  if(strcmp(color, "CORAL") == 0){
    rgb[0] = 255;
    rgb[1] = 127;
    rgb[2] = 80;
    return;
  }
  if(strcmp(color, "CORAL 1") == 0){
    rgb[0] = 255;
    rgb[1] = 114;
    rgb[2] = 86;
    return;
  }
  if(strcmp(color, "CORAL 2") == 0){
    rgb[0] = 238;
    rgb[1] = 106;
    rgb[2] = 80;
    return;
  }
  if(strcmp(color, "CORAL 3") == 0){
    rgb[0] = 205;
    rgb[1] = 91;
    rgb[2] = 69;
    return;
  }
  if(strcmp(color, "CORAL 4") == 0){
    rgb[0] = 139;
    rgb[1] = 62;
    rgb[2] = 47;
    return;
  }
  if(strcmp(color, "CORNFLOWER BLUE") == 0){
    rgb[0] = 100;
    rgb[1] = 149;
    rgb[2] = 237;
    return;
  }
  if(strcmp(color, "CORNSILK") == 0){
    rgb[0] = 255;
    rgb[1] = 248;
    rgb[2] = 220;
    return;
  }
  if(strcmp(color, "CORNSILK 1") == 0){
    rgb[0] = 238;
    rgb[1] = 232;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "CORNSILK 2") == 0){
    rgb[0] = 205;
    rgb[1] = 200;
    rgb[2] = 177;
    return;
  }
  if(strcmp(color, "CORNSILK 3") == 0){
    rgb[0] = 139;
    rgb[1] = 136;
    rgb[2] = 120;
    return;
  }
  if(strcmp(color, "CRIMSON") == 0){
    rgb[0] = 220;
    rgb[1] = 20;
    rgb[2] = 60;
    return;
  }
  if(strcmp(color, "CYAN") == 0){
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "CYAN 2") == 0){
    rgb[0] = 0;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "CYAN 3") == 0){
    rgb[0] = 0;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "CYAN 4") == 0){
    rgb[0] = 0;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "DARK GOLDENROD") == 0){
    rgb[0] = 184;
    rgb[1] = 134;
    rgb[2] = 11;
    return;
  }
  if(strcmp(color, "DARK GOLDENROD 1") == 0){
    rgb[0] = 255;
    rgb[1] = 185;
    rgb[2] = 15;
    return;
  }
  if(strcmp(color, "DARK GOLDENROD 2") == 0){
    rgb[0] = 238;
    rgb[1] = 173;
    rgb[2] = 14;
    return;
  }
  if(strcmp(color, "DARK GOLDENROD 3") == 0){
    rgb[0] = 205;
    rgb[1] = 149;
    rgb[2] = 12;
    return;
  }
  if(strcmp(color, "DARK GOLDENROD 4") == 0){
    rgb[0] = 139;
    rgb[1] = 101;
    rgb[2] = 8;
    return;
  }
  if(strcmp(color, "DARK GRAY") == 0){
    rgb[0] = 169;
    rgb[1] = 169;
    rgb[2] = 169;
    return;
  }
  if(strcmp(color, "DARK GREEN") == 0){
    rgb[0] = 0;
    rgb[1] = 100;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "DARK KHAKI") == 0){
    rgb[0] = 189;
    rgb[1] = 183;
    rgb[2] = 107;
    return;
  }
  if(strcmp(color, "DARK OLIVE GREEN") == 0){
    rgb[0] = 85;
    rgb[1] = 107;
    rgb[2] = 47;
    return;
  }
  if(strcmp(color, "DARK OLIVE GREEN 1") == 0){
    rgb[0] = 202;
    rgb[1] = 255;
    rgb[2] = 112;
    return;
  }
  if(strcmp(color, "DARK OLIVE GREEN 2") == 0){
    rgb[0] = 188;
    rgb[1] = 238;
    rgb[2] = 104;
    return;
  }
  if(strcmp(color, "DARK OLIVE GREEN 3") == 0){
    rgb[0] = 162;
    rgb[1] = 205;
    rgb[2] = 90;
    return;
  }
  if(strcmp(color, "DARK OLIVE GREEN 4") == 0){
    rgb[0] = 110;
    rgb[1] = 139;
    rgb[2] = 61;
    return;
  }
  if(strcmp(color, "DARK ORANGE") == 0){
    rgb[0] = 255;
    rgb[1] = 140;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "DARK ORANGE 1") == 0){
    rgb[0] = 255;
    rgb[1] = 127;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "DARK ORANGE 2") == 0){
    rgb[0] = 238;
    rgb[1] = 118;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "DARK ORANGE 3") == 0){
    rgb[0] = 205;
    rgb[1] = 102;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "DARK ORANGE 4") == 0){
    rgb[0] = 139;
    rgb[1] = 69;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "DARK ORCHID") == 0){
    rgb[0] = 153;
    rgb[1] = 50;
    rgb[2] = 204;
    return;
  }
  if(strcmp(color, "DARK ORCHID 1") == 0){
    rgb[0] = 191;
    rgb[1] = 62;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "DARK ORCHID 2") == 0){
    rgb[0] = 178;
    rgb[1] = 58;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "DARK ORCHID 3") == 0){
    rgb[0] = 154;
    rgb[1] = 50;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "DARK ORCHID 4") == 0){
    rgb[0] = 104;
    rgb[1] = 34;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "DARK SALMON") == 0){
    rgb[0] = 233;
    rgb[1] = 150;
    rgb[2] = 122;
    return;
  }
  if(strcmp(color, "DARK SEA GREEN") == 0){
    rgb[0] = 143;
    rgb[1] = 188;
    rgb[2] = 143;
    return;
  }
  if(strcmp(color, "DARK SEA GREEN 1") == 0){
    rgb[0] = 193;
    rgb[1] = 255;
    rgb[2] = 193;
    return;
  }
  if(strcmp(color, "DARK SEA GREEN 2") == 0){
    rgb[0] = 180;
    rgb[1] = 238;
    rgb[2] = 180;
    return;
  }
  if(strcmp(color, "DARK SEA GREEN 3") == 0){
    rgb[0] = 155;
    rgb[1] = 205;
    rgb[2] = 155;
    return;
  }
  if(strcmp(color, "DARK SEA GREEN 4") == 0){
    rgb[0] = 105;
    rgb[1] = 139;
    rgb[2] = 105;
    return;
  }
  if(strcmp(color, "DARK SLATE BLUE") == 0){
    rgb[0] = 72;
    rgb[1] = 61;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "DARK SLATE GRAY") == 0){
    rgb[0] = 47;
    rgb[1] = 79;
    rgb[2] = 79;
    return;
  }
  if(strcmp(color, "DARK SLATE GRAY 1") == 0){
    rgb[0] = 151;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "DARK SLATE GRAY 2") == 0){
    rgb[0] = 141;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "DARK SLATE GRAY 3") == 0){
    rgb[0] = 121;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "DARK SLATE GRAY 4") == 0){
    rgb[0] = 82;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "DARK TURQUOISE") == 0){
    rgb[0] = 0;
    rgb[1] = 206;
    rgb[2] = 209;
    return;
  }
  if(strcmp(color, "DARK VIOLET") == 0){
    rgb[0] = 148;
    rgb[1] = 0;
    rgb[2] = 211;
    return;
  }
  if(strcmp(color, "DEEP PINK") == 0){
    rgb[0] = 255;
    rgb[1] = 20;
    rgb[2] = 147;
    return;
  }
  if(strcmp(color, "DEEP PINK 1") == 0){
    rgb[0] = 238;
    rgb[1] = 18;
    rgb[2] = 137;
    return;
  }
  if(strcmp(color, "DEEP PINK 2") == 0){
    rgb[0] = 205;
    rgb[1] = 16;
    rgb[2] = 118;
    return;
  }
  if(strcmp(color, "DEEP PINK 3") == 0){
    rgb[0] = 139;
    rgb[1] = 10;
    rgb[2] = 80;
    return;
  }
  if(strcmp(color, "DEEP SKYBLUE") == 0){
    rgb[0] = 0;
    rgb[1] = 191;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "DEEP SKYBLUE 1") == 0){
    rgb[0] = 0;
    rgb[1] = 178;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "DEEP SKYBLUE 2") == 0){
    rgb[0] = 0;
    rgb[1] = 154;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "DEEP SKYBLUE 3") == 0){
    rgb[0] = 0;
    rgb[1] = 104;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "DIM GRAY") == 0){
    rgb[0] = 105;
    rgb[1] = 105;
    rgb[2] = 105;
    return;
  }
  if(strcmp(color, "DODGERBLUE") == 0){
    rgb[0] = 30;
    rgb[1] = 144;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "DODGERBLUE 1") == 0){
    rgb[0] = 28;
    rgb[1] = 134;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "DODGERBLUE 2") == 0){
    rgb[0] = 24;
    rgb[1] = 116;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "DODGERBLUE 3") == 0){
    rgb[0] = 16;
    rgb[1] = 78;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "EGGSHELL") == 0){
    rgb[0] = 252;
    rgb[1] = 230;
    rgb[2] = 201;
    return;
  }
  if(strcmp(color, "EMERALD GREEN") == 0){
    rgb[0] = 0;
    rgb[1] = 201;
    rgb[2] = 87;
    return;
  }
  if(strcmp(color, "FIREBRICK") == 0){
    rgb[0] = 178;
    rgb[1] = 34;
    rgb[2] = 34;
    return;
  }
  if(strcmp(color, "FIREBRICK 1") == 0){
    rgb[0] = 255;
    rgb[1] = 48;
    rgb[2] = 48;
    return;
  }
  if(strcmp(color, "FIREBRICK 2") == 0){
    rgb[0] = 238;
    rgb[1] = 44;
    rgb[2] = 44;
    return;
  }
  if(strcmp(color, "FIREBRICK 3") == 0){
    rgb[0] = 205;
    rgb[1] = 38;
    rgb[2] = 38;
    return;
  }
  if(strcmp(color, "FIREBRICK 4") == 0){
    rgb[0] = 139;
    rgb[1] = 26;
    rgb[2] = 26;
    return;
  }
  if(strcmp(color, "FLESH") == 0){
    rgb[0] = 255;
    rgb[1] = 125;
    rgb[2] = 64;
    return;
  }
  if(strcmp(color, "FLORAL WHITE") == 0){
    rgb[0] = 255;
    rgb[1] = 250;
    rgb[2] = 240;
    return;
  }
  if(strcmp(color, "FOREST GREEN") == 0){
    rgb[0] = 34;
    rgb[1] = 139;
    rgb[2] = 34;
    return;
  }
  if(strcmp(color, "GAINSBORO") == 0){
    rgb[0] = 220;
    rgb[1] = 220;
    rgb[2] = 220;
    return;
  }
  if(strcmp(color, "GHOST WHITE") == 0){
    rgb[0] = 248;
    rgb[1] = 248;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "GOLD") == 0){
    rgb[0] = 255;
    rgb[1] = 215;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GOLD 1") == 0){
    rgb[0] = 238;
    rgb[1] = 201;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GOLD 2") == 0){
    rgb[0] = 205;
    rgb[1] = 173;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GOLD 3") == 0){
    rgb[0] = 139;
    rgb[1] = 117;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GOLDENROD") == 0){
    rgb[0] = 218;
    rgb[1] = 165;
    rgb[2] = 32;
    return;
  }
  if(strcmp(color, "GOLDENROD 1") == 0){
    rgb[0] = 255;
    rgb[1] = 193;
    rgb[2] = 37;
    return;
  }
  if(strcmp(color, "GOLDENROD 2") == 0){
    rgb[0] = 238;
    rgb[1] = 180;
    rgb[2] = 34;
    return;
  }
  if(strcmp(color, "GOLDENROD 3") == 0){
    rgb[0] = 205;
    rgb[1] = 155;
    rgb[2] = 29;
    return;
  }
  if(strcmp(color, "GOLDENROD 4") == 0){
    rgb[0] = 139;
    rgb[1] = 105;
    rgb[2] = 20;
    return;
  }
  if(strcmp(color, "GRAY") == 0){
    rgb[0] = 128;
    rgb[1] = 128;
    rgb[2] = 128;
    return;
  }
  if(strcmp(color, "GRAY 1") == 0){
    rgb[0] = 3;
    rgb[1] = 3;
    rgb[2] = 3;
    return;
  }
  if(strcmp(color, "GRAY 10") == 0){
    rgb[0] = 26;
    rgb[1] = 26;
    rgb[2] = 26;
    return;
  }
  if(strcmp(color, "GRAY 11") == 0){
    rgb[0] = 28;
    rgb[1] = 28;
    rgb[2] = 28;
    return;
  }
  if(strcmp(color, "GRAY 12") == 0){
    rgb[0] = 31;
    rgb[1] = 31;
    rgb[2] = 31;
    return;
  }
  if(strcmp(color, "GRAY 13") == 0){
    rgb[0] = 33;
    rgb[1] = 33;
    rgb[2] = 33;
    return;
  }
  if(strcmp(color, "GRAY 14") == 0){
    rgb[0] = 36;
    rgb[1] = 36;
    rgb[2] = 36;
    return;
  }
  if(strcmp(color, "GRAY 15") == 0){
    rgb[0] = 38;
    rgb[1] = 38;
    rgb[2] = 38;
    return;
  }
  if(strcmp(color, "GRAY 16") == 0){
    rgb[0] = 41;
    rgb[1] = 41;
    rgb[2] = 41;
    return;
  }
  if(strcmp(color, "GRAY 17") == 0){
    rgb[0] = 43;
    rgb[1] = 43;
    rgb[2] = 43;
    return;
  }
  if(strcmp(color, "GRAY 18") == 0){
    rgb[0] = 46;
    rgb[1] = 46;
    rgb[2] = 46;
    return;
  }
  if(strcmp(color, "GRAY 19") == 0){
    rgb[0] = 48;
    rgb[1] = 48;
    rgb[2] = 48;
    return;
  }
  if(strcmp(color, "GRAY 2") == 0){
    rgb[0] = 5;
    rgb[1] = 5;
    rgb[2] = 5;
    return;
  }
  if(strcmp(color, "GRAY 20") == 0){
    rgb[0] = 51;
    rgb[1] = 51;
    rgb[2] = 51;
    return;
  }
  if(strcmp(color, "GRAY 21") == 0){
    rgb[0] = 54;
    rgb[1] = 54;
    rgb[2] = 54;
    return;
  }
  if(strcmp(color, "GRAY 22") == 0){
    rgb[0] = 56;
    rgb[1] = 56;
    rgb[2] = 56;
    return;
  }
  if(strcmp(color, "GRAY 23") == 0){
    rgb[0] = 59;
    rgb[1] = 59;
    rgb[2] = 59;
    return;
  }
  if(strcmp(color, "GRAY 24") == 0){
    rgb[0] = 61;
    rgb[1] = 61;
    rgb[2] = 61;
    return;
  }
  if(strcmp(color, "GRAY 25") == 0){
    rgb[0] = 64;
    rgb[1] = 64;
    rgb[2] = 64;
    return;
  }
  if(strcmp(color, "GRAY 26") == 0){
    rgb[0] = 66;
    rgb[1] = 66;
    rgb[2] = 66;
    return;
  }
  if(strcmp(color, "GRAY 27") == 0){
    rgb[0] = 69;
    rgb[1] = 69;
    rgb[2] = 69;
    return;
  }
  if(strcmp(color, "GRAY 28") == 0){
    rgb[0] = 71;
    rgb[1] = 71;
    rgb[2] = 71;
    return;
  }
  if(strcmp(color, "GRAY 29") == 0){
    rgb[0] = 74;
    rgb[1] = 74;
    rgb[2] = 74;
    return;
  }
  if(strcmp(color, "GRAY 3") == 0){
    rgb[0] = 8;
    rgb[1] = 8;
    rgb[2] = 8;
    return;
  }
  if(strcmp(color, "GRAY 30") == 0){
    rgb[0] = 77;
    rgb[1] = 77;
    rgb[2] = 77;
    return;
  }
  if(strcmp(color, "GRAY 31") == 0){
    rgb[0] = 79;
    rgb[1] = 79;
    rgb[2] = 79;
    return;
  }
  if(strcmp(color, "GRAY 32") == 0){
    rgb[0] = 82;
    rgb[1] = 82;
    rgb[2] = 82;
    return;
  }
  if(strcmp(color, "GRAY 33") == 0){
    rgb[0] = 84;
    rgb[1] = 84;
    rgb[2] = 84;
    return;
  }
  if(strcmp(color, "GRAY 34") == 0){
    rgb[0] = 87;
    rgb[1] = 87;
    rgb[2] = 87;
    return;
  }
  if(strcmp(color, "GRAY 35") == 0){
    rgb[0] = 89;
    rgb[1] = 89;
    rgb[2] = 89;
    return;
  }
  if(strcmp(color, "GRAY 36") == 0){
    rgb[0] = 92;
    rgb[1] = 92;
    rgb[2] = 92;
    return;
  }
  if(strcmp(color, "GRAY 37") == 0){
    rgb[0] = 94;
    rgb[1] = 94;
    rgb[2] = 94;
    return;
  }
  if(strcmp(color, "GRAY 38") == 0){
    rgb[0] = 97;
    rgb[1] = 97;
    rgb[2] = 97;
    return;
  }
  if(strcmp(color, "GRAY 39") == 0){
    rgb[0] = 99;
    rgb[1] = 99;
    rgb[2] = 99;
    return;
  }
  if(strcmp(color, "GRAY 4") == 0){
    rgb[0] = 10;
    rgb[1] = 10;
    rgb[2] = 10;
    return;
  }
  if(strcmp(color, "GRAY 40") == 0){
    rgb[0] = 102;
    rgb[1] = 102;
    rgb[2] = 102;
    return;
  }
  if(strcmp(color, "GRAY 42") == 0){
    rgb[0] = 107;
    rgb[1] = 107;
    rgb[2] = 107;
    return;
  }
  if(strcmp(color, "GRAY 43") == 0){
    rgb[0] = 110;
    rgb[1] = 110;
    rgb[2] = 110;
    return;
  }
  if(strcmp(color, "GRAY 44") == 0){
    rgb[0] = 112;
    rgb[1] = 112;
    rgb[2] = 112;
    return;
  }
  if(strcmp(color, "GRAY 45") == 0){
    rgb[0] = 115;
    rgb[1] = 115;
    rgb[2] = 115;
    return;
  }
  if(strcmp(color, "GRAY 46") == 0){
    rgb[0] = 117;
    rgb[1] = 117;
    rgb[2] = 117;
    return;
  }
  if(strcmp(color, "GRAY 47") == 0){
    rgb[0] = 120;
    rgb[1] = 120;
    rgb[2] = 120;
    return;
  }
  if(strcmp(color, "GRAY 48") == 0){
    rgb[0] = 122;
    rgb[1] = 122;
    rgb[2] = 122;
    return;
  }
  if(strcmp(color, "GRAY 49") == 0){
    rgb[0] = 125;
    rgb[1] = 125;
    rgb[2] = 125;
    return;
  }
  if(strcmp(color, "GRAY 5") == 0){
    rgb[0] = 13;
    rgb[1] = 13;
    rgb[2] = 13;
    return;
  }
  if(strcmp(color, "GRAY 50") == 0){
    rgb[0] = 127;
    rgb[1] = 127;
    rgb[2] = 127;
    return;
  }
  if(strcmp(color, "GRAY 51") == 0){
    rgb[0] = 130;
    rgb[1] = 130;
    rgb[2] = 130;
    return;
  }
  if(strcmp(color, "GRAY 52") == 0){
    rgb[0] = 133;
    rgb[1] = 133;
    rgb[2] = 133;
    return;
  }
  if(strcmp(color, "GRAY 53") == 0){
    rgb[0] = 135;
    rgb[1] = 135;
    rgb[2] = 135;
    return;
  }
  if(strcmp(color, "GRAY 54") == 0){
    rgb[0] = 138;
    rgb[1] = 138;
    rgb[2] = 138;
    return;
  }
  if(strcmp(color, "GRAY 55") == 0){
    rgb[0] = 140;
    rgb[1] = 140;
    rgb[2] = 140;
    return;
  }
  if(strcmp(color, "GRAY 56") == 0){
    rgb[0] = 143;
    rgb[1] = 143;
    rgb[2] = 143;
    return;
  }
  if(strcmp(color, "GRAY 57") == 0){
    rgb[0] = 145;
    rgb[1] = 145;
    rgb[2] = 145;
    return;
  }
  if(strcmp(color, "GRAY 58") == 0){
    rgb[0] = 148;
    rgb[1] = 148;
    rgb[2] = 148;
    return;
  }
  if(strcmp(color, "GRAY 59") == 0){
    rgb[0] = 150;
    rgb[1] = 150;
    rgb[2] = 150;
    return;
  }
  if(strcmp(color, "GRAY 6") == 0){
    rgb[0] = 15;
    rgb[1] = 15;
    rgb[2] = 15;
    return;
  }
  if(strcmp(color, "GRAY 60") == 0){
    rgb[0] = 153;
    rgb[1] = 153;
    rgb[2] = 153;
    return;
  }
  if(strcmp(color, "GRAY 61") == 0){
    rgb[0] = 156;
    rgb[1] = 156;
    rgb[2] = 156;
    return;
  }
  if(strcmp(color, "GRAY 62") == 0){
    rgb[0] = 158;
    rgb[1] = 158;
    rgb[2] = 158;
    return;
  }
  if(strcmp(color, "GRAY 63") == 0){
    rgb[0] = 161;
    rgb[1] = 161;
    rgb[2] = 161;
    return;
  }
  if(strcmp(color, "GRAY 64") == 0){
    rgb[0] = 163;
    rgb[1] = 163;
    rgb[2] = 163;
    return;
  }
  if(strcmp(color, "GRAY 65") == 0){
    rgb[0] = 166;
    rgb[1] = 166;
    rgb[2] = 166;
    return;
  }
  if(strcmp(color, "GRAY 66") == 0){
    rgb[0] = 168;
    rgb[1] = 168;
    rgb[2] = 168;
    return;
  }
  if(strcmp(color, "GRAY 67") == 0){
    rgb[0] = 171;
    rgb[1] = 171;
    rgb[2] = 171;
    return;
  }
  if(strcmp(color, "GRAY 68") == 0){
    rgb[0] = 173;
    rgb[1] = 173;
    rgb[2] = 173;
    return;
  }
  if(strcmp(color, "GRAY 69") == 0){
    rgb[0] = 176;
    rgb[1] = 176;
    rgb[2] = 176;
    return;
  }
  if(strcmp(color, "GRAY 7") == 0){
    rgb[0] = 18;
    rgb[1] = 18;
    rgb[2] = 18;
    return;
  }
  if(strcmp(color, "GRAY 70") == 0){
    rgb[0] = 179;
    rgb[1] = 179;
    rgb[2] = 179;
    return;
  }
  if(strcmp(color, "GRAY 71") == 0){
    rgb[0] = 181;
    rgb[1] = 181;
    rgb[2] = 181;
    return;
  }
  if(strcmp(color, "GRAY 72") == 0){
    rgb[0] = 184;
    rgb[1] = 184;
    rgb[2] = 184;
    return;
  }
  if(strcmp(color, "GRAY 73") == 0){
    rgb[0] = 186;
    rgb[1] = 186;
    rgb[2] = 186;
    return;
  }
  if(strcmp(color, "GRAY 74") == 0){
    rgb[0] = 189;
    rgb[1] = 189;
    rgb[2] = 189;
    return;
  }
  if(strcmp(color, "GRAY 75") == 0){
    rgb[0] = 191;
    rgb[1] = 191;
    rgb[2] = 191;
    return;
  }
  if(strcmp(color, "GRAY 76") == 0){
    rgb[0] = 194;
    rgb[1] = 194;
    rgb[2] = 194;
    return;
  }
  if(strcmp(color, "GRAY 77") == 0){
    rgb[0] = 196;
    rgb[1] = 196;
    rgb[2] = 196;
    return;
  }
  if(strcmp(color, "GRAY 78") == 0){
    rgb[0] = 199;
    rgb[1] = 199;
    rgb[2] = 199;
    return;
  }
  if(strcmp(color, "GRAY 79") == 0){
    rgb[0] = 201;
    rgb[1] = 201;
    rgb[2] = 201;
    return;
  }
  if(strcmp(color, "GRAY 8") == 0){
    rgb[0] = 20;
    rgb[1] = 20;
    rgb[2] = 20;
    return;
  }
  if(strcmp(color, "GRAY 80") == 0){
    rgb[0] = 204;
    rgb[1] = 204;
    rgb[2] = 204;
    return;
  }
  if(strcmp(color, "GRAY 81") == 0){
    rgb[0] = 207;
    rgb[1] = 207;
    rgb[2] = 207;
    return;
  }
  if(strcmp(color, "GRAY 82") == 0){
    rgb[0] = 209;
    rgb[1] = 209;
    rgb[2] = 209;
    return;
  }
  if(strcmp(color, "GRAY 83") == 0){
    rgb[0] = 212;
    rgb[1] = 212;
    rgb[2] = 212;
    return;
  }
  if(strcmp(color, "GRAY 84") == 0){
    rgb[0] = 214;
    rgb[1] = 214;
    rgb[2] = 214;
    return;
  }
  if(strcmp(color, "GRAY 85") == 0){
    rgb[0] = 217;
    rgb[1] = 217;
    rgb[2] = 217;
    return;
  }
  if(strcmp(color, "GRAY 86") == 0){
    rgb[0] = 219;
    rgb[1] = 219;
    rgb[2] = 219;
    return;
  }
  if(strcmp(color, "GRAY 87") == 0){
    rgb[0] = 222;
    rgb[1] = 222;
    rgb[2] = 222;
    return;
  }
  if(strcmp(color, "GRAY 88") == 0){
    rgb[0] = 224;
    rgb[1] = 224;
    rgb[2] = 224;
    return;
  }
  if(strcmp(color, "GRAY 89") == 0){
    rgb[0] = 227;
    rgb[1] = 227;
    rgb[2] = 227;
    return;
  }
  if(strcmp(color, "GRAY 9") == 0){
    rgb[0] = 23;
    rgb[1] = 23;
    rgb[2] = 23;
    return;
  }
  if(strcmp(color, "GRAY 90") == 0){
    rgb[0] = 229;
    rgb[1] = 229;
    rgb[2] = 229;
    return;
  }
  if(strcmp(color, "GRAY 91") == 0){
    rgb[0] = 232;
    rgb[1] = 232;
    rgb[2] = 232;
    return;
  }
  if(strcmp(color, "GRAY 92") == 0){
    rgb[0] = 235;
    rgb[1] = 235;
    rgb[2] = 235;
    return;
  }
  if(strcmp(color, "GRAY 93") == 0){
    rgb[0] = 237;
    rgb[1] = 237;
    rgb[2] = 237;
    return;
  }
  if(strcmp(color, "GRAY 94") == 0){
    rgb[0] = 240;
    rgb[1] = 240;
    rgb[2] = 240;
    return;
  }
  if(strcmp(color, "GRAY 95") == 0){
    rgb[0] = 242;
    rgb[1] = 242;
    rgb[2] = 242;
    return;
  }
  if(strcmp(color, "GRAY 97") == 0){
    rgb[0] = 247;
    rgb[1] = 247;
    rgb[2] = 247;
    return;
  }
  if(strcmp(color, "GRAY 98") == 0){
    rgb[0] = 250;
    rgb[1] = 250;
    rgb[2] = 250;
    return;
  }
  if(strcmp(color, "GRAY 99") == 0){
    rgb[0] = 252;
    rgb[1] = 252;
    rgb[2] = 252;
    return;
  }
  if(strcmp(color, "GREEN") == 0){
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GREEN 2") == 0){
    rgb[0] = 0;
    rgb[1] = 238;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GREEN 3") == 0){
    rgb[0] = 0;
    rgb[1] = 205;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GREEN 4") == 0){
    rgb[0] = 0;
    rgb[1] = 139;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "GREEN YELLOW") == 0){
    rgb[0] = 173;
    rgb[1] = 255;
    rgb[2] = 47;
    return;
  }
  if(strcmp(color, "HONEYDEW") == 0){
    rgb[0] = 240;
    rgb[1] = 255;
    rgb[2] = 240;
    return;
  }
  if(strcmp(color, "HONEYDEW 1") == 0){
    rgb[0] = 224;
    rgb[1] = 238;
    rgb[2] = 224;
    return;
  }
  if(strcmp(color, "HONEYDEW 2") == 0){
    rgb[0] = 193;
    rgb[1] = 205;
    rgb[2] = 193;
    return;
  }
  if(strcmp(color, "HONEYDEW 3") == 0){
    rgb[0] = 131;
    rgb[1] = 139;
    rgb[2] = 131;
    return;
  }
  if(strcmp(color, "HOT PINK") == 0){
    rgb[0] = 255;
    rgb[1] = 105;
    rgb[2] = 180;
    return;
  }
  if(strcmp(color, "HOT PINK 1") == 0){
    rgb[0] = 255;
    rgb[1] = 110;
    rgb[2] = 180;
    return;
  }
  if(strcmp(color, "HOT PINK 2") == 0){
    rgb[0] = 238;
    rgb[1] = 106;
    rgb[2] = 167;
    return;
  }
  if(strcmp(color, "HOT PINK 3") == 0){
    rgb[0] = 205;
    rgb[1] = 96;
    rgb[2] = 144;
    return;
  }
  if(strcmp(color, "HOT PINK 4") == 0){
    rgb[0] = 139;
    rgb[1] = 58;
    rgb[2] = 98;
    return;
  }
  if(strcmp(color, "INDIAN RED") == 0){
    rgb[0] = 205;
    rgb[1] = 92;
    rgb[2] = 92;
    return;
  }
  if(strcmp(color, "INDIAN RED 1") == 0){
    rgb[0] = 255;
    rgb[1] = 106;
    rgb[2] = 106;
    return;
  }
  if(strcmp(color, "INDIAN RED 2") == 0){
    rgb[0] = 238;
    rgb[1] = 99;
    rgb[2] = 99;
    return;
  }
  if(strcmp(color, "INDIAN RED 3") == 0){
    rgb[0] = 205;
    rgb[1] = 85;
    rgb[2] = 85;
    return;
  }
  if(strcmp(color, "INDIAN RED 4") == 0){
    rgb[0] = 139;
    rgb[1] = 58;
    rgb[2] = 58;
    return;
  }
  if(strcmp(color, "INDIGO") == 0){
    rgb[0] = 75;
    rgb[1] = 0;
    rgb[2] = 130;
    return;
  }
  if(strcmp(color, "IVORY") == 0){
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 240;
    return;
  }
  if(strcmp(color, "IVORY 1") == 0){
    rgb[0] = 238;
    rgb[1] = 238;
    rgb[2] = 224;
    return;
  }
  if(strcmp(color, "IVORY 2") == 0){
    rgb[0] = 205;
    rgb[1] = 205;
    rgb[2] = 193;
    return;
  }
  if(strcmp(color, "IVORY 3") == 0){
    rgb[0] = 139;
    rgb[1] = 139;
    rgb[2] = 131;
    return;
  }
  if(strcmp(color, "IVORY BLACK") == 0){
    rgb[0] = 41;
    rgb[1] = 36;
    rgb[2] = 33;
    return;
  }
  if(strcmp(color, "KELLY GREEN") == 0){
    rgb[0] = 0;
    rgb[1] = 128;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "KHAKI") == 0){
    rgb[0] = 240;
    rgb[1] = 230;
    rgb[2] = 140;
    return;
  }
  if(strcmp(color, "KHAKI 1") == 0){
    rgb[0] = 255;
    rgb[1] = 246;
    rgb[2] = 143;
    return;
  }
  if(strcmp(color, "KHAKI 2") == 0){
    rgb[0] = 238;
    rgb[1] = 230;
    rgb[2] = 133;
    return;
  }
  if(strcmp(color, "KHAKI 3") == 0){
    rgb[0] = 205;
    rgb[1] = 198;
    rgb[2] = 115;
    return;
  }
  if(strcmp(color, "KHAKI 4") == 0){
    rgb[0] = 139;
    rgb[1] = 134;
    rgb[2] = 78;
    return;
  }
  if(strcmp(color, "LAVENDER") == 0){
    rgb[0] = 230;
    rgb[1] = 230;
    rgb[2] = 250;
    return;
  }
  if(strcmp(color, "LAVENDER BLUSH") == 0){
    rgb[0] = 255;
    rgb[1] = 240;
    rgb[2] = 245;
    return;
  }
  if(strcmp(color, "LAVENDER BLUSH 1") == 0){
    rgb[0] = 238;
    rgb[1] = 224;
    rgb[2] = 229;
    return;
  }
  if(strcmp(color, "LAVENDER BLUSH 2") == 0){
    rgb[0] = 205;
    rgb[1] = 193;
    rgb[2] = 197;
    return;
  }
  if(strcmp(color, "LAVENDER BLUSH 3") == 0){
    rgb[0] = 139;
    rgb[1] = 131;
    rgb[2] = 134;
    return;
  }
  if(strcmp(color, "LAWN GREEN") == 0){
    rgb[0] = 124;
    rgb[1] = 252;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "LEMON CHIFFON") == 0){
    rgb[0] = 255;
    rgb[1] = 250;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "LEMON CHIFFON 1") == 0){
    rgb[0] = 238;
    rgb[1] = 233;
    rgb[2] = 191;
    return;
  }
  if(strcmp(color, "LEMON CHIFFON 2") == 0){
    rgb[0] = 205;
    rgb[1] = 201;
    rgb[2] = 165;
    return;
  }
  if(strcmp(color, "LEMON CHIFFON 3") == 0){
    rgb[0] = 139;
    rgb[1] = 137;
    rgb[2] = 112;
    return;
  }
  if(strcmp(color, "LIGHT BLUE") == 0){
    rgb[0] = 173;
    rgb[1] = 216;
    rgb[2] = 230;
    return;
  }
  if(strcmp(color, "LIGHT BLUE 1") == 0){
    rgb[0] = 191;
    rgb[1] = 239;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "LIGHT BLUE 2") == 0){
    rgb[0] = 178;
    rgb[1] = 223;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "LIGHT BLUE 3") == 0){
    rgb[0] = 154;
    rgb[1] = 192;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "LIGHT BLUE 4") == 0){
    rgb[0] = 104;
    rgb[1] = 131;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "LIGHT CORAL") == 0){
    rgb[0] = 240;
    rgb[1] = 128;
    rgb[2] = 128;
    return;
  }
  if(strcmp(color, "LIGHT CYAN") == 0){
    rgb[0] = 224;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "LIGHT CYAN 1") == 0){
    rgb[0] = 209;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "LIGHT CYAN 2") == 0){
    rgb[0] = 180;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "LIGHT CYAN 3") == 0){
    rgb[0] = 122;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "LIGHT GOLDENROD") == 0){
    rgb[0] = 255;
    rgb[1] = 236;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "LIGHT GOLDENROD 1") == 0){
    rgb[0] = 238;
    rgb[1] = 220;
    rgb[2] = 130;
    return;
  }
  if(strcmp(color, "LIGHT GOLDENROD 2") == 0){
    rgb[0] = 205;
    rgb[1] = 190;
    rgb[2] = 112;
    return;
  }
  if(strcmp(color, "LIGHT GOLDENROD 3") == 0){
    rgb[0] = 139;
    rgb[1] = 129;
    rgb[2] = 76;
    return;
  }
  if(strcmp(color, "LIGHT GOLDENROD YELLOW") == 0){
    rgb[0] = 250;
    rgb[1] = 250;
    rgb[2] = 210;
    return;
  }
  if(strcmp(color, "LIGHT GREY") == 0){
    rgb[0] = 211;
    rgb[1] = 211;
    rgb[2] = 211;
    return;
  }
  if(strcmp(color, "LIGHT PINK") == 0){
    rgb[0] = 255;
    rgb[1] = 182;
    rgb[2] = 193;
    return;
  }
  if(strcmp(color, "LIGHT PINK 1") == 0){
    rgb[0] = 255;
    rgb[1] = 174;
    rgb[2] = 185;
    return;
  }
  if(strcmp(color, "LIGHT PINK 2") == 0){
    rgb[0] = 238;
    rgb[1] = 162;
    rgb[2] = 173;
    return;
  }
  if(strcmp(color, "LIGHT PINK 3") == 0){
    rgb[0] = 205;
    rgb[1] = 140;
    rgb[2] = 149;
    return;
  }
  if(strcmp(color, "LIGHT PINK 4") == 0){
    rgb[0] = 139;
    rgb[1] = 95;
    rgb[2] = 101;
    return;
  }
  if(strcmp(color, "LIGHT SALMON") == 0){
    rgb[0] = 255;
    rgb[1] = 160;
    rgb[2] = 122;
    return;
  }
  if(strcmp(color, "LIGHT SALMON 1") == 0){
    rgb[0] = 238;
    rgb[1] = 149;
    rgb[2] = 114;
    return;
  }
  if(strcmp(color, "LIGHT SALMON 2") == 0){
    rgb[0] = 205;
    rgb[1] = 129;
    rgb[2] = 98;
    return;
  }
  if(strcmp(color, "LIGHT SALMON 3") == 0){
    rgb[0] = 139;
    rgb[1] = 87;
    rgb[2] = 66;
    return;
  }
  if(strcmp(color, "LIGHT SEA GREEN") == 0){
    rgb[0] = 32;
    rgb[1] = 178;
    rgb[2] = 170;
    return;
  }
  if(strcmp(color, "LIGHT SKY BLUE") == 0){
    rgb[0] = 135;
    rgb[1] = 206;
    rgb[2] = 250;
    return;
  }
  if(strcmp(color, "LIGHT SKY BLUE 1") == 0){
    rgb[0] = 176;
    rgb[1] = 226;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "LIGHT SKY BLUE 2") == 0){
    rgb[0] = 164;
    rgb[1] = 211;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "LIGHT SKY BLUE 3") == 0){
    rgb[0] = 141;
    rgb[1] = 182;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "LIGHT SKY BLUE 4") == 0){
    rgb[0] = 96;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "LIGHT SLATE BLUE") == 0){
    rgb[0] = 132;
    rgb[1] = 112;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "LIGHT SLATE GRAY") == 0){
    rgb[0] = 119;
    rgb[1] = 136;
    rgb[2] = 153;
    return;
  }
  if(strcmp(color, "LIGHT STEEL BLUE") == 0){
    rgb[0] = 176;
    rgb[1] = 196;
    rgb[2] = 222;
    return;
  }
  if(strcmp(color, "LIGHT STEEL BLUE 1") == 0){
    rgb[0] = 202;
    rgb[1] = 225;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "LIGHT STEEL BLUE 2") == 0){
    rgb[0] = 188;
    rgb[1] = 210;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "LIGHT STEEL BLUE 3") == 0){
    rgb[0] = 162;
    rgb[1] = 181;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "LIGHT STEEL BLUE 4") == 0){
    rgb[0] = 110;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "LIGHT YELLOW 1") == 0){
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 224;
    return;
  }
  if(strcmp(color, "LIGHT YELLOW 2") == 0){
    rgb[0] = 238;
    rgb[1] = 238;
    rgb[2] = 209;
    return;
  }
  if(strcmp(color, "LIGHT YELLOW 3") == 0){
    rgb[0] = 205;
    rgb[1] = 205;
    rgb[2] = 180;
    return;
  }
  if(strcmp(color, "LIGHT YELLOW 4") == 0){
    rgb[0] = 139;
    rgb[1] = 139;
    rgb[2] = 122;
    return;
  }
  if(strcmp(color, "LIME GREEN") == 0){
    rgb[0] = 50;
    rgb[1] = 205;
    rgb[2] = 50;
    return;
  }
  if(strcmp(color, "LINEN") == 0){
    rgb[0] = 250;
    rgb[1] = 240;
    rgb[2] = 230;
    return;
  }
  if(strcmp(color, "MAGENTA") == 0){
    rgb[0] = 255;
    rgb[1] = 0;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "MAGENTA 2") == 0){
    rgb[0] = 238;
    rgb[1] = 0;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "MAGENTA 3") == 0){
    rgb[0] = 205;
    rgb[1] = 0;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "MAGENTA 4") == 0){
    rgb[0] = 139;
    rgb[1] = 0;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "MANGANESE BLUE") == 0){
    rgb[0] = 3;
    rgb[1] = 168;
    rgb[2] = 158;
    return;
  }
  if(strcmp(color, "MAROON") == 0){
    rgb[0] = 128;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "MAROON 1") == 0){
    rgb[0] = 255;
    rgb[1] = 52;
    rgb[2] = 179;
    return;
  }
  if(strcmp(color, "MAROON 2") == 0){
    rgb[0] = 238;
    rgb[1] = 48;
    rgb[2] = 167;
    return;
  }
  if(strcmp(color, "MAROON 3") == 0){
    rgb[0] = 205;
    rgb[1] = 41;
    rgb[2] = 144;
    return;
  }
  if(strcmp(color, "MAROON 4") == 0){
    rgb[0] = 139;
    rgb[1] = 28;
    rgb[2] = 98;
    return;
  }
  if(strcmp(color, "MEDIUM ORCHID") == 0){
    rgb[0] = 186;
    rgb[1] = 85;
    rgb[2] = 211;
    return;
  }
  if(strcmp(color, "MEDIUM ORCHID 1") == 0){
    rgb[0] = 224;
    rgb[1] = 102;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "MEDIUM ORCHID 2") == 0){
    rgb[0] = 209;
    rgb[1] = 95;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "MEDIUM ORCHID 3") == 0){
    rgb[0] = 180;
    rgb[1] = 82;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "MEDIUM ORCHID 4") == 0){
    rgb[0] = 122;
    rgb[1] = 55;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "MEDIUM PURPLE") == 0){
    rgb[0] = 147;
    rgb[1] = 112;
    rgb[2] = 219;
    return;
  }
  if(strcmp(color, "MEDIUM PURPLE 1") == 0){
    rgb[0] = 171;
    rgb[1] = 130;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "MEDIUM PURPLE 2") == 0){
    rgb[0] = 159;
    rgb[1] = 121;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "MEDIUM PURPLE 3") == 0){
    rgb[0] = 137;
    rgb[1] = 104;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "MEDIUM PURPLE 4") == 0){
    rgb[0] = 93;
    rgb[1] = 71;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "MEDIUM SEA GREEN") == 0){
    rgb[0] = 60;
    rgb[1] = 179;
    rgb[2] = 113;
    return;
  }
  if(strcmp(color, "MEDIUM SLATE BLUE") == 0){
    rgb[0] = 123;
    rgb[1] = 104;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "MEDIUM SPRING GREEN") == 0){
    rgb[0] = 0;
    rgb[1] = 250;
    rgb[2] = 154;
    return;
  }
  if(strcmp(color, "MEDIUM TURQUOISE") == 0){
    rgb[0] = 72;
    rgb[1] = 209;
    rgb[2] = 204;
    return;
  }
  if(strcmp(color, "MEDIUM VIOLET RED") == 0){
    rgb[0] = 199;
    rgb[1] = 21;
    rgb[2] = 133;
    return;
  }
  if(strcmp(color, "MELON") == 0){
    rgb[0] = 227;
    rgb[1] = 168;
    rgb[2] = 105;
    return;
  }
  if(strcmp(color, "MIDNIGHT BLUE") == 0){
    rgb[0] = 25;
    rgb[1] = 25;
    rgb[2] = 112;
    return;
  }
  if(strcmp(color, "MINT") == 0){
    rgb[0] = 189;
    rgb[1] = 252;
    rgb[2] = 201;
    return;
  }
  if(strcmp(color, "MINT CREAM") == 0){
    rgb[0] = 245;
    rgb[1] = 255;
    rgb[2] = 250;
    return;
  }
  if(strcmp(color, "MISTY ROSE") == 0){
    rgb[0] = 255;
    rgb[1] = 228;
    rgb[2] = 225;
    return;
  }
  if(strcmp(color, "MISTY ROSE 1") == 0){
    rgb[0] = 238;
    rgb[1] = 213;
    rgb[2] = 210;
    return;
  }
  if(strcmp(color, "MISTY ROSE 2") == 0){
    rgb[0] = 205;
    rgb[1] = 183;
    rgb[2] = 181;
    return;
  }
  if(strcmp(color, "MISTY ROSE 3") == 0){
    rgb[0] = 139;
    rgb[1] = 125;
    rgb[2] = 123;
    return;
  }
  if(strcmp(color, "MOCCASIN") == 0){
    rgb[0] = 255;
    rgb[1] = 228;
    rgb[2] = 181;
    return;
  }
  if(strcmp(color, "NAVAJO WHITE") == 0){
    rgb[0] = 255;
    rgb[1] = 222;
    rgb[2] = 173;
    return;
  }
  if(strcmp(color, "NAVAJO WHITE 1") == 0){
    rgb[0] = 238;
    rgb[1] = 207;
    rgb[2] = 161;
    return;
  }
  if(strcmp(color, "NAVAJO WHITE 2") == 0){
    rgb[0] = 205;
    rgb[1] = 179;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "NAVAJO WHITE 3") == 0){
    rgb[0] = 139;
    rgb[1] = 121;
    rgb[2] = 94;
    return;
  }
  if(strcmp(color, "NAVY") == 0){
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 128;
    return;
  }
  if(strcmp(color, "OLD LACE") == 0){
    rgb[0] = 253;
    rgb[1] = 245;
    rgb[2] = 230;
    return;
  }
  if(strcmp(color, "OLIVE") == 0){
    rgb[0] = 128;
    rgb[1] = 128;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "OLIVE DRAB") == 0){
    rgb[0] = 192;
    rgb[1] = 255;
    rgb[2] = 62;
    return;
  }
  if(strcmp(color, "OLIVE DRAB 1") == 0){
    rgb[0] = 179;
    rgb[1] = 238;
    rgb[2] = 58;
    return;
  }
  if(strcmp(color, "OLIVE DRAB 2") == 0){
    rgb[0] = 154;
    rgb[1] = 205;
    rgb[2] = 50;
    return;
  }
  if(strcmp(color, "OLIVE DRAB 3") == 0){
    rgb[0] = 105;
    rgb[1] = 139;
    rgb[2] = 34;
    return;
  }
  if(strcmp(color, "ORANGE") == 0){
    rgb[0] = 255;
    rgb[1] = 128;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE 1") == 0){
    rgb[0] = 255;
    rgb[1] = 165;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE 2") == 0){
    rgb[0] = 238;
    rgb[1] = 154;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE 3") == 0){
    rgb[0] = 205;
    rgb[1] = 133;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE 4") == 0){
    rgb[0] = 139;
    rgb[1] = 90;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE RED") == 0){
    rgb[0] = 255;
    rgb[1] = 69;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE RED 1") == 0){
    rgb[0] = 238;
    rgb[1] = 64;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE RED 2") == 0){
    rgb[0] = 205;
    rgb[1] = 55;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORANGE RED 3") == 0){
    rgb[0] = 139;
    rgb[1] = 37;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ORCHID") == 0){
    rgb[0] = 218;
    rgb[1] = 112;
    rgb[2] = 214;
    return;
  }
  if(strcmp(color, "ORCHID 1") == 0){
    rgb[0] = 255;
    rgb[1] = 131;
    rgb[2] = 250;
    return;
  }
  if(strcmp(color, "ORCHID 2") == 0){
    rgb[0] = 238;
    rgb[1] = 122;
    rgb[2] = 233;
    return;
  }
  if(strcmp(color, "ORCHID 3") == 0){
    rgb[0] = 205;
    rgb[1] = 105;
    rgb[2] = 201;
    return;
  }
  if(strcmp(color, "ORCHID 4") == 0){
    rgb[0] = 139;
    rgb[1] = 71;
    rgb[2] = 137;
    return;
  }
  if(strcmp(color, "PALE GOLDENROD") == 0){
    rgb[0] = 238;
    rgb[1] = 232;
    rgb[2] = 170;
    return;
  }
  if(strcmp(color, "PALE GREEN") == 0){
    rgb[0] = 152;
    rgb[1] = 251;
    rgb[2] = 152;
    return;
  }
  if(strcmp(color, "PALE GREEN 1") == 0){
    rgb[0] = 154;
    rgb[1] = 255;
    rgb[2] = 154;
    return;
  }
  if(strcmp(color, "PALE GREEN 2") == 0){
    rgb[0] = 144;
    rgb[1] = 238;
    rgb[2] = 144;
    return;
  }
  if(strcmp(color, "PALE GREEN 3") == 0){
    rgb[0] = 124;
    rgb[1] = 205;
    rgb[2] = 124;
    return;
  }
  if(strcmp(color, "PALE GREEN 4") == 0){
    rgb[0] = 84;
    rgb[1] = 139;
    rgb[2] = 84;
    return;
  }
  if(strcmp(color, "PALE TURQUOISE") == 0){
    rgb[0] = 187;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "PALE TURQUOISE 1") == 0){
    rgb[0] = 174;
    rgb[1] = 238;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "PALE TURQUOISE 2") == 0){
    rgb[0] = 150;
    rgb[1] = 205;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "PALE TURQUOISE 3") == 0){
    rgb[0] = 102;
    rgb[1] = 139;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "PALE VIOLET RED") == 0){
    rgb[0] = 219;
    rgb[1] = 112;
    rgb[2] = 147;
    return;
  }
  if(strcmp(color, "PALE VIOLET RED 1") == 0){
    rgb[0] = 255;
    rgb[1] = 130;
    rgb[2] = 171;
    return;
  }
  if(strcmp(color, "PALE VIOLET RED 2") == 0){
    rgb[0] = 238;
    rgb[1] = 121;
    rgb[2] = 159;
    return;
  }
  if(strcmp(color, "PALE VIOLET RED 3") == 0){
    rgb[0] = 205;
    rgb[1] = 104;
    rgb[2] = 137;
    return;
  }
  if(strcmp(color, "PALE VIOLET RED 4") == 0){
    rgb[0] = 139;
    rgb[1] = 71;
    rgb[2] = 93;
    return;
  }
  if(strcmp(color, "PAPAYA WHIP") == 0){
    rgb[0] = 255;
    rgb[1] = 239;
    rgb[2] = 213;
    return;
  }
  if(strcmp(color, "PEACH PUFF") == 0){
    rgb[0] = 255;
    rgb[1] = 218;
    rgb[2] = 185;
    return;
  }
  if(strcmp(color, "PEACH PUFF 1") == 0){
    rgb[0] = 238;
    rgb[1] = 203;
    rgb[2] = 173;
    return;
  }
  if(strcmp(color, "PEACH PUFF 2") == 0){
    rgb[0] = 205;
    rgb[1] = 175;
    rgb[2] = 149;
    return;
  }
  if(strcmp(color, "PEACH PUFF 3") == 0){
    rgb[0] = 139;
    rgb[1] = 119;
    rgb[2] = 101;
    return;
  }
  if(strcmp(color, "PEACOCK") == 0){
    rgb[0] = 51;
    rgb[1] = 161;
    rgb[2] = 201;
    return;
  }
  if(strcmp(color, "PINK") == 0){
    rgb[0] = 255;
    rgb[1] = 192;
    rgb[2] = 203;
    return;
  }
  if(strcmp(color, "PINK 1") == 0){
    rgb[0] = 255;
    rgb[1] = 181;
    rgb[2] = 197;
    return;
  }
  if(strcmp(color, "PINK 2") == 0){
    rgb[0] = 238;
    rgb[1] = 169;
    rgb[2] = 184;
    return;
  }
  if(strcmp(color, "PINK 3") == 0){
    rgb[0] = 205;
    rgb[1] = 145;
    rgb[2] = 158;
    return;
  }
  if(strcmp(color, "PINK 4") == 0){
    rgb[0] = 139;
    rgb[1] = 99;
    rgb[2] = 108;
    return;
  }
  if(strcmp(color, "PLUM") == 0){
    rgb[0] = 221;
    rgb[1] = 160;
    rgb[2] = 221;
    return;
  }
  if(strcmp(color, "PLUM 1") == 0){
    rgb[0] = 255;
    rgb[1] = 187;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "PLUM 2") == 0){
    rgb[0] = 238;
    rgb[1] = 174;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "PLUM 3") == 0){
    rgb[0] = 205;
    rgb[1] = 150;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "PLUM 4") == 0){
    rgb[0] = 139;
    rgb[1] = 102;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "POWDER BLUE") == 0){
    rgb[0] = 176;
    rgb[1] = 224;
    rgb[2] = 230;
    return;
  }
  if(strcmp(color, "PURPLE") == 0){
    rgb[0] = 128;
    rgb[1] = 0;
    rgb[2] = 128;
    return;
  }
  if(strcmp(color, "PURPLE 1") == 0){
    rgb[0] = 155;
    rgb[1] = 48;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "PURPLE 2") == 0){
    rgb[0] = 145;
    rgb[1] = 44;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "PURPLE 3") == 0){
    rgb[0] = 125;
    rgb[1] = 38;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "PURPLE 4") == 0){
    rgb[0] = 85;
    rgb[1] = 26;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "RASPBERRY") == 0){
    rgb[0] = 135;
    rgb[1] = 38;
    rgb[2] = 87;
    return;
  }
  if(strcmp(color, "RAW SIENNA") == 0){
    rgb[0] = 199;
    rgb[1] = 97;
    rgb[2] = 20;
    return;
  }
  if(strcmp(color, "RED") == 0){
    rgb[0] = 255;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "RED 1") == 0){
    rgb[0] = 238;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "RED 2") == 0){
    rgb[0] = 205;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "RED 3") == 0){
    rgb[0] = 139;
    rgb[1] = 0;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "ROSY BROWN") == 0){
    rgb[0] = 188;
    rgb[1] = 143;
    rgb[2] = 143;
    return;
  }
  if(strcmp(color, "ROSY BROWN 1") == 0){
    rgb[0] = 255;
    rgb[1] = 193;
    rgb[2] = 193;
    return;
  }
  if(strcmp(color, "ROSY BROWN 2") == 0){
    rgb[0] = 238;
    rgb[1] = 180;
    rgb[2] = 180;
    return;
  }
  if(strcmp(color, "ROSY BROWN 3") == 0){
    rgb[0] = 205;
    rgb[1] = 155;
    rgb[2] = 155;
    return;
  }
  if(strcmp(color, "ROSY BROWN 4") == 0){
    rgb[0] = 139;
    rgb[1] = 105;
    rgb[2] = 105;
    return;
  }
  if(strcmp(color, "ROYAL BLUE") == 0){
    rgb[0] = 65;
    rgb[1] = 105;
    rgb[2] = 225;
    return;
  }
  if(strcmp(color, "ROYAL BLUE 1") == 0){
    rgb[0] = 72;
    rgb[1] = 118;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "ROYAL BLUE 2") == 0){
    rgb[0] = 67;
    rgb[1] = 110;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "ROYAL BLUE 3") == 0){
    rgb[0] = 58;
    rgb[1] = 95;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "ROYAL BLUE 4") == 0){
    rgb[0] = 39;
    rgb[1] = 64;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "SALMON") == 0){
    rgb[0] = 250;
    rgb[1] = 128;
    rgb[2] = 114;
    return;
  }
  if(strcmp(color, "SALMON 1") == 0){
    rgb[0] = 255;
    rgb[1] = 140;
    rgb[2] = 105;
    return;
  }
  if(strcmp(color, "SALMON 2") == 0){
    rgb[0] = 238;
    rgb[1] = 130;
    rgb[2] = 98;
    return;
  }
  if(strcmp(color, "SALMON 3") == 0){
    rgb[0] = 205;
    rgb[1] = 112;
    rgb[2] = 84;
    return;
  }
  if(strcmp(color, "SALMON 4") == 0){
    rgb[0] = 139;
    rgb[1] = 76;
    rgb[2] = 57;
    return;
  }
  if(strcmp(color, "SANDY BROWN") == 0){
    rgb[0] = 244;
    rgb[1] = 164;
    rgb[2] = 96;
    return;
  }
  if(strcmp(color, "SAP GREEN") == 0){
    rgb[0] = 48;
    rgb[1] = 128;
    rgb[2] = 20;
    return;
  }
  if(strcmp(color, "SEA GREEN") == 0){
    rgb[0] = 84;
    rgb[1] = 255;
    rgb[2] = 159;
    return;
  }
  if(strcmp(color, "SEA GREEN 1") == 0){
    rgb[0] = 78;
    rgb[1] = 238;
    rgb[2] = 148;
    return;
  }
  if(strcmp(color, "SEA GREEN 2") == 0){
    rgb[0] = 67;
    rgb[1] = 205;
    rgb[2] = 128;
    return;
  }
  if(strcmp(color, "SEA GREEN 3") == 0){
    rgb[0] = 46;
    rgb[1] = 139;
    rgb[2] = 87;
    return;
  }
  if(strcmp(color, "SEASHELL") == 0){
    rgb[0] = 255;
    rgb[1] = 245;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "SEASHELL 1") == 0){
    rgb[0] = 238;
    rgb[1] = 229;
    rgb[2] = 222;
    return;
  }
  if(strcmp(color, "SEASHELL 2") == 0){
    rgb[0] = 205;
    rgb[1] = 197;
    rgb[2] = 191;
    return;
  }
  if(strcmp(color, "SEASHELL 3") == 0){
    rgb[0] = 139;
    rgb[1] = 134;
    rgb[2] = 130;
    return;
  }
  if(strcmp(color, "SEPIA") == 0){
    rgb[0] = 94;
    rgb[1] = 38;
    rgb[2] = 18;
    return;
  }
  if(strcmp(color, "SIENNA") == 0){
    rgb[0] = 160;
    rgb[1] = 82;
    rgb[2] = 45;
    return;
  }
  if(strcmp(color, "SIENNA 1") == 0){
    rgb[0] = 255;
    rgb[1] = 130;
    rgb[2] = 71;
    return;
  }
  if(strcmp(color, "SIENNA 2") == 0){
    rgb[0] = 238;
    rgb[1] = 121;
    rgb[2] = 66;
    return;
  }
  if(strcmp(color, "SIENNA 3") == 0){
    rgb[0] = 205;
    rgb[1] = 104;
    rgb[2] = 57;
    return;
  }
  if(strcmp(color, "SIENNA 4") == 0){
    rgb[0] = 139;
    rgb[1] = 71;
    rgb[2] = 38;
    return;
  }
  if(strcmp(color, "SILVER") == 0){
    rgb[0] = 192;
    rgb[1] = 192;
    rgb[2] = 192;
    return;
  }
  if(strcmp(color, "SKY BLUE") == 0){
    rgb[0] = 135;
    rgb[1] = 206;
    rgb[2] = 235;
    return;
  }
  if(strcmp(color, "SKY BLUE 1") == 0){
    rgb[0] = 135;
    rgb[1] = 206;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "SKY BLUE 2") == 0){
    rgb[0] = 126;
    rgb[1] = 192;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "SKY BLUE 3") == 0){
    rgb[0] = 108;
    rgb[1] = 166;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "SKY BLUE 4") == 0){
    rgb[0] = 74;
    rgb[1] = 112;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "SLATE BLUE") == 0){
    rgb[0] = 106;
    rgb[1] = 90;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "SLATE BLUE 1") == 0){
    rgb[0] = 131;
    rgb[1] = 111;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "SLATE BLUE 2") == 0){
    rgb[0] = 122;
    rgb[1] = 103;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "SLATE BLUE 3") == 0){
    rgb[0] = 105;
    rgb[1] = 89;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "SLATE BLUE 4") == 0){
    rgb[0] = 71;
    rgb[1] = 60;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "SLATE GRAY") == 0){
    rgb[0] = 112;
    rgb[1] = 128;
    rgb[2] = 144;
    return;
  }
  if(strcmp(color, "SLATE GRAY 1") == 0){
    rgb[0] = 198;
    rgb[1] = 226;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "SLATE GRAY 2") == 0){
    rgb[0] = 185;
    rgb[1] = 211;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "SLATE GRAY 3") == 0){
    rgb[0] = 159;
    rgb[1] = 182;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "SLATE GRAY 4") == 0){
    rgb[0] = 108;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "SNOW") == 0){
    rgb[0] = 255;
    rgb[1] = 250;
    rgb[2] = 250;
    return;
  }
  if(strcmp(color, "SNOW 1") == 0){
    rgb[0] = 238;
    rgb[1] = 233;
    rgb[2] = 233;
    return;
  }
  if(strcmp(color, "SNOW 2") == 0){
    rgb[0] = 205;
    rgb[1] = 201;
    rgb[2] = 201;
    return;
  }
  if(strcmp(color, "SNOW 3") == 0){
    rgb[0] = 139;
    rgb[1] = 137;
    rgb[2] = 137;
    return;
  }
  if(strcmp(color, "SPRING GREEN") == 0){
    rgb[0] = 0;
    rgb[1] = 255;
    rgb[2] = 127;
    return;
  }
  if(strcmp(color, "SPRING GREEN 1") == 0){
    rgb[0] = 0;
    rgb[1] = 238;
    rgb[2] = 118;
    return;
  }
  if(strcmp(color, "SPRING GREEN 2") == 0){
    rgb[0] = 0;
    rgb[1] = 205;
    rgb[2] = 102;
    return;
  }
  if(strcmp(color, "SPRING GREEN 3") == 0){
    rgb[0] = 0;
    rgb[1] = 139;
    rgb[2] = 69;
    return;
  }
  if(strcmp(color, "STEEL BLUE") == 0){
    rgb[0] = 70;
    rgb[1] = 130;
    rgb[2] = 180;
    return;
  }
  if(strcmp(color, "STEEL BLUE 1") == 0){
    rgb[0] = 99;
    rgb[1] = 184;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "STEEL BLUE 2") == 0){
    rgb[0] = 92;
    rgb[1] = 172;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "STEEL BLUE 3") == 0){
    rgb[0] = 79;
    rgb[1] = 148;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "STEEL BLUE 4") == 0){
    rgb[0] = 54;
    rgb[1] = 100;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "TAN") == 0){
    rgb[0] = 210;
    rgb[1] = 180;
    rgb[2] = 140;
    return;
  }
  if(strcmp(color, "TAN 1") == 0){
    rgb[0] = 255;
    rgb[1] = 165;
    rgb[2] = 79;
    return;
  }
  if(strcmp(color, "TAN 2") == 0){
    rgb[0] = 238;
    rgb[1] = 154;
    rgb[2] = 73;
    return;
  }
  if(strcmp(color, "TAN 3") == 0){
    rgb[0] = 205;
    rgb[1] = 133;
    rgb[2] = 63;
    return;
  }
  if(strcmp(color, "TAN 4") == 0){
    rgb[0] = 139;
    rgb[1] = 90;
    rgb[2] = 43;
    return;
  }
  if(strcmp(color, "TEAL") == 0){
    rgb[0] = 0;
    rgb[1] = 128;
    rgb[2] = 128;
    return;
  }
  if(strcmp(color, "THISTLE") == 0){
    rgb[0] = 216;
    rgb[1] = 191;
    rgb[2] = 216;
    return;
  }
  if(strcmp(color, "THISTLE 1") == 0){
    rgb[0] = 255;
    rgb[1] = 225;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "THISTLE 2") == 0){
    rgb[0] = 238;
    rgb[1] = 210;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "THISTLE 3") == 0){
    rgb[0] = 205;
    rgb[1] = 181;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "THISTLE 4") == 0){
    rgb[0] = 139;
    rgb[1] = 123;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "TOMATO") == 0){
    rgb[0] = 255;
    rgb[1] = 99;
    rgb[2] = 71;
    return;
  }
  if(strcmp(color, "TOMATO 1") == 0){
    rgb[0] = 238;
    rgb[1] = 92;
    rgb[2] = 66;
    return;
  }
  if(strcmp(color, "TOMATO 2") == 0){
    rgb[0] = 205;
    rgb[1] = 79;
    rgb[2] = 57;
    return;
  }
  if(strcmp(color, "TOMATO 3") == 0){
    rgb[0] = 139;
    rgb[1] = 54;
    rgb[2] = 38;
    return;
  }
  if(strcmp(color, "TURQUOISE") == 0){
    rgb[0] = 64;
    rgb[1] = 224;
    rgb[2] = 208;
    return;
  }
  if(strcmp(color, "TURQUOISE 1") == 0){
    rgb[0] = 0;
    rgb[1] = 245;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "TURQUOISE 2") == 0){
    rgb[0] = 0;
    rgb[1] = 229;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "TURQUOISE 3") == 0){
    rgb[0] = 0;
    rgb[1] = 197;
    rgb[2] = 205;
    return;
  }
  if(strcmp(color, "TURQUOISE 4") == 0){
    rgb[0] = 0;
    rgb[1] = 134;
    rgb[2] = 139;
    return;
  }
  if(strcmp(color, "TURQUOISE BLUE") == 0){
    rgb[0] = 0;
    rgb[1] = 199;
    rgb[2] = 140;
    return;
  }
  if(strcmp(color, "VIOLET") == 0){
    rgb[0] = 238;
    rgb[1] = 130;
    rgb[2] = 238;
    return;
  }
  if(strcmp(color, "VIOLET RED") == 0){
    rgb[0] = 208;
    rgb[1] = 32;
    rgb[2] = 144;
    return;
  }
  if(strcmp(color, "VIOLET RED 1") == 0){
    rgb[0] = 255;
    rgb[1] = 62;
    rgb[2] = 150;
    return;
  }
  if(strcmp(color, "VIOLET RED 2") == 0){
    rgb[0] = 238;
    rgb[1] = 58;
    rgb[2] = 140;
    return;
  }
  if(strcmp(color, "VIOLET RED 3") == 0){
    rgb[0] = 205;
    rgb[1] = 50;
    rgb[2] = 120;
    return;
  }
  if(strcmp(color, "VIOLET RED 4") == 0){
    rgb[0] = 139;
    rgb[1] = 34;
    rgb[2] = 82;
    return;
  }
  if(strcmp(color, "WARM GREY") == 0){
    rgb[0] = 128;
    rgb[1] = 128;
    rgb[2] = 105;
    return;
  }
  if(strcmp(color, "WHEAT") == 0){
    rgb[0] = 245;
    rgb[1] = 222;
    rgb[2] = 179;
    return;
  }
  if(strcmp(color, "WHEAT 1") == 0){
    rgb[0] = 255;
    rgb[1] = 231;
    rgb[2] = 186;
    return;
  }
  if(strcmp(color, "WHEAT 2") == 0){
    rgb[0] = 238;
    rgb[1] = 216;
    rgb[2] = 174;
    return;
  }
  if(strcmp(color, "WHEAT 3") == 0){
    rgb[0] = 205;
    rgb[1] = 186;
    rgb[2] = 150;
    return;
  }
  if(strcmp(color, "WHEAT 4") == 0){
    rgb[0] = 139;
    rgb[1] = 126;
    rgb[2] = 102;
    return;
  }
  if(strcmp(color, "WHITE") == 0){
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 255;
    return;
  }
  if(strcmp(color, "WHITE SMOKE") == 0){
    rgb[0] = 245;
    rgb[1] = 245;
    rgb[2] = 245;
    return;
  }
  if(strcmp(color, "YELLOW") == 0){
    rgb[0] = 255;
    rgb[1] = 255;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "YELLOW 1") == 0){
    rgb[0] = 238;
    rgb[1] = 238;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "YELLOW 2") == 0){
    rgb[0] = 205;
    rgb[1] = 205;
    rgb[2] = 0;
    return;
  }
  if(strcmp(color, "YELLOW 3") == 0){
    rgb[0] = 139;
    rgb[1] = 139;
    rgb[2] = 0;
  } else{
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
  }
}
