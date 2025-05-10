#include "options.h"

#ifdef pp_GPU
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "glew.h"

#include "smokeviewvars.h"

GLuint p_smoke, p_3dslice, p_zonesmoke, p_volsmoke;

#define LINK_BAD 0
#define LINK_GOOD 1

/* ------------------ ShaderLinkStatus ------------------------ */

int ShaderLinkStatus(GLuint program){
  GLint isLinked;

  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
  if(isLinked == GL_FALSE){
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
    if(maxLength > 0){
#ifdef _DEBUG
      char *infoLog;

      NewMemory((void **)&infoLog, maxLength+1);
      glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);
      PRINTF("%s\n", infoLog);
      FREEMEMORY(infoLog);
#endif
      glDeleteProgram(program);
    }
    return GL_FALSE;
  }
  return GL_TRUE;
}

/* ------------------ ShaderCompileStatus ------------------------ */

int ShaderCompileStatus(GLuint obj,char *label){
  GLint isCompiled;

  glGetShaderiv(obj, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE){
    GLint logLength = 0;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &logLength);
    if(logLength > 0){
#ifdef _DEBUG
      char *infoLog;

      NewMemory((void **)&infoLog, logLength);
      glGetShaderInfoLog(obj, logLength, &logLength, infoLog);
      PRINTF("*****************\n");
      PRINTF("%s\n", label);
      PRINTF("*****************\n");
      PRINTF("%s\n", infoLog);
      FREEMEMORY(infoLog);
#endif
      glDeleteShader(obj);
    }
    return GL_FALSE;
  }
  return GL_TRUE;
}


/* ------------------ SetZoneSmokeShaders ------------------------ */

int SetZoneSmokeShaders(){
  GLuint vert_shader, frag_shader;

  const GLchar *FragmentShaderSource[]={
    "#version 120\n"
    "uniform int zonedir,zoneinside;"
    "uniform float xyzmaxdiff,zlay,odl,odu;"
    "uniform vec3 eyepos,boxmin, boxmax;"
    "varying vec3 fragpos;"

    "void main(){"
    "  float L,opacity,alpha,alpha_min,alpha_zlay;"
    "  float factor_U, factor_L, grey;"

    "  vec3 dalphamin,dalphamax;"
    "  L=distance(fragpos,eyepos)*xyzmaxdiff;"
    "  alpha_min=1000000.0;"
    "  dalphamin=-(boxmin-fragpos)/(eyepos-fragpos);"
    "  dalphamax=-(boxmax-fragpos)/(eyepos-fragpos);"
    "  alpha_zlay = -(zlay-fragpos.z)/(eyepos.z-fragpos.z);"
    "  factor_U=1.0;"
    "  factor_L=0.0;"
    "  if(zoneinside==0){"
    "    if(zonedir!=-1&&dalphamin.x>0.0&&dalphamin.x<alpha_min)alpha_min=dalphamin.x;"
    "    if(zonedir!=1 &&dalphamax.x>0.0&&dalphamax.x<alpha_min)alpha_min=dalphamax.x;"
    "    if(zonedir!=-2&&dalphamin.y>0.0&&dalphamin.y<alpha_min)alpha_min=dalphamin.y;"
    "    if(zonedir!=2 &&dalphamax.y>0.0&&dalphamax.y<alpha_min)alpha_min=dalphamax.y;"
    "    if(zonedir!=-3&&dalphamin.z>0.0&&dalphamin.z<alpha_min)alpha_min=dalphamin.z;"
    "    if(zonedir!=3 &&dalphamax.z>0.0&&dalphamax.z<alpha_min)alpha_min=dalphamax.z;"
    "    if(eyepos.z>zlay&&fragpos.z>zlay){"
    "      if(alpha_zlay>0.0&&alpha_zlay<alpha_min){"
    "        factor_U=alpha_zlay/odu;"
    "        factor_L=(alpha_min-alpha_zlay)/odl;"
    "      }"
    "      else{"
    "        factor_U=alpha_min/odu;"
    "        factor_L=0.0;"
    "      }"
    "    }"
    "    else if(eyepos.z>zlay&&fragpos.z<=zlay){"
    "      factor_U=0.0;"
    "      factor_L=alpha_min/odl;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z>zlay){"
    "      factor_U=alpha_min/odu;"
    "      factor_L=0.0;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z<=zlay){"
    "      if(alpha_zlay>0.0&&alpha_zlay<alpha_min){"
    "        factor_U=(alpha_min-alpha_zlay)/odu;"
    "        factor_L=alpha_zlay/odl;"
    "      }"
    "      else{"
    "        factor_U=0.0;"
    "        factor_L=alpha_min/odl;"
    "      }"
    "    }"
    "  }" // end inside=0
    "  if(zoneinside==1){"
    "    if(eyepos.z>zlay&&fragpos.z>zlay){"
    "      factor_U=1.0/odu;"
    "      factor_L=0.0;"
    "    }"
    "    else if(eyepos.z>zlay&&fragpos.z<=zlay){"
    "      factor_U=(1.0+alpha_zlay)/odu;"
    "      factor_L=-alpha_zlay/odl;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z>zlay){"
    "      factor_U=-alpha_zlay/odu;"
    "      factor_L=(1.0+alpha_zlay)/odl;"
    "    }"
    "    else if(eyepos.z<=zlay&&fragpos.z<=zlay){"
    "      factor_U=0.0;"
    "      factor_L=1.0/odl;"
    "    }"
    "  }" // end inside=1
    "  grey=0.0;"
    "  opacity = 1.0-exp(-(factor_L+factor_U)*L);"
    "  gl_FragColor = vec4(grey,grey,grey,opacity);"
    "}" // end of main
  };

  const GLchar *VertexShaderSource[]={
    "#version 120\n"
    "varying vec3 fragpos;"
    "void main(){"
    "  fragpos=vec3(gl_Vertex);"
    "  gl_Position = ftransform();"
    "}"
  };

  vert_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShader(vert_shader);
  if(ShaderCompileStatus(vert_shader,"zone vertex shader")==GL_FALSE)return 0;

  frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShader(frag_shader);
  if(ShaderCompileStatus(frag_shader,"zone fragment shader")==GL_FALSE)return 0;

  p_zonesmoke = glCreateProgram();
  glAttachShader(p_zonesmoke,vert_shader);
  glAttachShader(p_zonesmoke,frag_shader);

  glLinkProgram(p_zonesmoke);
  if(ShaderLinkStatus(p_zonesmoke)==GL_FALSE)return 0;

  GPUzone_zoneinside = glGetUniformLocation(p_zonesmoke,"zoneinside");
  GPUzone_zonedir = glGetUniformLocation(p_zonesmoke,"zonedir");
  GPUzone_eyepos = glGetUniformLocation(p_zonesmoke,"eyepos");
  GPUzone_xyzmaxdiff = glGetUniformLocation(p_zonesmoke,"xyzmaxdiff");
  GPUzone_boxmin = glGetUniformLocation(p_zonesmoke,"boxmin");
  GPUzone_boxmax = glGetUniformLocation(p_zonesmoke,"boxmax");
  GPUzone_zlay = glGetUniformLocation(p_zonesmoke,"zlay");
  GPUzone_odl = glGetUniformLocation(p_zonesmoke,"odl");
  GPUzone_odu = glGetUniformLocation(p_zonesmoke,"odu");

  return 1;
}

/* ------------------ Set3DSliceShaders ------------------------ */

int Set3DSliceShaders(void){
  GLuint vert_shader, frag_shader;

  const GLchar *FragmentShaderSource[]={
    "#version 120\n"
   "  uniform sampler1D colormap;"
   "  uniform sampler3D val_texture;"
   "  uniform float val_min,val_max;"
   "  uniform float transparent_level;"
   "  varying vec3 fragpos;"
   "  uniform vec3 boxmin,boxmax;"
   "void main(){"
   "  vec3 color_val,position;"
   "  float val,colorindex;"

   "  position = (fragpos-boxmin)/(boxmax-boxmin);"
   "  val = texture3D(val_texture,position).x;"
   "  colorindex = (val-val_min)/(val_max-val_min);"
   "  colorindex = clamp(colorindex,0.0,1.0);"
   "  color_val = texture1D(colormap,colorindex).rgb;"
   "  gl_FragColor = vec4(color_val.rgb,transparent_level);"
   "}"
  };

  const GLchar *VertexShaderSource[]={
    "#version 120\n"
    "varying vec3 fragpos;"
    "void main(){"
    "  fragpos=vec3(gl_Vertex);"
    "  gl_Position=ftransform();"
    "}"
  };

  vert_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShader(vert_shader);
  if(ShaderCompileStatus(vert_shader,"3D slice vertex shader")==GL_FALSE)return 0;

  frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShader(frag_shader);
  if(ShaderCompileStatus(frag_shader,"3D slice fragment shader")==GL_FALSE)return 0;

  p_3dslice = glCreateProgram();
  glAttachShader(p_3dslice,vert_shader);
  glAttachShader(p_3dslice,frag_shader);

  glLinkProgram(p_3dslice);
  if(ShaderLinkStatus(p_3dslice) == GL_FALSE)return 0;

  GPU3dslice_valtexture = glGetUniformLocation(p_3dslice,"valtexture");
  GPU3dslice_colormap = glGetUniformLocation(p_3dslice,"colormap");
  GPU3dslice_val_min = glGetUniformLocation(p_3dslice,"val_min");
  GPU3dslice_val_max = glGetUniformLocation(p_3dslice,"val_max");
  GPU3dslice_boxmin = glGetUniformLocation(p_3dslice,"boxmin");
  GPU3dslice_boxmax = glGetUniformLocation(p_3dslice,"boxmax");
  GPU3dslice_transparent_level = glGetUniformLocation(p_3dslice,"transparent_level");

  return 1;
}

/* ------------------ SetVolSmokeShaders ------------------------ */

int SetVolSmokeShaders(){
  GLuint vert_shader, frag_shader;

  const GLchar *FragmentShaderSource[] = {
    "#version 120\n"
    "uniform sampler1D smokecolormap;"
    "uniform sampler3D soot_density_texture,fire_texture,blockage_texture;"
    "uniform vec3 eyepos,boxmin,boxmax,dcell3;"
    "varying vec3 fragpos;"
    "uniform float xyzmaxdiff,dcell,fire_opacity_factor,gpu_vol_factor;"
    "uniform float temperature_min,temperature_cutoff,temperature_max;"
    "uniform float mass_extinct;"
    "uniform int inside,havefire,volbw,slicetype,block_volsmoke;"
    "uniform int drawsides[7];"

    "float color2bw(vec3 color){"
    " return 0.299*color.r+0.587*color.g+0.114*color.b;"
    "}"
    "void main(){"
    "  vec3 dalphamin,dalphamax,fragmaxpos,position,position2,color_val,color_total,block_pos,block_pos2;"
    "  vec3 uvec, vvec;"
    "  float d;"
    "  float soot_val,block_val,block_val2;"
    "  float alpha_min,factor,factor0,dfactor,dfactor0,pathdist;"
    "  float colorindex,last_tempval,tempval,gray;"
    "  float taui, alphai;"
    "  float taun, alphan;"
    "  float dstep;"
    "  int i,n_iter;"
    "  int side,in_fire;"
    "  int in_block;"
    "  float maxcolor;"

    "  alpha_min=1000000.0;"
    "  dalphamin=-(boxmin-fragpos)/(eyepos-fragpos);"
    "  dalphamax=-(boxmax-fragpos)/(eyepos-fragpos);"
    "  side=0;"
    "  if(inside==0){"
    "    if(drawsides[-1+3]==0&&dalphamin.x>0.0&&dalphamin.x<alpha_min){"
    "      alpha_min=dalphamin.x;"
    "      side=-1;"
    "    }"
    "    if(drawsides[ 1+3]==0&&dalphamax.x>0.0&&dalphamax.x<alpha_min){"
    "      alpha_min=dalphamax.x;"
    "      side=1;"
    "    }"
    "    if(drawsides[-2+3]==0&&dalphamin.y>0.0&&dalphamin.y<alpha_min){"
    "      alpha_min=dalphamin.y;"
    "      side=-2;"
    "    }"
    "    if(drawsides[ 2+3]==0&&dalphamax.y>0.0&&dalphamax.y<alpha_min){"
    "      alpha_min=dalphamax.y;"
    "      side=2;"
    "    }"
    "    if(drawsides[-3+3]==0&&dalphamin.z>0.0&&dalphamin.z<alpha_min){"
    "      alpha_min=dalphamin.z;"
    "      side=-3;"
    "    }"
    "    if(drawsides[ 3+3]==0&&dalphamax.z>0.0&&dalphamax.z<alpha_min){"
    "      alpha_min=dalphamax.z;"
    "      side=3;"
    "    }"
    "  }" // end inside=0
    "  if(inside==1){"
    "  }" // end inside=1
    "  fragmaxpos = mix(fragpos,eyepos,-alpha_min);"
    "  pathdist = distance(fragpos,fragmaxpos);"
    "  n_iter = int(gpu_vol_factor*pathdist/dcell+0.5);"
    "  if(n_iter<1)n_iter=1;"
    "  taun=1.0;"
    "  alphan=0.0;"
    "  in_fire=0;"
    "  color_total=vec3(0.0,0.0,0.0);"

    "  dfactor0 = 1.0/float(n_iter);"
    "  dfactor = dfactor0;"
    "  dstep = pathdist*xyzmaxdiff/float(n_iter);"

    "  factor0=0.5*dfactor;"
    "  factor=factor0;"

    "  last_tempval=0.0;"
    "  tempval=0.0;"
    "  position=vec3(0.0,0.0,0.0);"
    "  position2=position;"
    "  while(factor<1.0){"
    "    in_block=0;"
    "    position = (mix(fragpos,fragmaxpos,factor)-boxmin)/(boxmax-boxmin);"
    "    if(slicetype!=1){"
    //            boxmin+dcell3      position2     boxmax
    // boxmin                       position       boxmax
    // (position2-boxmin-dcell3)/(boxmax-boxmin-dcell3) = (position-boxmin)/(boxmax-boxmin)
    //  solve for position2
    "      position2=position+dcell3*(boxmax-position)/(boxmax-boxmin);"
    "    }"
    "    block_val=1.0;"
    "    if(block_volsmoke==0){"
    "      block_val=1.0;"
    "      block_val2=1.0;"
    "    }"
    "    else{"
    "      block_pos = position;"
    "      block_pos2 = (mix(fragpos,fragmaxpos,factor+dfactor)-boxmin)/(boxmax-boxmin);"
    "      block_val = texture3D(blockage_texture,block_pos).x;"
    "      block_val2 = texture3D(blockage_texture,block_pos2).x;"
    "    }"
    "    if(slicetype==1){"
    "      soot_val = texture3D(soot_density_texture,position).x;"
    "    }"
    "    else{"
    "      soot_val = texture3D(soot_density_texture,position2).x;"
    "    }"
    "    if(block_val<0.5)soot_val=0.0;"
    "    in_fire=0;"
    "    if(havefire==1){"
    "      if(slicetype==1){"
    "        tempval = texture3D(fire_texture,position).x;"
    "      }"
    "      else{"
    "        tempval = texture3D(fire_texture,position2).x;"
    "      }"
    "      colorindex = clamp((tempval-temperature_min)/(temperature_max-temperature_min),0.0,1.0);"
    "      color_val = texture1D(smokecolormap,colorindex).rgb;"
    "      if(tempval>temperature_cutoff){"
    "        soot_val *= fire_opacity_factor;"
    "        in_fire=1;"
    "      }"
    "    }"
    "    if(in_fire==0){"
    "      color_val = vec3(0.0,0.0,0.0);"
    "    }"
    //  block_val  0.5  block_val2
    //  0.0        x     dstep
    //  x = dstep*(.5-block_val)/(block_val2-block_val)
    "    if(block_val2<0.5){"
    "      in_block=1;"
    "      dstep *= (0.5-block_val)/(block_val2-block_val);"
    "    }"
    "    taui = exp(-mass_extinct*soot_val*dstep);"
    "    alphai = 1.0 - taui;"
    "    taun *= taui;"
    "    alphan = 1.0-taun;"
    "    color_total += alphai*taun*color_val;"
    "    factor+=dfactor;"
    "    if(block_val2<0.5)break;"
    "    if(in_block==1){"
    "      break;"
    "    }"
    "  }"
    "  if(volbw==1){"
    "    gray=color2bw(color_total);"
    "    color_total=vec3(gray,gray,gray);"
    "  }"
    "  if(alphan>0.0){"
    "    gl_FragColor = vec4(color_total/alphan,alphan);"
    "  }"
    "  else{"
    "    gl_FragColor = vec4(0.0,0.0,0.0,0.0);"
    "  }"
    "}" // end of main
  };

  const GLchar *VertexShaderSource[]={
    "#version 120\n"
    "varying vec3 fragpos;"
    "void main(){"
    "  fragpos=vec3(gl_Vertex);"
    "  gl_Position = ftransform();"
    "}"
  };

  vert_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShader(vert_shader);
  if(ShaderCompileStatus(vert_shader,"volume vertex shader")==GL_FALSE)return 0;

  frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShader(frag_shader);
  if(ShaderCompileStatus(frag_shader,"volume fragment shader")==GL_FALSE)return 0;

  p_volsmoke = glCreateProgram();
  glAttachShader(p_volsmoke,vert_shader);
  glAttachShader(p_volsmoke,frag_shader);

  glLinkProgram(p_volsmoke);
  if(ShaderLinkStatus(p_volsmoke)==GL_FALSE)return 0;

  GPUvol_inside = glGetUniformLocation(p_volsmoke,"inside");
  GPUvol_eyepos = glGetUniformLocation(p_volsmoke,"eyepos");
  GPUvol_block_volsmoke = glGetUniformLocation(p_volsmoke,"block_volsmoke");
  GPUvol_dcell = glGetUniformLocation(p_volsmoke,"dcell");
  GPUvol_dcell3 = glGetUniformLocation(p_volsmoke,"dcell3");
  GPUvol_slicetype = glGetUniformLocation(p_volsmoke, "slicetype");
  GPUvol_xyzmaxdiff = glGetUniformLocation(p_volsmoke,"xyzmaxdiff");
  GPUvol_gpu_vol_factor = glGetUniformLocation(p_volsmoke,"gpu_vol_factor");
  GPUvol_fire_opacity_factor = glGetUniformLocation(p_volsmoke,"fire_opacity_factor");
  GPUvol_mass_extinct = glGetUniformLocation(p_volsmoke,"mass_extinct");
  GPUvol_volbw = glGetUniformLocation(p_volsmoke,"volbw");
  GPUvol_temperature_min = glGetUniformLocation(p_volsmoke,"temperature_min");
  GPUvol_temperature_cutoff = glGetUniformLocation(p_volsmoke,"temperature_cutoff");
  GPUvol_temperature_max = glGetUniformLocation(p_volsmoke,"temperature_max");
  GPUvol_boxmin = glGetUniformLocation(p_volsmoke,"boxmin");
  GPUvol_boxmax = glGetUniformLocation(p_volsmoke,"boxmax");
  GPUvol_soot_density = glGetUniformLocation(p_volsmoke,"soot_density_texture");
  GPUvol_blockage = glGetUniformLocation(p_volsmoke,"blockage_texture");
  GPUvol_fire = glGetUniformLocation(p_volsmoke,"fire_texture");

  GPUvol_havefire = glGetUniformLocation(p_volsmoke,"havefire");
  GPUvol_smokecolormap = glGetUniformLocation(p_volsmoke,"smokecolormap");
  GPUvol_drawsides = glGetUniformLocation(p_volsmoke,"drawsides");

  return 1;
}

/* ------------------ SetSmokeShaders ------------------------ */

int SetSmokeShaders(){
  GLuint vert_shader, frag_shader;

  const GLchar *FragmentShaderSource[]={
    "#version 120\n"
    "varying vec4 newcolor;"
    "void main(){"
    "  gl_FragColor = newcolor;"
    "}"
  };

  const GLchar *VertexShaderSource[]={
    "#version 120\n"
    "uniform sampler1D smokecolormap;"
    "uniform float global_hrrpuv_max, global_hrrpuv_cb_min, emission_factor;"
    "uniform float fire_alpha;"
    "uniform int have_smoke, use_fire_alpha, force_alpha_opaque;"

    "attribute float hrr, smoke_alpha;"

    "varying vec4 newcolor;"
    "void main(){"
    "  float alpha;"
    "  vec4 hrrcolor,smokecolor;"
    "  float colorindex;"
    "  float hrrlocal, fcolor, opacity_multiplier;"

    "  hrrlocal=(hrr/254.0)*global_hrrpuv_max;"
    "  colorindex=(hrrlocal-global_hrrpuv_cb_min)/(global_hrrpuv_max-global_hrrpuv_cb_min);"
    "  colorindex=clamp(colorindex,0.0,1.0);"
    "  if(hrrlocal>global_hrrpuv_cb_min){"
    "    if(use_fire_alpha==0&&have_smoke==1){"
    "      fcolor=hrrlocal/global_hrrpuv_max;"
    "      opacity_multiplier=1.0+(emission_factor-1.0)*fcolor;"
    "      alpha = (smoke_alpha/255.0)*opacity_multiplier;"
    "    }"
    "    else{"
    "      alpha=fire_alpha/255.0;"
    "    }"
    "    hrrcolor = texture1D(smokecolormap,colorindex);"
    "    if(force_alpha_opaque == 1)alpha=1.0;"
    "    newcolor=vec4(vec3(hrrcolor),alpha);"
    "  }"
    "  else{"
    "    smokecolor = texture1D(smokecolormap,colorindex);"
    "    alpha=0.0;"
    "    if(have_smoke==1){"
    "      alpha=smoke_alpha/255.0;"
    "    }"
    "    if(force_alpha_opaque == 1)alpha=1.0;"
    "    newcolor=vec4(vec3(smokecolor),alpha);"
    "  }"
    "  gl_Position = ftransform();"
    "}"
};

  vert_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shader,1, VertexShaderSource,NULL);
  glCompileShader(vert_shader);
  if(ShaderCompileStatus(vert_shader,"slice vertex shader")==GL_FALSE)return 0;

  frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader, 1, FragmentShaderSource,NULL);
  glCompileShader(frag_shader);
  if(ShaderCompileStatus(frag_shader,"slice fragment shader")==GL_FALSE)return 0;

  p_smoke = glCreateProgram();
  glAttachShader(p_smoke,vert_shader);
  glAttachShader(p_smoke,frag_shader);

  glLinkProgram(p_smoke);
  if(ShaderLinkStatus(p_smoke)==GL_FALSE)return 0;

  GPU_global_hrrpuv_max      = glGetUniformLocation(p_smoke,"global_hrrpuv_max");
  GPU_global_hrrpuv_cb_min   = glGetUniformLocation(p_smoke,"global_hrrpuv_cb_min");
  GPU_fire_alpha             = glGetUniformLocation(p_smoke,"fire_alpha");
  GPU_smokecolormap          = glGetUniformLocation(p_smoke,"smokecolormap");
  GPU_have_smoke             = glGetUniformLocation(p_smoke,"have_smoke");
  GPU_use_fire_alpha         = glGetUniformLocation(p_smoke, "use_fire_alpha");
  GPU_force_alpha_opaque     = glGetUniformLocation(p_smoke, "force_alpha_opaque");
  GPU_emission_factor        = glGetUniformLocation(p_smoke, "emission_factor");
  GPU_hrr                    = glGetAttribLocation(p_smoke,"hrr");
  GPU_smokealpha             = glGetAttribLocation(p_smoke,"smoke_alpha");
  return 1;
}

/* ------------------ Load3DSliceShaders ------------------------ */

void Load3DSliceShaders(void){
  glUseProgram(p_3dslice);
}

/* ------------------ LoadZoneSmokeShaders ------------------------ */

void LoadZoneSmokeShaders(void){
  glUseProgram(p_zonesmoke);
}

/* ------------------ LoadSmokeShaders ------------------------ */

void LoadSmokeShaders(void){
  glUseProgram(p_smoke);
}

/* ------------------ LoadVolsmokeShaders ------------------------ */

void LoadVolsmokeShaders(void){
  glUseProgram(p_volsmoke);
}

/* ------------------ UnLoadShaders ------------------------ */

void UnLoadShaders(void){
  glUseProgram(0);
}

/* ------------------ InitShaders ------------------------ */

int InitShaders(void){
  GLenum err=0;

  gpuactive=0;
  usegpu=0;
  if(opengl_version<200){
    PRINTF("   Smokeview is running on a system using OpenGL %s\n",opengl_version_label);
    PRINTF("   OpenGL 2.0 or later is required to use the GPU.\n");
    PRINTF("   GPU smoke shader not supported.\n");
    return 1;
  }

  if(GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader){
    if(verbose_output==1)PRINTF("  GPU shaders\n");
    if(SetSmokeShaders()==1){
      if(verbose_output==1)PRINTF("    3D smoke loaded\n");
    }
    else{
      PRINTF("    3D smoke failed to load\n");
      err=1;
    }
    if(SetVolSmokeShaders()==1){
      if(verbose_output==1)PRINTF("    volume smoke loaded\n");
    }
    else{
      PRINTF("    volume smoke failed to load\n");
      err=1;
    }
    if(Set3DSliceShaders()==1){
      if(verbose_output==1)PRINTF("    3D slice loaded\n");
    }
    else{
      PRINTF("    3D slice failed to load\n");
      err=1;
    }
    if(SetZoneSmokeShaders()==1){
      if(verbose_output==1)PRINTF("    zone smoke loaded\n");
    }
    else{
      PRINTF("    zone smoke failed to load\n");
      err=1;
    }
  }
  else{
    PRINTF("  *** GPU not supported.\n");
    err=1;
  }
#ifdef pp_SMOKE3D_GPU
  if(err==0)gpuactive=1;
#endif
  return err;
}

#endif
