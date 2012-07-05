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

/* ______________________________________________________________________
**
** Liquid Executable
** ______________________________________________________________________
*/

#include <signal.h>

#ifdef _WIN32
#  pragma warning(disable:4786)
#endif

// Renderman Headers
extern "C" {
#include <ri.h>
}

// Maya's Headers
#include <maya/MArgList.h>
#include <maya/MLibrary.h>
#include <maya/MFileIO.h>
#include <maya/MGlobal.h>

#include <liquid.h>
#include <liqRibTranslator.h>
#include <liqGlobalHelpers.h>

#if defined(_WIN32)/* && !defined(DEFINED_LIQUIDVERSION)*/
// unix build gets this from the Makefile
static const char* LIQUIDVERSION =
#include "liquid.version"
;
#define DEFINED_LIQUIDVERSION
#endif

bool liquidBin = true;

static const char* usage =
"\nUsage: liquid [options] filename\n\
\n\
\t-GL     -useGlobals                       should always be the first flag\n\
\t-sel    -selected\n\
\n\
Frame\n\
\t-cam    -camera <name>\n\
\t-rcam   -rotateCamera\n\
\t-x      -width <n>\n\
\t-y      -height <n>\n\
\t-ar     -aspect <n>\n\
\t-n      -sequence <start> <stop> <step>\n\
\t-fl     -frameList <n,n,n,...>\n\
\t-mb     -motionBlur\n\
\t-db     -deformationBlur\n\
\t-m      -mbSamples <n>\n\
\t-blt    -blurTime <n>\n\
\t-dof    -dofOn\n\
\n\
Files\n\
\t-rnm    -ribName <name>\n\
\t-pad    -padding <n>\n\
\t-pid    -picdir <path>\n\
\t-txd    -texdir <path>\n\
\t-rid    -ribdir <path>\n\
\t-tmd    -tmpdir <path>\n\
\t-pd     -projectDir <path>\n\
\t-prm    -preFrameMel <string>\n\
\t-pom    -postFrameMel <string>\n\
\t-shn    -shotName <string>\n\
\t-shv    -shotVersion <string>\n\
\t-cmd    -createMissingDirs\n\
\t-rel    -relativePaths\n\
\n\
Image\n\
\t-bs     -bucketSize <x> <y>\n\
\t-cw     -cropWindow <x1> <x2> <y1> <y2>\n\
\t-es     -eyeSplits <n>\n\
\t-gs     -gridSize <n>\n\
\t-pf     -pixelFilter <n> <n> <n>\n\
\t-s      -samples <n>\n\
\t-sr     -shadingRate <n>\n\
\t-txm    -texmem <n>\n\
\n\
Options\n\
\t-acv    -allCurves\n\
\t-bin    -doBinary\n\
\t-cln    -cleanRib\n\
\t-d      -debug\n\
\t-err    -errHandler\n\
\t-fsr    -fullShadowRib\n\
\t-nsh    -noShadows\n\
\t-pro    -progress\n\
\t-ra     -readArchive\n\
\t-sh     -shadows\n\
\t-sfso   -singleFrameShadowsOnly\n\
\t-nsfs   -noSingleFrameShadows\n\
\t-sdb    -shaderDebug\n\
\t-tif    -tiff\n\
\t-zip    -doCompression\n\
\t-lyr    -layer\n\
\n\
Job\n\
\t-dbs    -defBlock\n\
\t-lr     -launchRender\n\
\t-net    -netRender\n\
\t-ndf    -noDef\n\
\t-nolr   -noLaunchRender\n\
\t-nrs    -noRenderScript\n\
\t-pec    -preCommand <string>\n\
\t-poc    -postJobCommand <string>\n\
\t-pof    -postFrameCommand <string>\n\
\t-prf    -preFrameCommand <string>\n\
\t-rem    -remote\n\
\t-rec    -renderCommand <string>\n\
\t-rgc    -ribgenCommand <string> \n\
\t-rgo    -ribGenOnly\n\
\t-rs     -renderScript\n\
\n\
RenderView\n\
\t-rv     -renderView\n\
\t-rvl    -renderViewLocal\n\
\t-rvp    -renderViewPort <n>\n\
\n\
Please see the Liquid Wiki for command line options.\n\
The options match the liquid MEL command parameters.\n";


void signalHandler(int sig)
{
  if (sig == SIGTERM) 
  {
    throw( MString( "Liquid command terminated!\n" ) );
  } else 
  {
    signal(sig, signalHandler);
  }
}

static bool isHelpArg(const char *arg) 
{
  return (
    !strcmp(arg, "-h")     ||
    !strcmp(arg, "-help")  ||
    !strcmp(arg, "--help")
  );
}

int main(int argc, char **argv)
//
//  Description:
//      Register the command when the plug-in is loaded
//
{
  MStatus status;
  MString command;
  MString UserClassify;
  MString fileName;

  //liquidBin = true;
  
  liquidMessage( LIQUIDVERSION, messageInfo );
  
  // cerr << "liquidBin = " << liquidBin << endl << flush; 
  
  char *maya_location = getenv( "MAYA_LOCATION" );
  if ( maya_location == NULL )
  {
    liquidMessage( "MAYA_LOCATION not defined", messageError );
    return 1;
  }  
  
  // initialize the maya library
  status = MLibrary::initialize ( true, argv[0], true );
  if ( !status ) 
  {
    status.perror("MLibrary::initialize");
    return 1;
  }
//  MString melSourceUserSetupCmd = "if(exists(\"userSetup\")){print(\"[liquidBin] source userSetup!\\\n\");source \"userSetup\";}'";
  MString melSourceUserSetupCmd = "if(exists(\"userSetup\")){source \"userSetup\";}";
  MGlobal::executeCommand(melSourceUserSetupCmd);
  
  // start building an argument list to
  // pass to the Maya liquid command
  MArgList myArgs;
  MString mainArg = "-GL";
  myArgs.addArg( mainArg );

#ifdef SIGRTMAX
#ifndef _WIN32
  for ( unsigned int i(0); i <= SIGRTMAX; i++) 
  {
    signal( i, signalHandler );
  }
#endif
#endif

  // user must at least specify the Maya scene file to render
  if ((argc < 2) || isHelpArg(argv[1])) 
  {
    cerr << usage << endl;
    return 1;
  }

  // now we grab the last argument as the Maya scene filename
  // and all the rest in the middle we gather to pass straight
  // through to the liquid Maya command
  fileName = argv[argc-1];
  for( unsigned int i(1) ; i<(unsigned int)(argc-1); ++i ) 
  {
    MString newArg = argv[i];
    myArgs.addArg( newArg );
  }

  // check that the filename has been specified and exists
  if ( fileName == "" ) 
  {
    status.perror("Liquid -> no filename specified!\n" );
    printf( "ALF_EXIT_STATUS 1\n" );
    MLibrary::cleanup( 1 );
    return (1);
  }

  if ( !fileExists( fileName ) ) 
  {
    status.perror("Liquid -> file not found: " + fileName + "\n");
    printf( "ALF_EXIT_STATUS 1\n" );
    MLibrary::cleanup( 1 );
    return ( 1 );
  }
 
  // load the file into liquid's virtual maya
  status = MFileIO::open( fileName, NULL, true );  // true for  `file -o -f`
  if ( !status ) 
  {
    MString error = " Error opening file: ";
    error += fileName.asChar();
    status.perror( error );
    printf( "ALF_EXIT_STATUS 1\n" );
    MLibrary::cleanup( 1 );
    return( 1 ) ;
  }
  
  liqRibTranslator liquidTrans;

#ifdef SIGRTMAX
#ifndef _WIN32
  for ( unsigned int i(0); i <= SIGRTMAX ; i++ ) 
  {
    signal(i, signalHandler);
  }
#endif
#endif
  
  status = liquidTrans.doIt( myArgs );

  if (status) 
    printf( "ALF_EXIT_STATUS 0\n" );
  else 
    printf( "ALF_EXIT_STATUS 1\n" );

  MLibrary::cleanup( 0 );
  return (0);
}
