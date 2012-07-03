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
extern long     liqglo_lframe;

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

  return syn;
}

MStatus liqJobList::doIt(const MArgList& args)
{
  MStatus status;
  MArgParser argParser(syntax(), args);

  doShadows = false;
  int flagIndex = args.flagIndex("sh", "shadows");
  if (flagIndex != MArgList::kInvalidArgIndex) 
  {
    doShadows = true;
  }

  doSingleShadows = false;
  flagIndex = args.flagIndex("ssh", "singleShadows");
  if (flagIndex != MArgList::kInvalidArgIndex) 
  {
    doSingleShadows = true;
  }

  doCamera = false;
  flagIndex = args.flagIndex("cam", "camera");
  if (flagIndex != MArgList::kInvalidArgIndex) 
  {
    doCamera = true;
  }

  fullPath = false;
  flagIndex = args.flagIndex("fp", "fullPath");
  if (flagIndex != MArgList::kInvalidArgIndex) 
  {
    fullPath = true;
  }

  debug = false;
  flagIndex = args.flagIndex("d", "debug");
  if (flagIndex != MArgList::kInvalidArgIndex) 
  {
    debug = true;
  }

  if ( !doShadows && !doSingleShadows && !doCamera ) 
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
    //
    // set the current scene name
    //
    liqglo_sceneName = liquidTransGetSceneName();
    //
    // set the frame
    //
    liqglo_lframe = ( int ) MAnimControl::currentTime().as( MTime::uiUnit() );
    //
    // read the globals
    //
    LIQDEBUGPRINTF( "read globals...");
    if ( ribTranslator.liquidInitGlobals() ) 
      ribTranslator.liquidReadGlobals();
    else 
    {
      MString err("no liquidGlobals node in the scene");
      throw err;
    }
    LIQDEBUGPRINTF( "done !");
    //
    // verify the output directories
    //
    if ( ribTranslator.verifyOutputDirectories() ) 
    {
      MString err("The output directories are not properly setup in the globals");
      throw err;
    }
    //
    // build the job list
    //
    LIQDEBUGPRINTF("  build jobs..." );
    if ( ribTranslator.buildJobs() != MS::kSuccess ) 
    {
      MString err("buildJob() Failed");
      throw err;
    }
    LIQDEBUGPRINTF( "done !");

    std::vector<structJob>::iterator iterShad = ribTranslator.jobList.begin();
    //
    // get the shadows
    //
    if ( doShadows || doSingleShadows ) 
    {
      LIQDEBUGPRINTF("  do shadows..." );
      while ( iterShad != ribTranslator.jobList.end() ) 
      {
        if ( doShadows && iterShad->isShadow && iterShad->everyFrame ) 
          result.append( liquidGetRelativePath(fullPath, iterShad->ribFileName, liqglo_projectDir) );
        if ( doSingleShadows && iterShad->isShadow && !iterShad->everyFrame ) 
          result.append( liquidGetRelativePath(fullPath, iterShad->ribFileName, liqglo_projectDir) );
        ++iterShad;
      }
      LIQDEBUGPRINTF( "done !");
    }
    //
    // get the camera
    //
    if ( doCamera ) 
    {
      LIQDEBUGPRINTF( "  do camera..." );
      iterShad = ribTranslator.jobList.end();
      --iterShad;
      result.append( liquidGetRelativePath(fullPath, iterShad->ribFileName, liqglo_projectDir) );
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


