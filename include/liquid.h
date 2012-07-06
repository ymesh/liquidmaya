/*
**
** The contents of this file are subject to the Mozilla Public License Version 1.1 (the
** "License"); you may not use this file except in compliance with the License. You may
** obtain a copy of the License at http://www.mozilla.org/MPL/
**
** Software distributed under the License is distributed on an "AS IS" basis, WITHOUT
** WARRANTY OF ANY KIND, either express or implied. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Code is the Liquid Rendering Toolkit.
**
** The Initial Developer of the Original Code is Colin Doncaster. Portions created by
** Colin Doncaster are Copyright (C) 2002. All Rights Reserved.
**
** Contributor(s): Berj Bannayan.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#ifndef liquid_H
#define liquid_H

/* ______________________________________________________________________
**
** Liquid Header File
** ______________________________________________________________________
*/

#if defined(WIN32) && !defined(DEBUG)
  // Disable double -> float conversion and signed <> unsigned mismatch warnings
  #pragma warning( disable : 4244 4305 4018 )
#endif

#include <math.h>
#include <string>
#include <assert.h>
#include <stdio.h>


#ifndef WIN32
#include <sys/param.h>
#include <sys/times.h>
#include <sys/types.h>
#endif

#if defined(_WIN32) && !defined(M_PI)
#  define M_PI 3.1415926535897932384626433832795
#endif

#ifdef OSX
#  include <stdlib.h>
#else
#  include <malloc.h>
#endif

#include <liqMemory.h>
#include <liqIOStream.h>

#include <maya/M3dView.h>
#include <maya/MComputation.h>
#include <maya/MString.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>

////////////////////////
// Macros and Defines //
////////////////////////
#ifndef debugMode
extern int debugMode;
#endif

#define HERE  cout<<"at line "<<__LINE__<<" in "<<__FUNCTION__<<endl<<flush;

#define TIMER_START       struct tms t,u;long r1,r2;r1 = times(&t);
#define TIMER_STOP(msg)   r2 = times(&u); cout <<"[liquid timer] "<<msg<<" :"<<endl<<"\t  user time = "<<((float)(u.tms_utime-t.tms_utime))/(HZ)<<endl<<"\tsystem time = "<<((float)(u.tms_stime-t.tms_stime))/(HZ)<<endl;


#if !defined(LINUX) && !defined(OSX)
#define STDERR stderr
#  ifndef LIQDEBUGPRINTF
#    define LIQDEBUGPRINTF(msg,...) if( debugMode ) fprintf(STDERR,(msg),__VA_ARGS__); fflush(STDERR); 
#  endif
#else
#define STDERR stdout
// gcc compatible variable args macro version of LIQDEBUGPRINTF
#  ifndef LIQDEBUGPRINTF
#    define LIQDEBUGPRINTF(msg,...) if( debugMode ) { fprintf(STDERR,(msg),## __VA_ARGS__) ; fflush(STDERR); }
#  endif
#endif

#ifndef LIQCHECKSTATUS
#define LIQCHECKSTATUS(stat,msg) \
  if (!(stat)) { \
    stat.perror((msg)); \
    return (stat); \
  }
#endif

#define LIQ_CANCEL_FEEDBACK_MESSAGE MString( "Liquid -> RIB Generation Cancelled!\n" )
#define LIQ_CHECK_CANCEL_REQUEST    if ( m_escHandler.isInterruptRequested() ) throw( LIQ_CANCEL_FEEDBACK_MESSAGE )
#define LIQ_ADD_SLASH_IF_NEEDED(a) if(a.length()){ if ( a.asChar()[a.length() - 1] != '/' ){ a += "/";}}
#define LIQ_ANIM_EXT MString( ".%0*d");
#define LIQ_SET_EXT MString( ".%0*s");

/* between Maya 3.0 /4.0/Linux/Other Platforms some functions changed their input type from long to int so
a stand-in type called liquidlong was created to get around the problem */
#ifdef LINUX
typedef int liquidlong;
#else
#if MAYA_API_VERSION > 300
typedef int liquidlong;
#else
typedef long liquidlong;
#endif
#endif

// Equivalence test for floats.  Equality tests are dangerous for floating
// point values
//

#define FLOAT_EPSILON 0.0001
inline bool equiv( float val1, float val2 )
{
  return ( fabsf( val1 - val2 ) < FLOAT_EPSILON );
}

// Specifies how the start/end frame is set
//
#define USE_TIMESLIDER 1
#ifndef MM_TO_INCH
#  define MM_TO_INCH 0.03937
#endif

#define LIQMAXMOTIONSAMPLES 16

///////////
// Enums //
///////////
enum ObjectType {
  MRT_Unknown         = 0,
  MRT_Nurbs           = 1,
  MRT_Mesh            = 2,
  MRT_Light           = 3,
  MRT_Weirdo          = 4,
  MRT_NuCurve         = 5,
  MRT_Particles       = 6,
  MRT_Locator         = 7,
  MRT_RibGen          = 8,
  MRT_Shader          = 9,
  MRT_Coord           = 10,
  MRT_Subdivision     = 11,
  MRT_MayaSubdivision = 12,
  MRT_Custom          = 13,
  MRT_ClipPlane       = 14,
  MRT_PfxToon         = 15,
  MRT_PfxHair         = 16,
  MRT_PfxTube         = 17,
  MRT_PfxLeaf         = 18,
  MRT_PfxPetal        = 19,
  MRT_ImplicitSphere  = 20,
  MRT_Curves          = 21
};

enum LightType {
  MRLT_Unknown  = 0,
  MRLT_Ambient  = 1,
  MRLT_Distant  = 2,
  MRLT_Point    = 3,
  MRLT_Spot     = 4,
  MRLT_Rman     = 5,
  MRLT_Area     = 6
};

enum AnimType {
  MRX_Const         = 0,
  MRX_Animated      = 1,
  MRX_Incompatible  = 2
};

enum RendererType {
  PRMan   = 0,
  BMRT    = 1,
  RDC     = 2
};

enum PointLightDirection {
  pPX     = 0,
  pPY     = 1,
  pPZ     = 2,
  pNX     = 3,
  pNY     = 4,
  pNZ     = 5
};


enum PixelFilterType {
  pfBoxFilter            = 0,
  pfTriangleFilter       = 1,
  pfCatmullRomFilter     = 2,
  pfGaussianFilter       = 3,
  pfSincFilter           = 4,

  pfBlackmanHarrisFilter = 5,
  pfMitchellFilter       = 6,
  pfSepCatmullRomFilter  = 7,

  pfLanczosFilter        = 8,
  pfBesselFilter         = 9,
  pfDiskFilter           = 10
};

enum HiderType {
  htHidden    = 0,
  htPhoton    = 1,
  htZbuffer   = 2,
  htRaytrace  = 3,
  htOpenGL    = 4,
  htDepthMask = 5
};


enum VolumeInterpretation {
  viNone       = 0, // renderer doesn't support DSMs
  viDiscrete   = 1,
  viContinuous = 2
};

enum TransmissionType { // shadow cast attribute
  trNone        = 0,  // not set
  trTransparent = 1,
  trOpaque      = 2,
  trOs          = 3,
  trShader      = 4
};

enum fileGenMode {
  fgm_shadow_tex,
  fgm_shadow_rib,
  fgm_shadow_archive,
  fgm_scene_archive,
  fgm_hero_rib,
  fgm_image
};

enum ShadowType {
  stStandart = 0,
  stMidPoint = 1,
  stMinMax   = 2,
  stDeep     = 3
};

enum ShadowHiderType {
  shNone     = 0,
  shMin      = 1,
  shMax      = 2,
  shAverage  = 3,
  shMidPoint = 4
};

struct structCamera {
  MMatrix  mat;   // camera inverse matrix
  double  neardb;
  double  fardb;
  double  hFOV;
  int     isOrtho;
  double  orthoWidth;
  double  orthoHeight;
  MString name;
  bool    motionBlur;
  double  shutter;
  double  fStop;
  double  focalDistance;
  double  focalLength;
  double  horizontalFilmOffset;
  double  verticalFilmOffset;
  int width;
  int height;
  double fov_ratio;
  structCamera *rightCam;
  structCamera *leftCam;
};

enum RenderPass {
  rpNone    = -1,
  rpHeroPass    = 0,
  rpShadowPass  = 1, // special shadow pass for compositing purpose
  rpShadowMap   = 2,
  rpReflectMap  = 3,
  rpEnvMap      = 4,
  rpMakeTexture = 5
};

struct structJob {
  MString name;
  MString renderName;
  
  MString ribFileName;

  MString texName;
  
  MString imageName;
  MString imageMode;
  MString format;

  int     width, height;
  float   aspectRatio;

  short   samples;
  float   shadingRate;
  float   shadingRateFactor;

  RenderPass  pass;
  
  bool    isShadowPass;
  bool    isStereoPass;

  int     renderFrame;

  // shadows specific job options
  // bool                  isShadow;
  bool    everyFrame;
  
  MString shadowObjectSet;
  bool    shadowArchiveRibDone;
  
  ShadowType      shadowType;
  ShadowHiderType shadowHiderType;

  bool    hasShadowCam;
  bool    shadowAggregation;
  
  // MidPoint shadows specific job options
  //bool    isMidPointShadow;
  float   midPointRatio;

  // MinMax shadows specific job options
  //bool    isMinMaxShadow;

  // Deep shadows specific job options
  //bool                  deepShadows;
  //int                   shadowPixelSamples;
  VolumeInterpretation  volume;
  //int                   shadowVolumeInterpretation;
  MString deepShadowOption; // deep shadows display driver option
  
  // pointlight shadow job options
  bool                  isPoint;
  PointLightDirection   pointDir;

  // camera job options
  structCamera          camera[ LIQMAXMOTIONSAMPLES ];
  structCamera          leftCamera[ LIQMAXMOTIONSAMPLES ];    // stereo cam
  structCamera          rightCamera[ LIQMAXMOTIONSAMPLES ];    // stereo cam

  MDagPath              path;
  MDagPath              shadowCamPath;

  MString               jobOptions;
  bool                  gotJobOptions;

  MString               jobFrameRib;
  bool                  gotJobFrameRib;
  
  bool                  skip;
};

typedef enum {
  TAG_CREASE,
  TAG_HOLE,
  TAG_CORNER,
  TAG_BOUNDARY,
  TAG_STITCH,
  TAG_FACEVARYINGBOUNDARY
} SBD_EXTRA_TAG;
#endif
