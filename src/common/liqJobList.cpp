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
** Contributor(s): Philippe Leprince.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#include <liquid.h>
#include <liqJobList.h>
#include <liqGlobalHelpers.h>

#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include <maya/MStringArray.h>
#include <maya/MGlobal.h>
#include <maya/MArgParser.h>
#include <maya/MAnimControl.h>

#include <liqIOStream.h>

#include <ri.h>

// RI_VERBATIM is in the current RenderMan spec but
// some RIB libraries don't know about it
#ifndef RI_VERBATIM
  #define RI_VERBATIM "verbatim"
#endif

extern MString  liqglo_ribDir;
extern MString  liqglo_textureDir;
extern MString  liqglo_projectDir;
extern MString  liqglo_sceneName;
extern MString  liqglo_renderCamera;
extern long     liqglo_lframe;

extern int debugMode;

void* liqJobList::creator()
{
  return new liqJobList();
}

MSyntax liqJobList::syntax()
{
  MSyntax syn;

  syn.addFlag("sh",  "shadows");
  syn.addFlag("ssh", "singleShadows");
  syn.addFlag("cam", "camera");
  syn.addFlag("fp",  "fullPath");
  syn.addFlag("d",   "debug");
  syn.addFlag("i",   "info");

  return syn;
}

MStatus liqJobList::doIt( const MArgList& args )
{
  MStatus status;
  MArgParser argParser(syntax(), args);

  doShadows = false;
  int flagIndex = args.flagIndex("sh", "shadows");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
  {
    doShadows = true;
  }

  doSingleShadows = false;
  flagIndex = args.flagIndex("ssh", "singleShadows");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
  {
    doSingleShadows = true;
  }

  doCamera = false;
  flagIndex = args.flagIndex("cam", "camera");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
  {
    doCamera = true;
  }

  fullPath = false;
  flagIndex = args.flagIndex("fp", "fullPath");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
  {
    fullPath = true;
  }

  debug = false;
  flagIndex = args.flagIndex("d", "debug");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
  {
    debug = true;
    debugMode = true;
  }

  info = false;
  flagIndex = args.flagIndex("i", "info");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
  {
    info = true;
  }

  if ( !doShadows && !doSingleShadows && !doCamera && !info ) 
  {
    status = MS::kFailure;
    MString err( "LiquidJobList : Not enough valid flags specified" );
    status.perror( err );
    MGlobal::displayError( err );
    return status;
  }

  result.clear();

  return redoIt();
}

MStatus liqJobList::redoIt()
{
  LIQDEBUGPRINTF( "redoIt" );
  clearResult();
  
  MStatus status;
  MObject cameraNode;
  MDagPath lightPath;

  liqRibTranslator ribTranslator;

  try {

    ribTranslator.m_escHandler.beginComputation();
    //
    // set the current project directory
    //
    MString MELCommand = "workspace -q -rd";
    MString MELReturn;
    MGlobal::executeCommand( MELCommand, MELReturn );
    liqglo_projectDir = MELReturn;

    //liquidMessage ( "liqglo_projectDir = '" + liqglo_projectDir + "'", messageInfo );
    //
    // set the current scene name
    //
    liqglo_sceneName = liquidTransGetSceneName();
    
    //liquidMessage ( "liqglo_sceneName = '" + liqglo_sceneName + "'", messageInfo );
    //
    // set the frame
    //
    liqglo_lframe = ( int ) MAnimControl::currentTime().as( MTime::uiUnit() );
    //
    // read the globals
    //
    LIQDEBUGPRINTF( "read globals..." );
    if ( ribTranslator.liquidInitGlobals() ) 
      ribTranslator.liquidReadGlobals();
    else 
    {
      MString err ( "no liquidGlobals node in the scene" );
      throw err;
    }
    LIQDEBUGPRINTF( "done !");
    
    // liquidMessage ( "liqglo_sceneName = '" + liqglo_renderCamera + "'", messageInfo );

    //
    // verify the output directories
    //
    if ( ribTranslator.verifyOutputDirectories() ) 
    {
      MString err( "The output directories are not properly setup in the globals" );
      throw err;
    }
    //
    // build the job list
    //
    LIQDEBUGPRINTF("  build jobs..." );
    if ( ribTranslator.buildJobs() != MS::kSuccess ) 
    {
      MString err( "buildJob() Failed" );
      throw err;
    }
    LIQDEBUGPRINTF( "done !");
    //
    // get jobs info
    //
    /*
    struct structJob {
      int      width, height;
      float    aspectRatio;
      MString  name;
      MString  texName;
      MString  imageMode;
      MString  format;
      MString  renderName;
      MString  ribFileName;
      MString  imageName;
      bool     isShadow;
      bool     isMinMaxShadow;
      bool     isMidPointShadow;
      float    midPointRatio;

      short   samples;
      float   shadingRate;
      float   shadingRateFactor;

      RenderPass  pass;
      
      enum RenderPass {
        rpHeroPass    = 0,
        rpShadowPass  = 1, // special shadow pass for compositing purpose
        rpShadowMap   = 2,
        rpReflectMap  = 3,
        rpEnvMap      = 4,
        rpMakeTexture = 5
      };

      // shadows specific job options
      ShadowType            shadowType;
      enum ShadowType {
        stStandart = 0,
        stMinMax   = 1,
        stDeep     = 2
      };

      ShadowHiderType       shadowHiderType;
      enum ShadowHiderType {
        shNone     = 0,
        shMin      = 1,
        shMax      = 2,
        shAverage  = 3,
        shMidPoint = 4
      };

      VolumeInterpretation  volume;
      enum VolumeInterpretation {
        viNone       = 0, // renderer doesn't support DSMs
        viDiscrete   = 1,
        viContinuous = 2
      };

      MString               deepShadowOption; // deep shadows display driver option

      bool                  hasShadowCam;
      bool                  isShadowPass;
      bool                  isStereoPass;
      int                   shadowPixelSamples;
      int                   shadowVolumeInterpretation;
      bool                  shadowAggregation;
      bool                  isPoint;
      PointLightDirection   pointDir;
      enum PointLightDirection {
        pPX     = 0,
        pPY     = 1,
        pPZ     = 2,
        pNX     = 3,
        pNY     = 4,
        pNZ     = 5
      };

      structCamera          camera[ LIQMAXMOTIONSAMPLES ];
      structCamera          leftCamera[ LIQMAXMOTIONSAMPLES ];    // stereo cam
      structCamera          rightCamera[ LIQMAXMOTIONSAMPLES ];    // stereo cam
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

      MDagPath              path;
      MDagPath              shadowCamPath;
      MString               jobOptions;
      bool                  gotJobOptions;
      MString               jobFrameRib;
      bool                  gotJobFrameRib;
      bool                  deepShadows;
      bool                  everyFrame;
      int                   renderFrame;
      MString               shadowObjectSet;
      bool                  shadowArchiveRibDone;
      bool                  skip;
    };
    */
    if ( info ) 
    {
      std::vector<structJob>::iterator iterJob = ribTranslator.jobList.begin();
      LIQDEBUGPRINTF( "  do jobs info ..." );
      int i = 0;
      while ( iterJob != ribTranslator.jobList.end() )
      {
        result.append( "\n>> JOB (" + ( MString("") += (int)( i ) ) + ")\n" );

        result.append( "name = " + iterJob->name + "\n" );
        result.append( "texName = " + iterJob->texName + "\n" );
        result.append( "imageMode = " + iterJob->imageMode + "\n" );
        result.append( "format = " + iterJob->format + "\n" );
 
        result.append( "width = " + ( MString("") += (int)( iterJob->width ) ) + "\n" );
        result.append( "height = " + ( MString("") += (int)( iterJob->height ) ) + "\n" );
        result.append( "aspectRatio = " + ( MString("") += (float)( iterJob->aspectRatio ) ) + "\n" );

        result.append( "renderName = " + iterJob->renderName + "\n" );
        result.append( "ribFileName = " + iterJob->ribFileName + "\n" );
        result.append( "imageName = " + iterJob->imageName + "\n" );

        result.append( "isShadow = " + MString( ( iterJob->isShadow )? "Yes" : "No" ) + "\n" );
        result.append( "isMinMaxShadow = " + MString( ( iterJob->isMinMaxShadow )? "Yes" : "No" ) + "\n" );
        result.append( "isMidPointShadow = " + MString( ( iterJob->isMidPointShadow )? "Yes" : "No" ) + "\n" );

        result.append( "midPointRatio = " + ( MString("") += (float)(iterJob->midPointRatio) ) + "\n" );

        result.append( "samples = " + ( MString("") += (int)( iterJob->samples ) ) + "\n" );
        result.append( "shadingRate = " + ( MString("") += (float)( iterJob->shadingRate ) ) + "\n" );
        result.append( "shadingRateFactor = " + ( MString("") += (float)( iterJob->shadingRateFactor ) ) + "\n" );
        
        MString renderPass( "rpNone" ); 
        switch ( iterJob->pass )
        {
          case rpHeroPass: renderPass = "rpHeroPass"; break;
          case rpShadowPass: renderPass = "rpShadowPass"; break;
          case rpShadowMap: renderPass = "rpShadowMap"; break;
          case rpReflectMap: renderPass = "rpReflectMap"; break;
          case rpEnvMap: renderPass = "rpEnvMap"; break;
          case rpMakeTexture: renderPass = "rpMakeTexture"; break;
        }
        
        result.append( "renderPass = " + renderPass + "\n" );

        MString shadowType(""); 
        switch ( iterJob->shadowType )
        {
          case stStandart: shadowType = "stStandart"; break;
          case stMidPoint: shadowType = "stMidPoint"; break;
          case stMinMax: shadowType = "stMinMax"; break;
          case stDeep: shadowType = "stDeep"; break;
        }
        result.append( "shadowType = " + shadowType + "\n" );

        MString shadowHiderType(""); 
        switch ( iterJob->shadowHiderType )
        {
          case shNone: shadowHiderType = "shNone"; break;
          case shMin: shadowHiderType = "shMin"; break;
          case shMax: shadowHiderType = "shMax"; break;
          case shAverage: shadowHiderType = "shAverage"; break;
          case shMidPoint: shadowHiderType = "shMidPoint"; break;
        }
        result.append( "shadowHiderType = " + shadowHiderType + "\n" );
        
        MString volume(""); 
        switch ( iterJob->volume )
        {
          case viNone: volume = "viNone"; break;
          case viDiscrete: volume = "viDiscrete"; break;
          case viContinuous: volume = "viContinuous"; break;
        }
        result.append( "VolumeInterpretation  volume = " + volume + "\n" );
        result.append( "deepShadowOption = " + iterJob->deepShadowOption + "\n" );

        result.append( "hasShadowCam = " + MString( ( iterJob->hasShadowCam )? "Yes" : "No" ) + "\n" );
        result.append( "isShadowPass = " + MString( ( iterJob->isShadowPass )? "Yes" : "No" ) + "\n" );
        result.append( "isStereoPass = " + MString( ( iterJob->isStereoPass )? "Yes" : "No" ) + "\n" );

        result.append( "shadowPixelSamples = " + ( MString("") += (int)( iterJob->shadowPixelSamples ) ) + "\n" );
        result.append( "shadowVolumeInterpretation = " + ( MString("") += (int)( iterJob->shadowVolumeInterpretation ) ) + "\n" );

        result.append( "shadowAggregation = " + MString( ( iterJob->shadowAggregation )? "Yes" : "No" ) + "\n" );
        result.append( "isPoint = " + MString( ( iterJob->isPoint )? "Yes" : "No" ) + "\n" );
        
        MString pointDir("");
        const char *namePointLightDirection[] = { "pPX", "pPY", "pPZ", "pNX", "pNY", "pNZ" };
        if ( iterJob->pointDir >= pPX && iterJob->pointDir <= pNZ ) 
          pointDir = MString( namePointLightDirection[ iterJob->pointDir ] );
        result.append( "pointDir = " + pointDir + "\n" );

        result.append( "camera.name = " + iterJob->camera[0].name + "\n" );
        result.append( "camera.width = " + ( MString("") += (int)( iterJob->camera[0].width ) ) + "\n" );
        result.append( "camera.height = " + ( MString("") += (int)( iterJob->camera[0].height ) ) + "\n" );
        result.append( "camera.fov_ratio = " + ( MString("") += (double)( iterJob->camera[0].fov_ratio ) ) + "\n" );
        result.append( "camera.hFOV = " + ( MString("") += (double)( iterJob->camera[0].hFOV ) ) + "\n" );

        result.append( "camera.isOrtho = " + MString( ( iterJob->camera[0].isOrtho )? "Yes" : "No" ) + "\n" );
        result.append( "camera.orthoWidth = " + ( MString("") += (int)( iterJob->camera[0].orthoWidth ) ) + "\n" );
        result.append( "camera.orthoHeight = " + ( MString("") += (int)( iterJob->camera[0].orthoHeight ) ) + "\n" );

        result.append( "MDagPath path = " + iterJob->path.fullPathName() + "\n" );
        result.append( "MDagPath shadowCamPath = " + iterJob->shadowCamPath.fullPathName() + "\n" );

        result.append( "jobOptions = " + iterJob->jobOptions + "\n" );
        result.append( "gotJobOptions = " + MString( ( iterJob->gotJobOptions )? "Yes" : "No" ) + "\n" );
        result.append( "jobFrameRib = " + iterJob->jobFrameRib + "\n" );
        result.append( "gotJobFrameRib = " + MString( ( iterJob->gotJobFrameRib )? "Yes" : "No" ) + "\n" );

        result.append( "deepShadows = " + MString( ( iterJob->deepShadows )? "Yes" : "No" ) + "\n" );

        result.append( "everyFrame = " + MString( ( iterJob->everyFrame )? "Yes" : "No" ) + "\n" );
        result.append( "renderFrame = " + ( MString("") += (int)( iterJob->renderFrame ) ) + "\n" );

        result.append( "shadowObjectSet = " + iterJob->shadowObjectSet + "\n" );
        result.append( "shadowArchiveRibDone = " + MString( ( iterJob->shadowArchiveRibDone )? "Yes" : "No" ) + "\n" );

        result.append( "skip = " + MString( ( iterJob->skip )? "Yes" : "No" ) + "\n" );

        ++i;
        ++iterJob;
      }
      LIQDEBUGPRINTF( "done !");
    }
    //
    // get the shadows
    //
    if ( doShadows || doSingleShadows ) 
    {
      std::vector<structJob>::iterator iterShad = ribTranslator.jobList.begin();

      LIQDEBUGPRINTF("  do shadows..." );
      while ( iterShad != ribTranslator.jobList.end() ) 
      {
        if ( doShadows && iterShad->isShadow && iterShad->everyFrame ) 
          result.append( liquidGetRelativePath( !fullPath, iterShad->ribFileName, liqglo_projectDir ) );

        if ( doSingleShadows && iterShad->isShadow && !iterShad->everyFrame ) 
          result.append( liquidGetRelativePath( !fullPath, iterShad->ribFileName, liqglo_projectDir ) );
          
        ++iterShad;
      }
      LIQDEBUGPRINTF( "done !");
    }
    //
    // get the camera
    //
    if ( doCamera ) 
    {
      std::vector<structJob>::iterator iterShad = ribTranslator.jobList.end();
      LIQDEBUGPRINTF( "  do camera..." );
      --iterShad;
      result.append( liquidGetRelativePath( !fullPath, iterShad->ribFileName, liqglo_projectDir ) );
      LIQDEBUGPRINTF( "done !");
    }

    ribTranslator.m_escHandler.endComputation();

  } catch ( MString msg ) {
    //
    // catch any error
    //
    MGlobal::displayError( "liquidJobList : " + msg );
    ribTranslator.m_escHandler.endComputation();
    return MS::kFailure;
  }
  //
  // output the result
  //
  setResult( result );

  return MS::kSuccess;
}


