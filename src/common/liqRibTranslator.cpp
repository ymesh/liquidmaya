/*
**
**
** The contents of this file are subject to the Mozilla Public License Version
** 1.1  (the "License"); you may not use this file except in compliance with
** the License. You may obtain a copy of the License at
** http://www.mozilla.org/MPL/
**
** Software distributed under the License is distributed on an "AS IS" basis,
** WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
** for the specific language governing rights and limitations under the
** License.
**
** The Original Code is the Liquid Rendering Toolkit.
**
** The Initial Developer of the Original Code is Colin Doncaster. Portions
** created by Colin Doncaster are Copyright (C) 2002. All Rights Reserved.
**
** Contributor(s): Berj Bannayan.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1 989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/


#include <sys/types.h>

#ifndef _WIN32
#include <sys/time.h>
#include <sys/stat.h>
// Dynamic Object Headers
#include <dlfcn.h>
#endif

#ifdef _WIN32
#pragma warning(disable:4786)
#endif



// Renderman Headers
extern "C" {
#include <ri.h>
}

#ifdef _WIN32
#include <process.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#endif

#include <algorithm>
#include <time.h>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <strstream>
#include <iomanip>

#if defined(_WIN32) && !defined(DEFINED_LIQUIDVERSION)
// unix build gets this from the Makefile
static const char *LIQUIDVERSION =
#include "../liquid.version"
;
#define DEFINED_LIQUIDVERSION
#endif

#ifdef _WIN32
#  define RM_CMD "cmd.exe /C DEL"
#else
#  define RM_CMD "/bin/rm"
#endif

// Maya headers
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MAnimControl.h>
#include <maya/MFileIO.h>
#include <maya/MFnLight.h>
#include <maya/MFnTransform.h>
#include <maya/MItDag.h>
#include <maya/MItInstancer.h>
#include <maya/MItSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>
#include <maya/MFnSet.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MFnPfxGeometry.h>
#include <maya/MDistance.h>
#include <maya/MDagModifier.h>
#include <maya/MPxNode.h>
#include <maya/MRenderLineArray.h>

// Liquid headers
#include <liquid.h>
#include <liqRibHT.h>
#include <liqRenderer.h>
#include <liqRibTranslator.h>
#include <liqGlobalHelpers.h>
#include <liqProcessLauncher.h>
#include <liqCustomNode.h>
#include <liqShaderFactory.h>

using namespace boost;
//using namespace std;

typedef int RtError;

// this get's set if we are running the commandline version of liquid
extern bool liquidBin;
int  debugMode;

liquidVerbosityType liqglo_verbosity( verbosityAll );
// Kept global for liquidRigGenData and liquidRibParticleData
FILE        *liqglo_ribFP;
long         liqglo_lframe;
structJob    liqglo_currentJob;
bool         liqglo_doMotion;                         // Motion blur for transformations
bool         liqglo_doDef;                            // Motion blur for deforming objects
bool         liqglo_doCompression;                    // output compressed ribs
bool         liqglo_doBinary;                         // output binary ribs
bool         liqglo_relativeMotion;                   // Use relative motion blocks
RtFloat      liqglo_sampleTimes[LIQMAXMOTIONSAMPLES]; // current sample times
RtFloat      liqglo_sampleTimesOffsets[LIQMAXMOTIONSAMPLES]; // current sample times (as offsets from frame)
int          liqglo_motionSamples;                    // used to assign more than two motion blur samples!
float        liqglo_motionFactor;
float        liqglo_shutterTime;
float        liqglo_shutterEfficiency;
bool         liqglo_doShadows;                        // Kept global for liquidRigLightData
bool         liqglo_shapeOnlyInShadowNames;
MString      liqglo_sceneName;
bool         liqglo_beautyRibHasCameraName;           // if true, usual behaviour, otherwise, no camera name in beauty rib
bool         liqglo_isShadowPass;                     // true if we are rendering a shadow pass
bool         liqglo_expandShaderArrays;
bool         liqglo_shortShaderNames;                 // true if we don't want to output path names with shaders
bool         liqglo_relativeFileNames;                // true if we only want to output project relative names
// unused anymore as MStringArray.
MString      liqglo_DDimageName; 
double       liqglo_FPS;                              // Frame-rate (for particle streak length)
bool         liqglo_outputMeshUVs;                    // true if we are writing uvs for subdivs/polys (in addition to "st")
bool         liqglo_outputMeshAsRMSArrays;            // true => write uvs as arrays
bool         liqglo_noSingleFrameShadows;             // allows you to skip single-frame shadows when you chunk a render
bool         liqglo_singleFrameShadowsOnly;           // allows you to skip single-frame shadows when you chunk a render
MString      liqglo_renderCamera;                     // a global copy for liqRibPfxToonData
bool         liqglo_relativeTransforms;               // wether we do Transform or ConcatTransform (relative)
bool         liqglo_exportAllShadersParams;           // true => write all shaders parameters even with default values
bool				 liqglo_skipDefaultMatte;

// Kept global for liquidGlobalHelper
MString      liqglo_projectDir;
MString      liqglo_ribDir;
MString      liqglo_textureDir;
MString      liqglo_shaderPath;               // Shader searchpath
MString      liqglo_texturePath;             // Texture searchpath
MString      liqglo_archivePath;
MString      liqglo_proceduralPath;

// Kept global for liqRibNode.cpp
MStringArray liqglo_preReadArchive;
MStringArray liqglo_preRibBox;
MStringArray liqglo_preReadArchiveShadow;
MStringArray liqglo_preRibBoxShadow;
MString      liqglo_currentNodeName;
MString      liqglo_currentNodeShortName;

bool         liqglo_useMtorSubdiv;  // use mtor subdiv attributes
bool         liqglo_outputMayaPolyCreases;
bool         liqglo_renderAllCurves;
HiderType    liqglo_hider;

// Kept global for raytracing
bool         rt_useRayTracing;
RtFloat      rt_traceBreadthFactor;
RtFloat      rt_traceDepthFactor;
int          rt_traceMaxDepth;
RtFloat      rt_traceSpecularThreshold;
int          rt_traceRayContinuation;
int          rt_traceCacheMemory;
bool         rt_traceDisplacements;
bool         rt_traceSampleMotion;
RtFloat      rt_traceBias;
int          rt_traceMaxDiffuseDepth;
int          rt_traceMaxSpecularDepth;
RtFloat      rt_irradianceMaxError;
RtFloat      rt_irradianceMaxPixelDist;

MString      rt_irradianceGlobalHandle;
int          rt_irradianceGlobalFileMode;

MString      rt_photonGlobalHandle;
MString      rt_causticGlobalHandle;
int          rt_photonShadingModel;
int          rt_photonEstimator;

// Additionnal globals for organized people
MString      liqglo_shotName;
MString      liqglo_shotVersion;
MString      liqglo_layer;
bool         liqglo_doExtensionPadding;
int          liqglo_outPadding;

// renderer properties
liqRenderer liquidRenderer;

#if 0
#ifdef _WIN32
// Hmmmmmmmm what's this ?
int RiNColorSamples;
#endif
// these are little storage variables to keep track of the current graphics state and will eventually be wrapped in
// a specific class
#endif

// Hmmmmm should change magic to Liquid
MString liqRibTranslator::magic("##Liquid");

/**
 * Creates a new instance of the translator.
 */
void *liqRibTranslator::creator()
{
  return new liqRibTranslator();
}
/**
 * printProgress
 */
void liqRibTranslator::printProgress( unsigned stat, unsigned numFrames, unsigned where )
// for printing the progress to the Maya Console or stdout. If alfred is being used it
// will print it in a format that causes the correct formatting for the progress meters
//
// TODO - should be able to set the progress output format somehow to cater for
// different render pipelines - with a user-specifiable string in printf format?
{
  float statSize   = ( ( 1 / ( float )( numFrames + 1 ) ) / 4 ) * ( float )stat * 100.0;
  float progressf  = ( ( ( float )where / ( float )( numFrames + 1 ) ) * 100.0 ) + statSize;
  int progress     = ( int ) progressf;

  if ( liquidBin ) cout << "ALF_PROGRESS " << progress << "%" << endl << flush;
  else 
  {
    strstream progressOutput;
    progressOutput << "Progress: " << progress << "%" << ends;
    liquidMessage( progressOutput.str(), messageInfo );
  }
}
/**
 * Class constructor.
 */
liqRibTranslator::liqRibTranslator()
{
  char* envTmp;
  if ( ( envTmp = getenv( "TMP" ) ) ||
       ( envTmp = getenv( "TEMP" ) ) ||
       ( envTmp = getenv( "TEMPDIR" ) ) ) 
  {
    m_systemTempDirectory = envTmp;
    m_systemTempDirectory = liquidSanitizePath( m_systemTempDirectory );
    LIQ_ADD_SLASH_IF_NEEDED( m_systemTempDirectory );
  }
  else 
  {
#ifndef _WIN32
    m_systemTempDirectory = "/tmp/";
#else
    m_systemTempDirectory = "%SystemRoot%/Temp/";
#endif
  }

  m_rFilterX = 1;
  m_rFilterY = 1;
  m_rFilter = pfBoxFilter;

  m_showProgress = false;
  m_deferredBlockSize = 1;
  m_deferredGen = false;
  m_rgain = 1.0;
  m_rgamma = 1.0;
  m_outputHeroPass = true;
  m_outputShadowPass = false;
  m_illuminateByDefault = false;
  m_liquidSetLightLinking = false;
  m_ignoreLights = false;
  m_ignoreSurfaces = false;
  m_ignoreDisplacements = false;
  m_ignoreVolumes = false;
  m_renderAllCurves = false;
  m_renderSelected = false;
  m_exportReadArchive = false;
  m_justRib = false;
  m_animation = false;
  m_useFrameExt = true;  // Use frame extensions
  m_lazyCompute = false;
  m_outputShadersInShadows = false;
  m_outputShadersInDeepShadows = false;
  m_outputLightsInDeepShadows = false;
  m_alfredExpand = false;
  m_errorMode = 0;
  m_blurTime = 1.0;
  m_outputComments = false;
  m_shaderDebug = false;

  m_pixDir = "rmanpix/";
  m_tmpDir = "rmantmp/";
  
#ifdef AIR
  m_renderCommand = "air";
#elif defined( AQSIS )
  m_renderCommand = "aqsis";
#elif defined( DELIGHT )
  m_renderCommand = "renderdl";
#elif defined( PIXIE )
  m_renderCommand = "rndr";
#elif defined( PRMAN )
  #ifdef _WIN32
  m_renderCommand = "prman";
  #else
  m_renderCommand = "render";
  #endif
#endif
  m_ribgenCommand = "liquid";

// display channels defaults
  m_channels.clear();

  // Display Driver Defaults
  m_displays.clear();

  m_renderView        = false;
  m_renderViewCrop    = false;
  m_renderViewLocal   = true;
  m_renderViewPort    = 6667;
  m_renderViewTimeOut = 10;

  m_statistics        = 0;
  m_statisticsFile    = "";

  m_hiddenJitter = 1;
  // PRMAN 13 BEGIN
  m_hiddenAperture[0] = 0.0;
  m_hiddenAperture[1] = 0.0;
  m_hiddenAperture[2] = 0.0;
  m_hiddenAperture[3] = 0.0;
  m_hiddenShutterOpening[0] = 0.0;
  m_hiddenShutterOpening[0] = 1.0;
  // PRMAN 13 END
  m_hiddenOcclusionBound = 0;
  m_hiddenMpCache = true;
  m_hiddenMpMemory = 6144;
  m_hiddenMpCacheDir = ".";
  m_hiddenSampleMotion = true;
  m_hiddenSubPixel = 1;
  m_hiddenExtremeMotionDof = false;
  m_hiddenMaxVPDepth = -1;
  // PRMAN 13 BEGIN
  m_hiddenSigma = false;
  m_hiddenSigmaBlur = 1.0;
  // PRMAN 13 END
  m_raytraceFalseColor = 0;
  m_photonEmit = 0;
  m_photonSampleSpectrum = 0;

  m_depthMaskZFile = "";
  m_depthMaskReverseSign = false;
  m_depthMaskDepthBias = 0.01;

  m_minCPU = m_maxCPU = 1;
  m_cropX1 = m_cropY1 = 0.0;
  m_cropX2 = m_cropY2 = 1.0;
  liqglo_isShadowPass = false;

  m_bakeNonRasterOrient    = false;
  m_bakeNoCullBackface    = false;
  m_bakeNoCullHidden    = false;

  m_renderScriptFormat = XML;
  m_exportSpecificList = false;
	m_exportOnlyObjectBlock = false;

	m_skipVisibilityAttributes = false;
	m_skipShadingAttributes = false;
	m_skipRayTraceAttributes = false;
  m_isStereoCamera = false;

  m_preFrameRIB.clear();
  m_preWorldRIB.clear();
  m_postWorldRIB.clear();
  m_preGeomRIB.clear();

  m_alfredTags.clear();
  m_alfredServices.clear();
  m_dirmaps.clear();
  m_defGenKey.clear();
  m_defGenService.clear();
  m_preFrameMel.clear();
  m_postFrameMel.clear();
  m_preCommand.clear();
  m_preJobCommand.clear();
  m_postJobCommand.clear();
  m_postFrameCommand.clear();
  m_preFrameCommand.clear();
  m_preTransformMel.clear();
  m_postTransformMel.clear();
  m_preShapeMel.clear();
  m_postShapeMel.clear();
  

  liqglo_shortShaderNames = false;
  liqglo_relativeFileNames = false;
  liqglo_doBinary = false;
  liqglo_doCompression = false;
  liqglo_doMotion = false;          // matrix motion blocks
  liqglo_doDef = false;             // geometry motion blocks
  liqglo_relativeMotion = false;
  liqglo_rotateCamera = false;      // rotate the camera 90 degrees around Z axis
  liqglo_doExtensionPadding = false;// pad the frame number in the rib file names
  liqglo_doShadows = true;          // render shadows
  liqglo_shapeOnlyInShadowNames = false;
  liqglo_noSingleFrameShadows = false;
  liqglo_singleFrameShadowsOnly = false;
  liqglo_motionSamples = 2;
  liqglo_motionFactor = 1.0;
  liqglo_FPS = 24.0;
  liqglo_outPadding = 0;
  liqglo_shutterTime = 0.5;
  liqglo_shutterEfficiency = 1.0;
  liqglo_projectDir = m_systemTempDirectory;
  liqglo_expandShaderArrays = false;
  liqglo_useMtorSubdiv = false;
  liqglo_outputMayaPolyCreases = false;
  liqglo_renderAllCurves = false;
  liqglo_hider = htHidden;

  liqglo_ribDir = "rib";
  liqglo_textureDir = "rmantex";
  
  liqglo_shaderPath = "&:@:.:~:rmanshader";
  liqglo_texturePath = "&:@:.:~:" + liqglo_textureDir;
  liqglo_archivePath = "&:@:.:~:" + liqglo_ribDir;
  liqglo_proceduralPath = "&:@:.:~";
  
  liqglo_exportAllShadersParams = false;
	liqglo_skipDefaultMatte = false;
  
  liqglo_renderCamera.clear();
  liqglo_shotName.clear();
  liqglo_shotVersion.clear();
  liqglo_layer.clear();
  liqglo_preReadArchive.clear();
  liqglo_preRibBox.clear(); 
  
  
  outFormat = "it";
  outExt = "tif";

  width = 360;
  height = 243;
  aspectRatio = 1.0;
  
  ignoreFilmGate = true;
  cleanShadows = 0;                 // render shadows
  cleanTextures = 0;                // render shadows
  pixelSamples = 1;
  shadingRate = 1.0;
  depth = 1;
  doDof = false;
  doCameraMotion = false;           // camera motion blocks
  launchRender = false;
  useNetRman = false;
  remoteRender = false;
  useRenderScript = true;
  cleanRib = false;
  cleanRenderScript = false;
  createOutputDirectories = true;
  
#ifdef DEBUG
  debugMode = 1;
#else
  debugMode = 0;
#endif
 
  extension = "rib";
  bucketSize[0] = 16;
  bucketSize[1] = 16;
  gridSize = 256;
  textureMemory = 2048;
  eyeSplits = 10;
  othreshold[0] = 0.996; othreshold[1] = 0.996; othreshold[2] = 0.996;
  zthreshold[0] = 0.996; zthreshold[1] = 0.996; zthreshold[2] = 0.996;
  
  shutterConfig = OPEN_ON_FRAME;
  
  fullShadowRib = false;
  baseShadowName = "";
  baseSceneName = "";
  quantValue = 8;
  
  originalLayer.clear();  
  frameNumbers.clear();

  // raytracing
  rt_useRayTracing = false;
  rt_traceBreadthFactor = 1.0;
  rt_traceDepthFactor = 1.0;
  rt_traceMaxDepth = 10;
  rt_traceSpecularThreshold = 10.0;
  rt_traceRayContinuation = 1;
  rt_traceCacheMemory = 30720;
  rt_traceDisplacements = false;
  rt_traceSampleMotion = false;
  rt_traceBias = 0.05;
  rt_traceMaxDiffuseDepth = 2;
  rt_traceMaxSpecularDepth = 2;

  rt_irradianceGlobalHandle="";
  rt_irradianceGlobalFileMode=0;

  rt_photonGlobalHandle = "";
  rt_causticGlobalHandle = "";
  rt_photonShadingModel = liqRibNode::photon::SHADINGMODEL_MATTE;
  rt_photonEstimator = 0;

  MString tmphome( getenv( "LIQUIDHOME" ) );
  tmphome = liquidSanitizeSearchPath( tmphome );

  if ( tmphome != "" ) 
  {
    liqglo_shaderPath += ":" + tmphome + "/lib/shaders";
    liqglo_texturePath += ":" + tmphome + "/lib/textures";
    liqglo_archivePath += ":" + tmphome + "/lib/rib";
  }
  
  liqglo_shaderPath += ":" + tmphome + "/lib/shaders";
  liqglo_texturePath += ":" + tmphome + "/lib/textures";
  liqglo_archivePath += ":" + tmphome + "/lib/rib";
}
/**
 * Class destructor
 */
liqRibTranslator::~liqRibTranslator()
{
}
/**
 * Error handling function.
 * This gets called when the RIB library detects an error.
 */
#if defined( DELIGHT ) || defined( ENTROPY ) ||  defined( PRMAN ) || defined( AIR ) || defined( PIXIE ) || defined( GENERIC_RIBLIB ) 
void liqRibTranslatorErrorHandler( RtInt code, RtInt severity, char* message )
#else
void liqRibTranslatorErrorHandler( RtInt code, RtInt severity, const char* message )
#endif
{
  printf( "The renderman library is reporting and error! Code: %d  Severity: %d", code, severity );
  MString error( message );
  throw error;
}

/**
 *
 */
MString liqRibTranslator::verifyResourceDir( const char *resourceName, MString resourceDir, bool &problem )
{
#ifdef _WIN32
  int dirMode = 6; // dummy arg
  int mkdirMode = 0;
  _chdir( liqglo_projectDir.asChar() );
#else
  mode_t dirMode,mkdirMode;
  dirMode = R_OK|W_OK|X_OK|F_OK;
  mkdirMode = S_IRWXU|S_IRWXG|S_IRWXO;
  chdir( liqglo_projectDir.asChar() );
#endif

#ifndef DIR_CREATION_WARNING
  #define DIR_CREATION_WARNING(type, path) \
  liquidMessage( "Had trouble creating " + string( type ) + " directory, '" + path + "'. Defaulting to system temp directory!", messageWarning );
#endif
#ifndef DIR_MISSING_WARNING
  #define DIR_MISSING_WARNING(type, path) \
  liquidMessage( string( type ) + " directory, '" + path + "', does not exist or is not accessible. Defaulting to system temp directory!", messageWarning );
#endif  

  LIQ_ADD_SLASH_IF_NEEDED( resourceDir );
  MString tmp_path( liquidSanitizePath( liquidGetRelativePath( liqglo_relativeFileNames, resourceDir, liqglo_projectDir ) ) );
  if ( !fileFullyAccessible( tmp_path ) ) 
  {
    if ( createOutputDirectories ) 
    {
      if ( !makeFullPath( tmp_path.asChar(), mkdirMode ) ) 
      {
        DIR_CREATION_WARNING( resourceName, tmp_path.asChar() );
        resourceDir = m_systemTempDirectory;
        problem = true;
      }
    } 
    else 
    {
      DIR_MISSING_WARNING( resourceName, tmp_path.asChar() );
      resourceDir = m_systemTempDirectory;
      problem = true;
    }
  } 
  else 
    resourceDir = tmp_path;  
  return resourceDir;
}
/**
 * 
 */
bool liqRibTranslator::verifyOutputDirectories()
{
  bool problem( false );
  
  liqglo_ribDir = verifyResourceDir( "RIB", liqglo_ribDir, problem );
  liqglo_textureDir = verifyResourceDir( "Texture", liqglo_textureDir, problem );
  m_pixDir = verifyResourceDir( "Picture", m_pixDir, problem );
  m_tmpDir = verifyResourceDir( "Temp Files", m_tmpDir, problem );

  return problem;
}
/**
 * Read the values from the render globals and set internal values.
 */
MString liqRibTranslator::generateRenderScriptName() const
{
  MString renderScriptName;
  renderScriptName = m_tmpDir;
  
  if ( m_userRenderScriptFileName != MString( "" ) )
    renderScriptName += m_userRenderScriptFileName;
  else 
  {
    renderScriptName += liqglo_sceneName;
    size_t tempSize = 0;
    char currentHostName[1024];
    short alfRand;
    gethostname( currentHostName, tempSize );
    liquidlong hashVal = liquidHash( currentHostName );
    
#ifndef _WIN32
    struct timeval  t_time;
    struct timezone t_zone;
    gettimeofday( &t_time, &t_zone );
    srandom( t_time.tv_usec + hashVal );
    alfRand = random();
#else
    struct tm *time;
    __time64_t long_time;
    _time64( &long_time );
    time = _localtime64( &long_time ); 
    srand( time->tm_sec + hashVal );
    alfRand = rand();
#endif
    renderScriptName += alfRand;
  }
  
  if ( m_renderScriptFormat == ALFRED ) renderScriptName += ".alf";
  if ( m_renderScriptFormat == XML ) renderScriptName += ".xml";
  
  return renderScriptName;
}
/**
 *
 */
MString liqRibTranslator::generateTempMayaSceneName() const
{
  MString tempDefname = m_tmpDir;
  tempDefname += liqglo_sceneName;
  size_t tempSize = 0;
  char currentHostName[1024];
  short defRand;
  
  gethostname( currentHostName, tempSize );
  liquidlong hashVal = liquidHash( currentHostName );
  
#ifndef _WIN32
  struct timeval  t_time;
  struct timezone t_zone;
  gettimeofday( &t_time, &t_zone );
  srandom( t_time.tv_usec + hashVal );
  defRand = random();
#else
  struct tm *time;
  __time64_t long_time;
  _time64( &long_time );
  time = _localtime64( &long_time ); 
  srand( time->tm_sec + hashVal );
  defRand = rand();
#endif
  
  tempDefname += defRand;
  
  MString currentFileType = MFileIO::fileType();
  if ( MString( "mayaAscii" ) == currentFileType ) tempDefname += ".ma";
  if ( MString( "mayaBinary" ) == currentFileType ) tempDefname += ".mb";
  
  return tempDefname;
}
/**
 * 
 */
string liqRibTranslator::generateImageName( MString aovName, const structJob& job, MString format )
{
  stringstream ss;
  // MString pixDir = liquidGetRelativePath ( false, m_pixDir, liqglo_projectDir ); // use full name instead of relative
  MString pixDir = getFullPathFromRelative ( m_pixDir);
  ss << pixDir.asChar();
  // cerr << "liqRibTranslator::generateImageName m_pixDir = " << m_pixDir.asChar() << " liqglo_projectDir = " << liqglo_projectDir.asChar() <<  endl;
  // cerr << "liqRibTranslator::generateImageName liquidGetRelativePath = " << ss.str() <<  endl;
  string ext( outExt.asChar() );
  
  if ( format != "" )
  {
    if ( format == "openexr" || format == "exr") 
      ext = "exr"; 
    else if ( format == "jpeg" ) 
      ext = "jpg"; 
    else if ( format == "tga" ) 
      ext = "tga"; 
    else if ( format == "sgi" ) 
      ext = "sgi"; 
    else        
      ext = "tif"; 
  }
  
  if ( liqglo_DDimageName == "" )
    ss << parseString( liqglo_sceneName, false ).asChar();
  else
    ss << parseString( liqglo_DDimageName, false ).asChar();
  
  // TODO: add new global var liqglo_displayHasCameraName
  if ( liqglo_beautyRibHasCameraName )
	  ss << "." << sanitizeNodeName( job.name ).asChar();
  
  if ( aovName != "" )
    ss << "." << aovName.asChar();  
	
	ss << "." << setfill('0') << setw( (liqglo_doExtensionPadding)? liqglo_outPadding : 0 ) <<  job.renderFrame;

  ss << "." << ext ;
  
  // cerr << "liqRibTranslator::generateImageName( " << aovName.asChar() << " ) -> " << format.asChar() << " ext = " << ext.c_str() << endl;

	return liquidSanitizePath( ss.str() );  	
}
/**
 * 
 */
MString liqRibTranslator::generateFileName( fileGenMode mode, const structJob& job )
{
	MString filename;
  stringstream ss;
 
  MString debug, suffix, fileExt;
	MString geometrySet = sanitizeNodeName( job.shadowObjectSet.substring(0, 99));
	switch ( mode )
	{
	  case fgm_shadow_tex:  
	    ss << liqglo_textureDir.asChar(); 
	    break;
	  
	  case fgm_shadow_rib:
	  case fgm_shadow_archive:
	  case fgm_beauty_rib:
	    ss << liqglo_ribDir.asChar(); 
	    break;
	    
	  case fgm_image:
	    // ss << m_pixDir.asChar(); 
	    filename = generateImageName( "", job, job.format ).c_str();
	    return filename;
	    break;
	}
	
	switch ( mode )
	{
	  case fgm_shadow_tex:  
	  case fgm_shadow_rib:
	  case fgm_shadow_archive:
	    if ( !liqglo_shapeOnlyInShadowNames )
        ss << liqglo_sceneName.asChar() << "_"; 
      break;
      
    case fgm_beauty_rib:
      ss << liqglo_sceneName.asChar(); 
      if ( liqglo_beautyRibHasCameraName )
			  ss << "_" << sanitizeNodeName( job.name ).asChar();
      break;
	}
	
	switch ( mode )
	{
	  case fgm_shadow_tex:  
	  case fgm_shadow_rib:
	    suffix = "_";
	    suffix += ( job.deepShadows ? "DSH" : "SHD");
      if ( job.isPoint && ( job.deepShadows || !job.shadowAggregation ) )
      {
        switch ( job.pointDir )
        {
          case pPX: suffix += "_PX"; break;
          case pPY: suffix += "_PY"; break;
          case pPZ: suffix += "_PZ"; break;
          case pNX: suffix += "_NX"; break;
          case pNY: suffix += "_NY"; break;
          case pNZ: suffix += "_NZ"; break;
        }
      } 
      break;
      
	  case fgm_shadow_archive:
	    suffix = "SHADOWBODY";
	    break;
	  case fgm_scene_archive:
	    suffix = "SCENE";
	    break;
	}
	
	switch ( mode )
	{
		case fgm_shadow_tex:
      debug = "fgm_shadow_tex";
      
      // check if aggregate is on
      if ( job.texName != "" )
        ss << sanitizeNodeName( job.texName ).asChar();
      else
        ss << sanitizeNodeName( job.name ).asChar();
  
      ss << suffix.asChar();
      
      // only if aggregate is off
      if ( job.shadowObjectSet != "" && job.texName == "" )
        ss << "." << geometrySet.asChar();
      fileExt = liquidRenderer.textureExtension; // ".tex";
      break;

		case fgm_shadow_rib:
			debug = "fgm_shadow_rib";
			ss << sanitizeNodeName( job.name ).asChar();
			ss << suffix.asChar();
			
			if ( job.shadowObjectSet != "" ) 
			  ss << "." << geometrySet.asChar();
			fileExt = extension;
			break;
		
		case fgm_shadow_archive:
		  debug = "fgm_shadow_archive"; 
			ss << suffix.asChar();

      if ( job.shadowObjectSet != "" ) 
			  ss << "." << geometrySet.asChar();
		  
			fileExt = extension;
			break;
		
	  case fgm_scene_archive:
			debug = "fgm_scene_archive";
			ss << suffix.asChar();
			fileExt = extension;
			break;
			
		case fgm_beauty_rib:
			debug = "fgm_beauty_rib";
			fileExt = extension;
			break;

		case fgm_image:
			debug = "fgm_image";
			if ( liqglo_DDimageName == "" )
			  ss << parseString( liqglo_sceneName, false ).asChar();
			else
			  ss << parseString( liqglo_DDimageName, false ).asChar();
			fileExt = outExt;
			break;

		default:
			liquidMessage( "liqRibTranslator::generateFileName: unknown case", messageError );
	}
 //  prepare format string "%0*d" ( boost format doesn't support * modificator )...
 if ( ( m_animation || m_useFrameExt )  ) // &&  mode != fgm_image
   ss << "." << setfill('0') << setw( (liqglo_doExtensionPadding)? liqglo_outPadding : 0 ) <<  job.renderFrame;

  ss << "." << fileExt.asChar() ;
  
  filename = liquidSanitizePath ( ss.str() ).c_str();
  // filename = liquidGetRelativePath ( false, filename, liqglo_projectDir );
  filename = getFullPathFromRelative ( filename );
  // cerr << "liqRibTranslator::generateFileName( " << debug.asChar() << " ) -> " << filename.asChar() << endl;
  LIQDEBUGPRINTF( "liqRibTranslator::generateFileName(%s) -> %s\n", debug.asChar(), filename.asChar() );
  return filename;
}
/**
 * This method setups render layer and save originalLayer
 */
MStatus liqRibTranslator::setRenderLayer( const MArgList& args )
{
  MStatus status = MS::kSuccess;
  // check if we need to switch to a specific render layer
  // we do that here because we need to switch to the chosen layer first
  // to be able to read overriden gloabsl and stuff...
  unsigned int argIndex = args.flagIndex( "lyr", "layer" );
  
  if ( argIndex != MArgList::kInvalidArgIndex ) liqglo_layer = args.asString( argIndex + 1, &status );

  // get the name of the current render layer
  
  if ( MGlobal::executeCommand( "editRenderLayerGlobals -q -currentRenderLayer;", originalLayer, false, false ) == MS::kFailure ) 
  {
    liquidMessage( "Could not get the current render layer name! ABORTING.", messageError );
    return MS::kFailure;
  }

  // switch to the specified render layer
  if ( liqglo_layer != "" ) 
  {
    MString cmd = "if( `editRenderLayerGlobals -q -currentRenderLayer` != \"" + liqglo_layer + "\" ) ";
    cmd += "editRenderLayerGlobals( \"-currentRenderLayer\", \"" + liqglo_layer + "\");";
    if (  MGlobal::executeCommand( cmd, false, false ) == MS::kFailure ) 
    {
      liquidMessage( "Could not switch to render layer '" + string( liqglo_layer.asChar() ) + "'! ABORTING.", messageError );
      return MS::kFailure;
    }
  } 
  else 
  {
    // we fill liqglo_layer with current layer name
    // to be able to substitute $LYR in strings.
    liqglo_layer = originalLayer;
  }
  return status;
}
/**
 * Process RIB output
 */
MStatus liqRibTranslator::ribOutput( long scanTime, MString ribName, bool world_only, bool out_lightBlock, MString archiveName )
{
  MStatus status = MS::kSuccess;
  
  // Rib client file creation options MUST be set before RiBegin
  LIQDEBUGPRINTF( "-> setting RiOptions\n" );
 
#if defined(PRMAN) || defined(DELIGHT)
  /* THERE IS A RIBLIB BUG WHICH PREVENTS THIS WORKING */

  RtString format[ 2 ] = { "ascii", "binary" };
  if ( liqglo_doBinary )
  {
    LIQDEBUGPRINTF( "-> setting binary option\n" );
    RiOption( "rib", "format", ( RtPointer )&format[1], RI_NULL );
  }
  else
  {
    LIQDEBUGPRINTF( "-> setting ascii option\n" );
    RiOption( "rib", "format", ( RtPointer )&format[0], RI_NULL );
    RtString style = "indented";
    RiOption( "rib", "string asciistyle", &style, RI_NULL );
  }
#endif // PRMAN || DELIGHT
#if defined(PRMAN) || defined(DELIGHT) || defined(GENERIC_RIBLIB)
  LIQDEBUGPRINTF( "-> setting compression option\n" );
  if ( liqglo_doCompression ) 
  {
    RtString comp[ 1 ] = { "gzip" };
    RiOption( "rib", "compression", ( RtPointer )comp, RI_NULL );
  }
#endif // PRMAN || DELIGHT || GENERIC_RIBLIB
  liquidMessage( "Beginning RIB output to " + string( ribName.asChar() ), messageInfo );
#ifndef RENDER_PIPE
  RiBegin( const_cast< RtToken >( ribName.asChar() ) );
#else
  liqglo_ribFP = fopen( ribName.asChar(), "w" );
  if ( liqglo_ribFP ) 
  {
    LIQDEBUGPRINTF( "-> setting pipe option\n" );
    RtInt ribFD( fileno( liqglo_ribFP ) );
    RiOption( "rib", "pipe", &ribFD, RI_NULL );
  }
  else
    liquidMessage( "Error opening RIB -- writing to stdout.", messageError );

  liquidMessage( "Beginning RI output directly to renderer", messageInfo );
  
  RiBegin( RI_NULL );
#endif
  //cerr << ">> writng RIB " << ribName.asChar() << endl;
  if ( !world_only )
  {
    //cerr << "writng ribPrologue()" << endl;
    if ( ribPrologue() != MS::kSuccess ) return MS::kFailure;
    //cerr << "writng framePrologue()" << endl;
    if ( framePrologue( scanTime ) != MS::kSuccess ) return MS::kFailure;
  }    
  if ( archiveName != "" )
  {
    // reference the correct shadow/scene archive
    //
    liquidMessage( "Writng archiveName " + string( archiveName.asChar() ), messageInfo ); 
    RiArchiveRecord( RI_COMMENT, "Read Archive Data:\n" );
    RiReadArchive( const_cast< RtToken >( archiveName.asChar() ), NULL, RI_NULL );
  }
  else
  {
    // full beauty/shadow rib generation
    //
    //cerr << "writng worldPrologue()" << endl;
    if ( worldPrologue() != MS::kSuccess ) return MS::kFailure;
    //cerr << "writng lightBlock()" << endl;
    if ( out_lightBlock && lightBlock() != MS::kSuccess ) return MS::kFailure;
    //cerr << "writng coordSysBlock()" << endl;    
    if ( coordSysBlock() != MS::kSuccess ) return MS::kFailure;
    //cerr << "writng objectBlock()" << endl;     
    if ( objectBlock() != MS::kSuccess ) return MS::kFailure;
    //cerr << "writng worldEpilogue()" << endl;      
    if ( worldEpilogue() != MS::kSuccess ) return MS::kFailure;
  }
  
  if ( !world_only )
  {
    //cerr << "writng frameEpilogue()" << endl;
    if ( frameEpilogue( scanTime ) != MS::kSuccess ) return MS::kFailure;  
    //cerr << "writng ribEpilogue()" << endl;
    if ( ribEpilogue() != MS::kSuccess ) return MS::kFailure;
  }
  RiEnd();
  // output info when done with the rib - Alf
  liquidMessage( "Finished RIB generation " + string( ribName.asChar()), messageInfo ); 
  
#ifdef RENDER_PIPE  
  fclose( liqglo_ribFP );
#endif
  liqglo_ribFP = NULL;
  return status;
}
/**
 * This method actually does the renderman output.
 */
MStatus liqRibTranslator::doIt( const MArgList& args )
{
  MStatus status;
  MString lastRibName;
  bool hashTableInited = false;

  status = setRenderLayer( args );
  if ( status != MS::kSuccess ) return MS::kFailure;
  
  liquidRenderer.setRenderer();
  m_renderCommand = liquidRenderer.renderCommand;

  status = liquidDoArgs( args );
  if ( status != MS::kSuccess ) return MS::kFailure;

  if ( !liquidBin && !m_deferredGen ) liquidMessage( "Creating RIB <Press ESC To Cancel> ...", messageInfo );

  // Remember the frame the scene was at so we can restore it later.
  MTime originalTime = MAnimControl::currentTime();

  // Set the frames-per-second global (we'll need this for
  // streak particles)
  //
  MTime oneSecond( 1, MTime::kSeconds );
  liqglo_FPS = oneSecond.as( MTime::uiUnit() );

// check to see if the output camera, if specified, is available. If exporting only objects, don't care about camera
	if ( !m_exportOnlyObjectBlock )
	{
		MStatus camStatus;
  	// check to see if the output camera, if specified, is available
  	if ( liquidBin && ( liqglo_renderCamera == "" ) ) 
  	{
    	liquidMessage( "No render camera specified!", messageError );
    	return MS::kFailure;
  	}
  
  	if ( liqglo_renderCamera != "" ) 
  	{
    	MStatus selectionStatus;
    	MSelectionList camList;
    	selectionStatus = camList.add( liqglo_renderCamera );
    	if ( selectionStatus != MS::kSuccess ) 
    	{
      	liquidMessage( "Invalid render camera!", messageError );
      	return MS::kFailure;
    	}
			camList.getDagPath(0, m_camDagPath);
  	}
		else
			m_activeView.getCamera( m_camDagPath );
		// check stereo camera
		MFnCamera fnCamera( m_camDagPath, &camStatus );
		if ( camStatus != MS::kSuccess )
		{
			liquidMessage( "Cannot create FN for render camera!", messageError );
			return MS::kFailure;
		}
		MString camType = fnCamera.typeName();
		if ( camType == "stereoRigCamera" ) m_isStereoCamera = true;
	}
	else
	{
		liqglo_renderCamera = "";
		liqglo_beautyRibHasCameraName = 0;
	}

  // check to see if all the directories we are working with actually exist.
  /*if( verifyOutputDirectories() ) {
    MString err( "The output directories are not properly setup in the globals" );
    throw err;
  }*/
  // This is bollocks! Liquid defaults to system temp folders if it can't setup shit. It should always work, not breaks
  verifyOutputDirectories();

  // setup the error handler
#if( !defined (GENERIC_RIBLIB) ) && ( defined ( AQSIS ) || ( _WIN32 && DELIGHT ) )
#  ifdef _WIN32
  if ( m_errorMode ) RiErrorHandler( (void(__cdecl*)(int,int,char*))liqRibTranslatorErrorHandler );
#  else
  if ( m_errorMode ) RiErrorHandler( (void(*)(int,int,char*))liqRibTranslatorErrorHandler );
#  endif
#else
  if ( m_errorMode ) RiErrorHandler( liqRibTranslatorErrorHandler );
#endif
  
  // Setup helper variables for alfred
  MString alfredCleanUpCommand = ( remoteRender ) ? MString( "RemoteCmd" ) : MString( "Cmd" );
  MString alfredRemoteTagsAndServices;
  MString alfredCleanupRemoteTagsAndServices;
  
  if ( remoteRender || useNetRman ) 
  {
    alfredRemoteTagsAndServices  = MString( "-service { " );
    alfredRemoteTagsAndServices += m_alfredServices.asChar();
    alfredRemoteTagsAndServices += MString( " } -tags { " );
    alfredRemoteTagsAndServices += m_alfredTags.asChar();
    alfredRemoteTagsAndServices += MString( " } " );
    
    // A seperate one for cleanup as it doesn't need a tag!
    alfredCleanupRemoteTagsAndServices  = MString( "-service { " );
    alfredCleanupRemoteTagsAndServices += m_alfredServices.asChar();
    alfredCleanupRemoteTagsAndServices += MString( " } " );
  }
  
  // exception handling block, this tracks liquid for any possible errors and tries to catch them
  // to avoid crashing
  try 
  {
    m_escHandler.beginComputation();

    MString preFrameMel = parseString( m_preFrameMel );
    MString postFrameMel = parseString( m_postFrameMel );
   
    if ( ( preFrameMel  != "" ) && !fileExists( preFrameMel ) ) 
      liquidMessage( "Cannot find pre frame MEL script file! Assuming local.", messageWarning );
    
    if ( ( m_postFrameMel != "" ) && !fileExists( postFrameMel ) ) 
      liquidMessage( "Cannot find post frame MEL script file! Assuming local.", messageWarning );

    // build temp file names
    MString renderScriptName = generateRenderScriptName();
    MString tempDefname = generateTempMayaSceneName();

    if ( m_deferredGen ) 
    {
      MString currentFileType = MFileIO::fileType();
      MFileIO::exportAll( tempDefname, currentFileType.asChar() );
    }

    if ( ( !m_deferredGen && m_justRib ) || m_exportReadArchive ) useRenderScript = false;

    liqRenderScript jobScript;
    liqRenderScript::Job preJobInstance;
    preJobInstance.title = "liquid pre-job";
    preJobInstance.isInstance = true;

    if ( useRenderScript ) 
    {
      // append the progress flag for render job feedback
      /*
      if (( m_renderCommand == MString( "render" ) ) || 
          ( m_renderCommand == MString( "prman" ) ) || 
          ( m_renderCommand == MString( "renderdl" ) ) ) 
        m_renderCommand = m_renderCommand + " -Progress";
      */
      
      if ( renderJobName == "" ) renderJobName = liqglo_sceneName;
      
      jobScript.title = renderJobName.asChar();

      if ( useNetRman ) 
      {
        jobScript.minServers = m_minCPU;
        jobScript.maxServers = m_maxCPU;
      } 
      else 
      {
        jobScript.minServers = 1;
        jobScript.maxServers = 1;
      }
      
      if ( m_dirmaps.length() ) jobScript.dirmaps = m_dirmaps.asChar();
      
      if ( m_preJobCommand != "" ) 
      {
        liqRenderScript::Job preJob;
        preJob.title = "liquid pre-job";
        liqRenderScript::Cmd jobCommand( m_preJobCommand.asChar(), ( remoteRender && !useNetRman ) );
        jobCommand.alfredServices = m_alfredServices.asChar();
		    jobCommand.alfredTags = m_alfredTags.asChar();  
		    preJob.commands.push_back( jobCommand );
        jobScript.addJob( preJob );
      }
    }
    // build the frame array
    //
    if ( m_renderView ) 
    {
      // if we are in renderView mode,
      // just ignore the animation range
      // and render the current frame.
      frameNumbers.clear();
      frameNumbers.push_back( ( int ) originalTime.as( MTime::uiUnit() ) );
    }
    //
    // start looping through the frames  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //
    liquidMessage( "Starting to loop through frames", messageInfo );

    int currentBlock( 0 );
    unsigned frameIndex( 0 );
    
    for ( ; frameIndex < frameNumbers.size(); frameIndex++ ) 
    {
      liqShaderFactory::instance().clearShaders();
        
      liqglo_lframe = frameNumbers[ frameIndex ];

      if ( m_showProgress ) printProgress( 1, frameNumbers.size(), frameIndex );

      liqRenderScript::Job frameScriptJob;

      m_alfShadowRibGen = false;
      liqglo_preReadArchive.clear();
      liqglo_preRibBox.clear();
      liqglo_preReadArchiveShadow.clear();
      liqglo_preRibBoxShadow.clear();

      // make sure all the global strings are parsed for this frame
      MString frameRenderCommand    = parseString( m_renderCommand + " " + liquidRenderer.renderCmdFlags, false );
      MString frameRibgenCommand    = parseString( m_ribgenCommand, false );
      MString framePreCommand       = parseString( m_preCommand, false );
      MString framePreFrameCommand  = parseString( m_preFrameCommand, false );
      MString framePostFrameCommand = parseString( m_postFrameCommand, false );

      if ( useRenderScript ) 
      {
        if ( m_deferredGen ) 
        {
          liqRenderScript::Job deferredJob;
          
          if ( ( frameIndex % m_deferredBlockSize ) == 0 ) 
          {
      			MString frameRangePart;      
						if ( m_deferredBlockSize == 1 ) 
						{
							currentBlock = liqglo_lframe;
							frameRangePart = MString( "-t " ) + liqglo_lframe;
						}
            else
						{
               currentBlock++;
							// Add list of frames to process for this block
							unsigned lastGenFrame( ( frameIndex + m_deferredBlockSize ) < frameNumbers.size() ? frameIndex + m_deferredBlockSize : frameNumbers.size() );
							//frameRangePart = MString( "-sequence " ) + frameIndex + " " + lastGenFrame  + " " + "1";
							frameRangePart = MString( "-t " ) + liqglo_lframe;
							for( unsigned outputFrame( frameIndex + 1 ); outputFrame < lastGenFrame; outputFrame++ )
							{
									frameRangePart += "," + frameNumbers[ outputFrame ];
							}
            }

            stringstream ribGenExtras;
            // ribGenExtras << " -progress -noDef -nop -noalfred -projectDir " << liqglo_projectDir.asChar() << " -ribName " << liqglo_sceneName.asChar() << " -mf " << tempDefname.asChar() << " -t ";
            if ( debugMode ) ribGenExtras << " -debug";
            ribGenExtras << " -progress -noDef -ribGenOnly -noLaunchRender";
            ribGenExtras << " -projectDir \"" << liqglo_projectDir.asChar() << "\" -ribName \"" << liqglo_sceneName.asChar() << "\" -fl ";
            
            unsigned lastGenFrame( ( frameIndex + m_deferredBlockSize ) < frameNumbers.size() ? frameIndex + m_deferredBlockSize : frameNumbers.size() );

            for ( unsigned outputFrame( frameIndex ); outputFrame < lastGenFrame; outputFrame++ )
            {  
              ribGenExtras << frameNumbers[ outputFrame ];
              ribGenExtras << (( outputFrame != ( lastGenFrame - 1 ) )? "," : " ");
            }

            stringstream titleStream;
            titleStream << liqglo_sceneName.asChar() << ".FrameRIBGEN." << currentBlock;
            deferredJob.title = titleStream.str();
                        
            stringstream ss;
            ss << framePreCommand.asChar() << " " << frameRibgenCommand.asChar() << ribGenExtras.str() << " \"" << tempDefname.asChar() << "\" ";
            // ss << framePreCommand.asChar() << " " << frameRibgenCommand.asChar() << " -debug ";
            liqRenderScript::Cmd cmd( ss.str(), false ); // remoteRender
            cmd.alfredServices = m_defGenService.asChar();
            cmd.alfredTags     = m_defGenKey.asChar();
            if ( m_alfredExpand ) cmd.alfredExpand = true;
            
            deferredJob.commands.push_back( cmd );
            jobScript.addJob( deferredJob );
          }
        }
        if ( !m_justRib ) 
        {
          stringstream titleStream;
          titleStream << liqglo_sceneName.asChar() << ".Frame." << liqglo_lframe;
          frameScriptJob.title = titleStream.str();

          if ( m_deferredGen ) 
          {
            stringstream ss;
            ss << liqglo_sceneName.asChar() << ".FrameRIBGEN." << currentBlock;
            liqRenderScript::Job instanceJob;
            instanceJob.isInstance = true;
            instanceJob.title = ss.str();
            frameScriptJob.childJobs.push_back( instanceJob );
          }
        }
      }

      LIQDEBUGPRINTF( "-> building jobs\n" );
      if ( buildJobs() != MS::kSuccess ) // Hmmmmmm not really clean ....
      {  
				liquidMessage( "[liqRibTranslator::doIt] Error while buildJobs", messageError );
				break;
			}
	
      if ( !m_deferredGen ) 
      {
        if ( m_showProgress ) printProgress( 2, frameNumbers.size(), frameIndex );

        long lastScannedFrame = -100000;
        long scanTime = liqglo_lframe;
        hashTableInited = false;
        //
        // start iterating through the job list   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
        //
        if ( jobList.size() == 0 ) 
        {
          liquidMessage( "Nothing to render!", messageWarning );
          return MS::kSuccess;
        }
        vector< structJob >::iterator iter( jobList.begin() );
        for ( ; iter != jobList.end(); ++iter ) 
        {
          m_currentMatteMode = false;
          liqglo_currentJob = *iter;

          if ( liqglo_currentJob.skip ) continue;

          // set the scan time based on the job's render frame
          //
          scanTime = liqglo_currentJob.renderFrame;
          // if we changed the frame to calculate a shadow at a different time,
          // we need to rescan the scene, otherwise not.
          //
          if ( lastScannedFrame != scanTime ) 
          {
            LIQDEBUGPRINTF( "Scanning at time: %u \n", (unsigned int)scanTime );
            // hash table handling
            //
            /*if( hashTableInited && htable ) {
              //cout <<"delete old table... "<<flush;
              //delete htable;
              htable.reset();
              //freeShaders();
            }*/

            htable = boost::shared_ptr< liqRibHT >( new liqRibHT() );
            hashTableInited = true;
            LIQDEBUGPRINTF( "Created hash table...\n" );

            //  calculate sampling time
            //
            float sampleinc( ( liqglo_shutterTime * m_blurTime ) / ( liqglo_motionSamples - 1 ) );
            for ( unsigned msampleOn( 0 ); msampleOn < liqglo_motionSamples; msampleOn++ ) 
            {
              float subframe;
              switch ( shutterConfig ) 
              {
                case OPEN_ON_FRAME:
                default:
                  subframe = scanTime + ( msampleOn * sampleinc );
                  break;
                case CENTER_ON_FRAME:
                  subframe = ( scanTime - ( liqglo_shutterTime * m_blurTime * 0.5 ) ) + msampleOn * sampleinc;
                  break;
                case CENTER_BETWEEN_FRAMES:
                  subframe = scanTime + ( 0.5 * ( 1 - ( liqglo_shutterTime * m_blurTime ) ) ) + ( msampleOn * sampleinc );
                  break;
                case CLOSE_ON_NEXT_FRAME:
                  subframe = scanTime + ( 1 - ( liqglo_shutterTime * m_blurTime ) ) + ( msampleOn * sampleinc );
                  break;
              }
              liqglo_sampleTimes[ msampleOn ] = subframe;
              liqglo_sampleTimesOffsets[ msampleOn ] = msampleOn * sampleinc;
            }
            // scan the scene
            //
            if ( doCameraMotion || liqglo_doMotion || liqglo_doDef ) 
            {
              for ( int msampleOn = 0; msampleOn < liqglo_motionSamples; msampleOn++ ) 
                scanScene( liqglo_sampleTimes[ msampleOn ] , msampleOn );
            } 
            else 
            {
              liqglo_sampleTimes[ 0 ] = scanTime;
              liqglo_sampleTimesOffsets[ 0 ] = 0;
              scanScene( scanTime, 0 );
            }

            // mark the frame as already scanned
            lastScannedFrame = scanTime;
            liqglo_currentJob = *iter;
          }
          //
          // start scene parsing ------------------------------------------------------------------
          //
          liqglo_isShadowPass = liqglo_currentJob.isShadowPass; 

          // build the shadow archive name for the job
          baseShadowName = generateFileName( fgm_shadow_archive, liqglo_currentJob); 
          baseShadowName = liquidGetRelativePath( liqglo_relativeFileNames, baseShadowName, liqglo_ribDir );
          bool out_lightBlock = false;
          // world RiReadArchives and Rib Boxes ************************************************
          //
          if ( liqglo_currentJob.isShadow && !liqglo_currentJob.shadowArchiveRibDone && !fullShadowRib ) 
          {
            //  create the read-archive shadow files
            //  world_only = true
            //  out_lightBlock = (liqglo_currentJob.deepShadows && m_outputLightsInDeepShadows )
            //  archiveName = ""
            out_lightBlock = (liqglo_currentJob.deepShadows && m_outputLightsInDeepShadows );
            if ( ribOutput( scanTime, baseShadowName, true, out_lightBlock, MString("") ) != MS::kSuccess )
              break;

            // mark all other jobs with the same set as done
            vector<structJob>::iterator iterCheck = jobList.begin();
            while ( iterCheck != jobList.end() ) 
            {
              if ( iterCheck->shadowObjectSet == liqglo_currentJob.shadowObjectSet &&
                  iterCheck->everyFrame == liqglo_currentJob.everyFrame &&
                  iterCheck->renderFrame == liqglo_currentJob.renderFrame
                )
                iterCheck->shadowArchiveRibDone = true;
              ++iterCheck;
            }
            m_alfShadowRibGen = true;
          }
          //  create beauty/shadow rib files
          //  world_only = false
          MString archiveName = "";
          
          if ( liqglo_currentJob.isShadow && !fullShadowRib ) 
            archiveName = baseShadowName;
          
          out_lightBlock = (!liqglo_currentJob.isShadow || ( liqglo_currentJob.isShadow && liqglo_currentJob.deepShadows && m_outputLightsInDeepShadows) );
          
          if ( ribOutput( scanTime, liqglo_currentJob.ribFileName, false, out_lightBlock, archiveName ) != MS::kSuccess )
            break;
          
          if ( m_showProgress ) printProgress( 3, frameNumbers.size(), frameIndex );
        }
        // set the rib file for the 'view last rib' menu command
        // NOTE: this may be overridden later on in certain code paths
        lastRibName = liqglo_currentJob.ribFileName;
      }
      // now we re-iterate through the job list to write out the alfred file if we are using it
      if ( useRenderScript && !m_justRib ) 
      {
        bool alf_textures = false;
        bool alf_shadows = false;
        bool alf_refmaps = false;
        bool use_dirmaps = ( m_dirmaps.length() )? 1 : 0;
        
        // write out make texture pass
        LIQDEBUGPRINTF( "-> Generating job for MakeTexture pass\n");
        vector<structJob>::iterator iter = txtList.begin();
        if ( txtList.size() ) 
        {
          alf_textures = true;
          liqRenderScript::Job textureJob;
          stringstream ts;
          ts << "Textures." << liqglo_lframe;
          textureJob.title = ts.str();

          while ( iter != txtList.end() ) 
          {
            liqRenderScript::Job textureSubtask;
            stringstream ts;
            ts << textureJob.title << " " << iter->imageName.asChar();
            textureSubtask.title = ts.str();
            if ( m_deferredGen ) 
            {

            }
            stringstream ss;
            ss << iter->renderName.asChar() << " " << iter->ribFileName.asChar();
            liqRenderScript::Cmd cmd( ss.str(), ( remoteRender && !useNetRman ) );

            if( m_alfredExpand ) cmd.alfredExpand = true;
            
            cmd.alfredServices = m_alfredServices.asChar();
            cmd.alfredTags     = m_alfredTags.asChar();
            textureSubtask.commands.push_back( cmd );
            textureSubtask.chaserCommand = ( string( "sho \"" ) + liqglo_textureDir.asChar() + " " + iter->imageName.asChar() + "\"" );
            ++iter;
            textureJob.childJobs.push_back( textureSubtask );
          }
          frameScriptJob.childJobs.push_back( textureJob );
        }

        // write out shadows
        if ( liqglo_doShadows ) 
        {
          LIQDEBUGPRINTF( "-> writing out shadow information to alfred file.\n" );
          vector< structJob >::iterator iter = shadowList.begin();
          if ( shadowList.size() ) 
          {
            alf_shadows = true;
            liqRenderScript::Job shadowJob;
            stringstream ts;
            ts << "Shadows." << liqglo_lframe;
            shadowJob.title = ts.str();
            while ( iter != shadowList.end() ) 
            {
              alf_shadows = true;
              liqRenderScript::Job shadowSubtask;
              shadowSubtask.title = iter->name.asChar();
              
              if ( alf_textures ) 
              {
                stringstream ss;
                ss << "Textures." << liqglo_lframe;
                liqRenderScript::Job instanceJob;
                instanceJob.isInstance = true;
                instanceJob.title = ss.str();
                shadowSubtask.childJobs.push_back(instanceJob);
              }
              
              if ( m_deferredGen ) 
              {
                stringstream ss;
                ss << liqglo_sceneName.asChar() << ".FrameRIBGEN." << currentBlock;
                liqRenderScript::Job instanceJob;
                instanceJob.isInstance = true;
                instanceJob.title = ss.str();
                shadowSubtask.childJobs.push_back(instanceJob);
              }
              
              stringstream ss;
              
              if ( useNetRman ) ss << framePreCommand.asChar() << " netrender %H ";
              else ss << framePreCommand.asChar() << " " << frameRenderCommand.asChar() << " ";
              ss << "-Progress ";
              
              if ( use_dirmaps )  ss << "%D(" << iter->ribFileName.asChar() << ")";
              else
#ifdef _WIN32
                ss << "\"" << iter->ribFileName.asChar() << "\"";
#else
                ss << iter->ribFileName.asChar();
#endif
              liqRenderScript::Cmd cmd(ss.str(), (remoteRender && !useNetRman));
              
              if ( m_alfredExpand ) cmd.alfredExpand = true;
              
              cmd.alfredServices = m_alfredServices.asChar();
              cmd.alfredTags     = m_alfredTags.asChar();
              shadowSubtask.commands.push_back(cmd);

              if ( cleanRib )  
              {
                stringstream ss;
                
                ss << framePreCommand.asChar() << " " << RM_CMD << " ";
#ifdef _WIN32
                ss << "\"" << iter->ribFileName.asChar() << "\"";
#else
                ss << iter->ribFileName.asChar();
#endif
                liqRenderScript::Cmd jobShdCommand( ss.str(), remoteRender );
        				jobShdCommand.alfredServices = m_alfredServices.asChar();
        				jobShdCommand.alfredTags = m_alfredTags.asChar();
                shadowSubtask.cleanupCommands.push_back( jobShdCommand );
              }
              shadowSubtask.chaserCommand = ( string( "sho \"" ) + iter->imageName.asChar() + "\"" );
              ++iter;
              
              if ( !m_alfShadowRibGen && !fullShadowRib ) m_alfShadowRibGen = true;
              
              shadowJob.childJobs.push_back( shadowSubtask );
            }
            frameScriptJob.childJobs.push_back( shadowJob );
          }
        }
        LIQDEBUGPRINTF( "-> finished writing out shadow information to render script file.\n" );

        // write out make reflection pass
        if ( refList.size() ) 
        {
          LIQDEBUGPRINTF( "-> Generating job for ReflectionMap pass\n" );
          vector<structJob>::iterator iter = refList.begin();

          alf_refmaps = true;
          liqRenderScript::Job reflectJob;
          stringstream ts;
          ts << "Reflections." << liqglo_lframe;
          reflectJob.title = ts.str();

          while ( iter != refList.end() ) 
          {
            liqRenderScript::Job reflectSubtask;
            stringstream ts;
            ts << reflectJob.title << " " << iter->imageName.asChar();
            reflectSubtask.title = ts.str();
            if ( m_deferredGen ) 
						{

            }
            if ( alf_textures ) 
            {
              stringstream ss;
              ss << "Textures." << liqglo_lframe;
              liqRenderScript::Job instanceJob;
              instanceJob.isInstance = true;
              instanceJob.title = ss.str();
              reflectJob.childJobs.push_back( instanceJob );
            }
            if ( alf_shadows ) 
            {
              stringstream ss;
              ss << "Shadows." << liqglo_lframe;
              liqRenderScript::Job instanceJob;
              instanceJob.isInstance = true;
              instanceJob.title = ss.str();
              reflectJob.childJobs.push_back( instanceJob );
            }

            stringstream ss;
            ss << iter->renderName.asChar() << " " << iter->ribFileName.asChar();
            liqRenderScript::Cmd cmd( ss.str(), (remoteRender && !useNetRman) );

            if( m_alfredExpand ) cmd.alfredExpand = true;
            
            cmd.alfredServices = m_alfredServices.asChar();
            cmd.alfredTags     = m_alfredTags.asChar();
            reflectSubtask.commands.push_back( cmd );
            reflectSubtask.chaserCommand = ( string( "sho \"" ) + liqglo_textureDir.asChar() + " " + iter->imageName.asChar() + "\"" );
            ++iter;
            reflectJob.childJobs.push_back( reflectSubtask );
          }
          frameScriptJob.childJobs.push_back( reflectJob );
        }

        LIQDEBUGPRINTF( "-> initiating hero pass information.\n" );
        structJob *frameJob = NULL;
        structJob *shadowPassJob = NULL;
        
        LIQDEBUGPRINTF( "-> setting hero pass.\n" );
        if ( m_outputHeroPass && !m_outputShadowPass ) 
          frameJob = &jobList[jobList.size() - 1];
        else if ( m_outputShadowPass && m_outputHeroPass ) 
        {
          frameJob = &jobList[jobList.size() - 1];
          shadowPassJob = &jobList[jobList.size() - 2];
        } 
        else if ( m_outputShadowPass && !m_outputHeroPass ) 
          shadowPassJob = &jobList[jobList.size() - 1];
        
        LIQDEBUGPRINTF( "-> hero pass set.\n" );
        LIQDEBUGPRINTF( "-> writing out pre frame command information to render script file.\n" );
        if ( framePreFrameCommand != MString("") ) 
        {
          liqRenderScript::Cmd cmd(framePreFrameCommand.asChar(), (remoteRender && !useNetRman));
          cmd.alfredServices = m_alfredServices.asChar();
          cmd.alfredTags     = m_alfredTags.asChar();
          frameScriptJob.commands.push_back(cmd);
        }
        
        if ( m_outputHeroPass || m_outputShadowPass ) 
        {
          stringstream ss;
          string  ribFileName;
          
          if( m_outputHeroPass ) ribFileName = string( frameJob->ribFileName.asChar() );
          else                   ribFileName = string( shadowPassJob->ribFileName.asChar() );
          
          if ( useNetRman ) ss << framePreCommand.asChar() << " netrender %H ";
          else              ss << framePreCommand.asChar() << " " << frameRenderCommand.asChar();
          ss << " -Progress ";
          
          if ( use_dirmaps ) ss << "%D("  << ribFileName << ")";
          else
#ifdef _WIN32            
            ss << "\"" << ribFileName << "\"";
#else           
            ss << ribFileName;
#endif
          
          liqRenderScript::Cmd cmd(ss.str(), (remoteRender && !useNetRman));
          if ( m_alfredExpand ) cmd.alfredExpand = true;
          
          cmd.alfredServices = m_alfredServices.asChar();
          cmd.alfredTags     = m_alfredTags.asChar();
          frameScriptJob.commands.push_back(cmd);
        }
        LIQDEBUGPRINTF( "-> finished writing out hero information to alfred file.\n" );
        
        if ( cleanRib || ( framePostFrameCommand != MString( "" ) ) ) 
        {
          if ( cleanRib & ( m_outputHeroPass ||  m_outputShadowPass ||  m_alfShadowRibGen ) ) 
          {
            stringstream ss;
            string  ribFileName;
          
            if ( m_outputHeroPass )       ribFileName = string( frameJob->ribFileName.asChar() );
            else if ( m_outputShadowPass) ribFileName = string( shadowPassJob->ribFileName.asChar() );
            else                          ribFileName = string( baseShadowName.asChar() );
            
            ss << framePreCommand.asChar() << " " << RM_CMD << " ";
            if ( use_dirmaps ) ss << "%D(" << ribFileName << ")";
            else
#ifdef _WIN32
              ss << "\"" << ribFileName << "\"";
#else
              ss << ribFileName;
#endif
            // frameScriptJob.cleanupCommands.push_back(liqRenderScript::Cmd(ss.str(), remoteRender));
            liqRenderScript::Cmd jobCleanCommand( ss.str(), remoteRender );
      			jobCleanCommand.alfredServices = m_alfredServices.asChar();
      			jobCleanCommand.alfredTags = m_alfredTags.asChar();
      			frameScriptJob.cleanupCommands.push_back( jobCleanCommand );
          }
          
          if ( framePostFrameCommand != MString("") ) 
          {
            // liqRenderScript::Cmd cmd(framePostFrameCommand.asChar(), (remoteRender && !useNetRman));
            liqRenderScript::Cmd cmd( framePostFrameCommand.asChar(), (remoteRender && !useNetRman) );
      			cmd.alfredServices = m_alfredServices.asChar();
      			cmd.alfredTags = m_alfredTags.asChar();
            frameScriptJob.cleanupCommands.push_back(cmd);
          }
        }
        
        if ( m_outputHeroPass ) frameScriptJob.chaserCommand = (string( "sho \"" ) + frameJob->imageName.asChar() + "\"" );
        if ( m_outputShadowPass ) frameScriptJob.chaserCommand = (string( "sho \"" ) + shadowPassJob->imageName.asChar() + "\"" );
        if ( m_outputShadowPass && !m_outputHeroPass ) 
          lastRibName = liquidGetRelativePath( liqglo_relativeFileNames, shadowPassJob->ribFileName, liqglo_projectDir );
        else 
          lastRibName = liquidGetRelativePath( liqglo_relativeFileNames, frameJob->ribFileName, liqglo_projectDir );
      }
      
      jobScript.addJob( frameScriptJob );
      
      if( ( ribStatus != kRibOK ) && !m_deferredGen ) break;
    } // frame for-loop

    if ( useRenderScript ) 
    {
      if ( m_preJobCommand != MString( "" ) ) jobScript.addLeafDependency( preJobInstance );
      // clean up the alfred file in the future
      if ( !m_justRib ) 
      {
        if ( m_deferredGen )
        {
          string cmd = RM_CMD  + string( MString( " \""  + tempDefname + "\"" ).asChar() );
          liqRenderScript::Cmd jobCleanCmd( cmd, 0 );
    			jobCleanCmd.alfredServices = m_alfredServices.asChar();
    			jobCleanCmd.alfredTags = m_alfredTags.asChar();
    			jobScript.cleanupCommands.push_back( jobCleanCmd );
        }
        
        if ( cleanRenderScript )
        {
          string cmd = RM_CMD  + string( MString( " \""  + renderScriptName + "\"" ).asChar() );
          liqRenderScript::Cmd jobCleanCmd( cmd, 0 );
    			jobCleanCmd.alfredServices = m_alfredServices.asChar();
    			jobCleanCmd.alfredTags = m_alfredTags.asChar();
    			jobScript.cleanupCommands.push_back( jobCleanCmd );
        }

        if ( m_postJobCommand != MString("") )
        {
          string cmd = m_postJobCommand.asChar();
        
          liqRenderScript::Cmd jobCleanCmd( cmd, 0 );
    			jobCleanCmd.alfredServices = m_alfredServices.asChar();
    			jobCleanCmd.alfredTags = m_alfredTags.asChar();
    			jobScript.cleanupCommands.push_back( jobCleanCmd );
        }
      }
      
      if ( m_renderScriptFormat == ALFRED ) 
        jobScript.writeALF( liquidGetRelativePath( liqglo_relativeFileNames, renderScriptName, liqglo_projectDir ).asChar() );
      if ( m_renderScriptFormat == XML ) 
        jobScript.writeXML( liquidGetRelativePath( liqglo_relativeFileNames, renderScriptName, liqglo_projectDir ).asChar() );
    }
    
    LIQDEBUGPRINTF( "-> ending escape handler.\n" );
    m_escHandler.endComputation();

    if ( !m_deferredGen ) liquidMessage( "Finished creating RIB", messageInfo );
    
    LIQDEBUGPRINTF( "-> clearing job list.\n" );
    jobList.clear();
    jobScript.clear();

    // set the attributes on the liquidGlobals for the last rib file and last alfred script name
    LIQDEBUGPRINTF( "-> setting lastAlfredScript and lastRibFile.\n" );
    MGlobal::executeCommand("if(!attributeExists(\"lastRenderScript\",\"liquidGlobals\")) { addAttr -ln \"lastRenderScript\" -dt \"string\" liquidGlobals; }");
    MFnDependencyNode rGlobalNode( rGlobalObj );
    MPlug nPlug;
    nPlug = rGlobalNode.findPlug( "lastRenderScript" );
    nPlug.setValue( renderScriptName );
    nPlug = rGlobalNode.findPlug( "lastRibFile" );
    nPlug.setValue( lastRibName );
    LIQDEBUGPRINTF( "-> spawning command.\n" );
    if ( launchRender ) 
    {
      if ( useRenderScript ) 
      {
        bool wait = false;
        // mesh: This already cheched while reading globals
        //if ( m_renderScriptCommand == "" ) 
        //  m_renderScriptCommand = "alfred";
        if ( m_renderScriptFormat == NONE ) 
          liquidMessage( "No render script format specified to Liquid, and direct render execution not selected.", messageWarning );

        // mesh: this allows to debug output from custom renderScriptCommand
        if ( m_renderScriptCommand != "alfred" )
        {
          MString cmd = m_renderScriptCommand;
#ifndef _WIN32
          chdir( liqglo_projectDir.asChar() );
          cmd += " " + renderScriptName + " " + liqglo_projectDir + " " +( wait ? "" : "&" ); 
#else
          _chdir( liqglo_projectDir.asChar() );
          cmd += " \"" + renderScriptName + "\"" + " \"" + liqglo_projectDir + "\""; 
#endif          
          stringstream err;
          err << ">> render (" << ( (!wait)? "no " : "" ) << "wait) "<< cmd.asChar() << endl << ends;
          liquidMessage( err.str(), messageInfo );
          int returnCode = system( cmd.asChar() );
        }
        else
        {
          liqProcessLauncher::execute( m_renderScriptCommand,        
#ifdef _WIN32
          // Moritz: Added quotes to render script name as it may contain spaces in bloody Windoze
          // Note: also adding quotes to the path (aka project dir) breaks ShellExecute() -- cost me one hour to trace this!!!
          // Bloody, damn, asinine Windoze!!!
          "\"" + renderScriptName + "\"", 
          "\"" + liqglo_projectDir + "\"", 
#else
          renderScriptName,
          liqglo_projectDir, 
#endif
          wait );
        }
      } 
      else 
      {
        // launch renders directly
        liquidMessage( string(), messageInfo ); // emit a '\n'
        // int exitstat = 0; ???
        
        //
        // write out make texture pass
        //
        vector<structJob>::iterator iter = txtList.begin();
        while ( iter != txtList.end() ) 
        {
          liquidMessage( "Making textures '" + string( iter->imageName.asChar() ) + "'", messageInfo );
          liqProcessLauncher::execute( iter->renderName, 
#ifdef _WIN32
          (" -progress \"" + iter->ribFileName + "\""), 
#else
          (" -progress " + iter->ribFileName), 
#endif
          liqglo_projectDir, true );
          
          ++iter;
        }
        //
        // write out shadows
        //
        if ( liqglo_doShadows ) 
        {
          liquidMessage( "Rendering shadow maps... ", messageInfo );
          vector<structJob>::iterator iter = shadowList.begin();
          
          while ( iter != shadowList.end() ) 
          {
            if ( iter->skip ) 
            {
              liquidMessage( "    - skipping '" + string( iter->ribFileName.asChar() ) + "'", messageInfo );
              ++iter;
              continue;
            }
            liquidMessage( "    + '" + string( iter->ribFileName.asChar() ) + "'", messageInfo );
            
            if ( !liqProcessLauncher::execute( liquidRenderer.renderCommand, liquidRenderer.renderCmdFlags + " " +
#ifdef _WIN32
            "\"" + iter->ribFileName + "\"", 
#else
            iter->ribFileName, 
#endif
            liqglo_projectDir, true ) )  break;
            ++iter;
          } // while ( iter != shadowList.end() )
        }
        //
        // write out hero pass
        //
        liquidMessage( "Rendering hero pass... ", messageInfo );
        cerr << "liquidBin = " << liquidBin << endl << flush; 
        
        if ( liqglo_currentJob.skip ) 
          liquidMessage( "    - skipping '" + string( liqglo_currentJob.ribFileName.asChar() ) + "'", messageInfo );
        else 
        {
          liquidMessage( "    + '" + string( liqglo_currentJob.ribFileName.asChar() ) + "'", messageInfo );
          liqProcessLauncher::execute( liquidRenderer.renderCommand, liquidRenderer.renderCmdFlags + " " +
#ifdef _WIN32
          "\"" + liqglo_currentJob.ribFileName + "\"", "\"" + liqglo_projectDir + "\"",
#else
           liqglo_currentJob.ribFileName, liqglo_projectDir,
#endif
           false );
        }
      } // if ( useRenderScript ) 
      //
      //  philippe: here we launch the liquidRenderView command which will listen to the liqmaya display driver
      //  to display buckets in the renderview.
      if ( m_renderView ) 
      {
        stringstream displayCmd;
        displayCmd << "liquidRenderView -c " << liqglo_renderCamera.asChar();
        displayCmd << " -l " << ( ( m_renderViewLocal )? "1":"0" );
        displayCmd << " -port " << m_renderViewPort;
        displayCmd << " -timeout " <<  m_renderViewTimeOut;
        if ( m_renderViewCrop ) displayCmd << " -doRegion";
        
        displayCmd << ";liquidSaveRenderViewImage();";
        MGlobal::executeCommand( MString( displayCmd.str().c_str() ) );
      } 
    } // if( launchRender )

    // return to the frame we were at before we ran the animation
    LIQDEBUGPRINTF( "-> setting frame to current frame.\n" );
    MGlobal::viewFrame (originalTime);

    if ( originalLayer != "" ) 
    {
      MString cmd;
      cmd = "if( `editRenderLayerGlobals -q -currentRenderLayer` != \"" + originalLayer + "\" ) editRenderLayerGlobals -currentRenderLayer \"" + originalLayer + "\";";
      if (  MGlobal::executeCommand( cmd, false, false ) == MS::kFailure ) 
      {
        MString err = "Liquid : could not switch back to render layer \"" + originalLayer + "\" !";
        throw err;
      }
    }
    return ( (ribStatus == kRibOK || m_deferredGen) ? MS::kSuccess : MS::kFailure);
  } 
  catch ( MString errorMessage ) 
  {
    liquidMessage( errorMessage.asChar(), messageError );
    /*if( htable && hashTableInited ) delete htable;
    freeShaders();*/
    m_escHandler.endComputation();
    return MS::kFailure;
  } 
  catch ( ... ) 
  {
    liquidMessage( "Unknown exception thrown", messageError );
    /*if( htable && hashTableInited ) delete htable;
    freeShaders();*/
    m_escHandler.endComputation();
    return MS::kFailure;
  }
}

/**
 * Calculate the port field of view for the camera.
 */
void liqRibTranslator::portFieldOfView( int port_width, int port_height,
                                        double& horizontal,
                                        double& vertical,
                                        MFnCamera& fnCamera )
{
  // note : works well - api tested
  double left, right, bottom, top;
  double aspect = (double) port_width / port_height;
  computeViewingFrustum(aspect,left,right,bottom,top,fnCamera);

  double neardb = fnCamera.nearClippingPlane();
  horizontal    = atan( ( ( right - left ) * 0.5 ) / neardb ) * 2.0;
  vertical      = atan( ( ( top - bottom ) * 0.5 ) / neardb ) * 2.0;
}

/**
 * Calculate the viewing frustrum for the camera.
 */
void liqRibTranslator::computeViewingFrustum ( double     window_aspect,
                                               double&    left,
                                               double&    right,
                                               double&    bottom,
                                               double&    top,
                                               MFnCamera& cam )
{
  double film_aspect   = cam.aspectRatio();
  double aperture_x    = cam.horizontalFilmAperture();
  double aperture_y    = cam.verticalFilmAperture();
  double offset_x      = cam.horizontalFilmOffset();
  double offset_y      = cam.verticalFilmOffset();
  double focal_to_near = cam.nearClippingPlane() / (cam.focalLength() * MM_TO_INCH);

  focal_to_near *= cam.cameraScale();

  double scale_x = 1.0;
  double scale_y = 1.0;
  double translate_x = 0.0;
  double translate_y = 0.0;

  switch ( cam.filmFit() ) 
  {
    case MFnCamera::kFillFilmFit:
      if ( window_aspect < film_aspect ) scale_x = window_aspect / film_aspect;
      else                               scale_y = film_aspect / window_aspect;
      break;
      
    case MFnCamera::kHorizontalFilmFit:
      scale_y = film_aspect / window_aspect;
      if ( scale_y > 1.0 ) translate_y = cam.filmFitOffset() * ( aperture_y - ( aperture_y * scale_y ) ) / 2.0;
      break;
      
    case MFnCamera::kVerticalFilmFit:
      scale_x = window_aspect / film_aspect;
      if ( scale_x > 1.0 ) translate_x = cam.filmFitOffset() * ( aperture_x - ( aperture_x * scale_x ) ) / 2.0;
      break;
      
    case MFnCamera::kOverscanFilmFit:
      if ( window_aspect < film_aspect ) scale_y = film_aspect / window_aspect;
      else                               scale_x = window_aspect / film_aspect;
      break;
      
    case MFnCamera::kInvalid:
      break;
  }

  left   = focal_to_near * (-.5 * aperture_x * scale_x + offset_x + translate_x );
  right  = focal_to_near * ( .5 * aperture_x * scale_x + offset_x + translate_x );
  bottom = focal_to_near * (-.5 * aperture_y * scale_y + offset_y + translate_y );
  top    = focal_to_near * ( .5 * aperture_y * scale_y + offset_y + translate_y );

  // NOTE :
  //      all the code above could be replaced by :
  //
  //      cam.getRenderingFrustum( window_aspect, left, right, bottom, top );
  //
  //      should we keep this for educationnal purposes or use the API call ??
}

void liqRibTranslator::exportJobCamera(const structJob &job, const structCamera camera[])
{
	if ( camera[0].isOrtho )
	{
		RtFloat frameWidth, frameHeight;
		// the whole frame width has to be scaled according to the UI Unit
		frameWidth  = camera[0].orthoWidth  * 0.5 ;
		frameHeight = camera[0].orthoHeight * 0.5 ;
		RiProjection( "orthographic", RI_NULL );
		// if we are describing a shadow map camera,
		// we need to set the screenwindow to the default,
		// as shadow maps are always square.
		if( job.isShadow == true ) RiScreenWindow( -frameWidth, frameWidth, -frameHeight, frameHeight );
		else 			                 RiScreenWindow( -1.0, 1.0, -1.0, 1.0 );
	}
	else
	{
		RtFloat fieldOfView = camera[0].hFOV * 180.0 / M_PI ;
		if ( job.isShadow && job.isPoint ) fieldOfView = job.camera[0].hFOV;
		
		RiProjection( "perspective", RI_FOV, &fieldOfView, RI_NULL );

		// if we are describing a shadow map camera,
		// we need to set the screenwindow to the default,
		// as shadow maps are always square.

		if ( job.isShadow == false )
		{
			double ratio = (double)job.width / (double)job.height;
			double left, right, bottom, top;
			if ( ratio <= 0 )
			{
				left    = -1 + camera[0].horizontalFilmOffset;
				right   =  1 + camera[0].horizontalFilmOffset;
				bottom  = -1 / ratio + camera[0].verticalFilmOffset;
				top     =  1 / ratio + camera[0].verticalFilmOffset;
			}
			else
			{
				left    = -ratio + camera[0].horizontalFilmOffset;
				right   =  ratio + camera[0].horizontalFilmOffset;
				bottom  = -1 + camera[0].verticalFilmOffset;
				top     =  1 + camera[0].verticalFilmOffset;
			}
			RiScreenWindow( left, right, bottom, top );
		}
		else
		{
			RiScreenWindow( -1.0, 1.0, -1.0, 1.0 );
		}
	}
	RiClipping( camera[0].neardb, camera[0].fardb );
	if ( doDof && !job.isShadow ) RiDepthOfField( camera[0].fStop, camera[0].focalLength, camera[0].focalDistance );
	// if we motion-blur the cam, open the motion block
	//
	if ( doCameraMotion && ( !job.isShadow || job.deepShadows) )
	{
		if ( liqglo_relativeMotion ) RiMotionBeginV( liqglo_motionSamples, liqglo_sampleTimesOffsets );
		else 		                     RiMotionBeginV( liqglo_motionSamples, liqglo_sampleTimes );
	}

	// write the camera transform
	//
	RtMatrix cameraMatrix;
	camera[0].mat.get( cameraMatrix );
	RiTransform( cameraMatrix );

	// if we motion-blur the cam, write the subsequent motion samples and close the motion block
	//
	if ( doCameraMotion && ( !job.isShadow || job.deepShadows ) )
	{
		int mm = 1;
		while ( mm < liqglo_motionSamples )
		{
			camera[mm].mat.get( cameraMatrix );
			RiTransform( cameraMatrix );
			++mm;
		}
		RiMotionEnd();
	}
}
/**
 * getCameraTransform
 */
MStatus liqRibTranslator::getCameraTransform( MFnCamera& cam, structCamera &camStruct )
{
	MStatus status;
	MDagPath cameraPath;
	cam.getPath(cameraPath);
	MTransformationMatrix xform( cameraPath.inclusiveMatrix(&status) );
	if ( status != MS::kSuccess ) // error ?!... set identity...
	{
		char errorMsg[512];
		sprintf(errorMsg, "Cannot get transfo matrix for camera '%s' \n", cam.name().asChar());
		//liquidMessage(errorMsg, messageError );
		printf(errorMsg);
		MMatrix id;
		camStruct.mat = id;
		return MS::kFailure;
	}
	// MMatrix mxform = xform.asMatrix();
	// printf("CAM MATRIX '%s' : \n", cam.name().asChar() );
	// printf("%f %f %f %f \n", mxform(0, 0), mxform(0, 1), mxform(0, 2), mxform(0, 3));
	// printf("%f %f %f %f \n", mxform(1, 0), mxform(1, 1), mxform(1, 2), mxform(1, 3));
	// printf("%f %f %f %f \n", mxform(2, 0), mxform(2, 1), mxform(2, 2), mxform(2, 3));
	// printf("%f %f %f %f \n", mxform(3, 0), mxform(3, 1), mxform(3, 2), mxform(3, 3));

	// the camera is pointing toward negative Z
	double scale[] = { 1, 1, -1 };
	xform.setScale( scale, MSpace::kTransform );

	// scanScene:
	// philippe : rotate the main camera 90 degrees around Z-axis if necessary
	// ( only in main camera )
	MMatrix camRotMatrix;
	if ( liqglo_rotateCamera == true )
	{
		float crm[4][4] =	{	{  0.0,  1.0,  0.0,  0.0 },
								{ -1.0,  0.0,  0.0,  0.0 },
								{  0.0,  0.0,  1.0,  0.0 },
								{  0.0,  0.0,  0.0,  1.0 }	};
		camRotMatrix = crm;
	}
	camStruct.mat = xform.asMatrixInverse() * camRotMatrix;
	return MS::kSuccess;
}
/**
 * getCameraFilmOffset
 */
void liqRibTranslator::getCameraFilmOffset( MFnCamera& cam, structCamera &camStruct )
{
	// film back offsets
	double hSize, vSize, hOffset, vOffset;
	cam.getFilmFrustum( cam.focalLength(), hSize, vSize, hOffset, vOffset );

	double imr = ((float)camStruct.width / (float)camStruct.height);
	double fbr = hSize / vSize;
	double ho, vo;
	// convert inches to mm !
	hOffset *= 25.4;
	vOffset *= 25.4;
	switch ( cam.filmFit() )
	{
		case MFnCamera::kVerticalFilmFit:
		case MFnCamera::kFillFilmFit:
		{
			ho = hOffset / vSize * 2.0;
			vo = vOffset / vSize * 2.0;
			break;
		}
		case MFnCamera::kHorizontalFilmFit:
		case MFnCamera::kOverscanFilmFit:
		{
			ho = hOffset / ( vSize * fbr / imr ) * 2.0;
			vo = vOffset / ( vSize * fbr / imr ) * 2.0;
			break;
		}
		default:
		{
			ho = 0;
			vo = 0;
			break;
		}
	}
	camStruct.horizontalFilmOffset = ho;
	camStruct.verticalFilmOffset   = vo;
}
/**
 * Get information about the given camera.
 */
void liqRibTranslator::getCameraInfo( MFnCamera& cam, structCamera &camStruct )
{
  // Resolution can change if camera film-gate clips image
  // so we must keep camera width/height separate from render
  // globals width/height.
  //
  camStruct.width  = width;
  camStruct.height = height;

  // If we are using a film-gate then we may need to
  // adjust the resolution to simulate the 'letter-boxed'
  // effect.
  if ( cam.filmFit() == MFnCamera::kHorizontalFilmFit ) 
	{
    if ( !ignoreFilmGate ) 
		{
      double new_height = camStruct.width / ( cam.horizontalFilmAperture() / cam.verticalFilmAperture() );

      if ( new_height < camStruct.height ) camStruct.height = ( int )new_height;
    }

    double hfov, vfov;
    portFieldOfView( camStruct.width, camStruct.height, hfov, vfov, cam );
    camStruct.fov_ratio = hfov / vfov;
  }
  else if ( cam.filmFit() == MFnCamera::kVerticalFilmFit ) 
	{
    double new_width = camStruct.height / ( cam.verticalFilmAperture() / cam.horizontalFilmAperture() );
    double hfov, vfov;

    // case 1 : film-gate smaller than resolution
    //         film-gate on
    if ( ( new_width < camStruct.width ) && ( !ignoreFilmGate ) ) 
		{
      camStruct.width = ( int )new_width;
      camStruct.fov_ratio = 1.0;
    }
    // case 2 : film-gate smaller than resolution
    //         film-gate off
    else if ( ( new_width < camStruct.width ) && ( ignoreFilmGate ) ) 
		{
      portFieldOfView( ( int )new_width, camStruct.height, hfov, vfov, cam );
      camStruct.fov_ratio = hfov / vfov;
    }
    // case 3 : film-gate larger than resolution
    //         film-gate on
    else if ( !ignoreFilmGate ) 
		{
      portFieldOfView( ( int )new_width, camStruct.height, hfov, vfov, cam );
      camStruct.fov_ratio = hfov / vfov;
    }
    // case 4 : film-gate larger than resolution
    //         film-gate off
    else if ( ignoreFilmGate ) 
		{
      portFieldOfView( ( int )new_width, camStruct.height, hfov, vfov, cam );
      camStruct.fov_ratio = hfov / vfov;
    }
  }
  else if ( cam.filmFit() == MFnCamera::kOverscanFilmFit ) 
  {
    double new_height = camStruct.width / ( cam.horizontalFilmAperture() / cam.verticalFilmAperture() );
    double new_width = camStruct.height / ( cam.verticalFilmAperture() / cam.horizontalFilmAperture() );

    if ( new_width < camStruct.width ) 
		{
      if ( !ignoreFilmGate ) 
			{
        camStruct.width = ( int ) new_width;
        camStruct.fov_ratio = 1.0;
      }
      else 
      {
        double hfov, vfov;
        portFieldOfView( ( int )new_width, camStruct.height, hfov, vfov, cam );
        camStruct.fov_ratio = hfov / vfov;
      }
    }
    else 
    {
      if ( !ignoreFilmGate ) camStruct.height = ( int ) new_height;
      double hfov, vfov;
      portFieldOfView( camStruct.width, camStruct.height, hfov, vfov, cam );
      camStruct.fov_ratio = hfov / vfov;
    }
  }
  else if ( cam.filmFit() == MFnCamera::kFillFilmFit ) 
	{
    double new_width = camStruct.height / ( cam.verticalFilmAperture() / cam.horizontalFilmAperture() );
    double hfov, vfov;

    if ( new_width >= camStruct.width ) 
		{
      portFieldOfView( ( int )new_width, camStruct.height, hfov, vfov, cam );
      camStruct.fov_ratio = hfov / vfov;
    }
    else 
		{
      portFieldOfView( camStruct.width, camStruct.height, hfov, vfov, cam );
      camStruct.fov_ratio = hfov / vfov;
    }
  }
}

/**
 * Set up data for the current job.
 */
MStatus liqRibTranslator::buildJobs()
{
  LIQDEBUGPRINTF( "-> beginning to build job list\n" );
  MStatus returnStatus = MS::kSuccess;
  MStatus status;
  MObject cameraNode;
  MDagPath lightPath;
  jobList.clear();
  shadowList.clear();
  structJob thisJob;

  // what we do here is make all of the lights with depth shadows turned on into
  // cameras and add them to the renderable camera list *before* the main camera
  // so all the automatic depth map shadows are complete before the main pass

  if ( liqglo_doShadows ) 
  {

    MItDag dagIterator( MItDag::kDepthFirst, MFn::kLight, &returnStatus );
    for ( ; !dagIterator.isDone(); dagIterator.next()) 
    {
      if ( !dagIterator.getPath( lightPath ) ) continue;
      bool usesDepthMap = false;
      MFnLight fnLightNode( lightPath );
      liquidGetPlugValue( fnLightNode, "useDepthMapShadows", usesDepthMap, status );
      if ( usesDepthMap && areObjectAndParentsVisible( lightPath ) ) 
      {
        // philippe : this is the default and can be overriden
        // by the everyFrame/renderAtFrame attributes.
        //
        thisJob.renderFrame           = liqglo_lframe;
        thisJob.everyFrame            = true;
        thisJob.shadowObjectSet       = "";
        thisJob.shadowArchiveRibDone  = false;
        thisJob.skip                  = false;
        //
        // We have a shadow job, so find out if we need to use deep shadows,
        // and the pixel sample count
        //
        thisJob.deepShadows                 = false;
        thisJob.shadowPixelSamples          = 1;
        thisJob.shadowVolumeInterpretation  = 1;
        thisJob.shadingRateFactor           = 1.0;
		    thisJob.shadowAggregation			= 0;

        thisJob.imageMode = "z";
        thisJob.format = "shadow";

        // philippe : we grab the job's resolution now instead of in the output phase
        // that way , we can make sure one light can generate many shadow maps
        // with different resolutions
        thisJob.aspectRatio = 1.0;
        liquidGetPlugValue( fnLightNode, "dmapResolution", thisJob.width, status );
        thisJob.height = thisJob.width;

        // Get to our shader node.
        //
        MPlug liquidLightShaderNodeConnection;
        MStatus liquidLightShaderStatus;
        liquidLightShaderNodeConnection = fnLightNode.findPlug( "liquidLightShaderNode", &liquidLightShaderStatus );
        if ( liquidLightShaderStatus == MS::kSuccess && liquidLightShaderNodeConnection.isConnected() )
        {
          MPlugArray liquidLightShaderNodePlugArray;
          liquidLightShaderNodeConnection.connectedTo( liquidLightShaderNodePlugArray, true, true );
          MFnDependencyNode fnLightShaderNode( liquidLightShaderNodePlugArray[0].node() );

          // Now grab the parameters.
          //
          liquidGetPlugValue( fnLightShaderNode, "deepShadows", thisJob.deepShadows, status );
 
          // Only use the pixel samples and volume interpretation with deep shadows.
          //
          if ( thisJob.deepShadows )
          {
            liquidGetPlugValue( fnLightShaderNode, "pixelSamples", thisJob.shadowPixelSamples, status );
            liquidGetPlugValue( fnLightShaderNode, "volumeInterpretation", thisJob.shadowVolumeInterpretation, status );
            
            thisJob.imageMode    = liquidRenderer.dshImageMode;        //"deepopacity";
            thisJob.format       = liquidRenderer.dshDisplayName;    //"deepshad";

            int displayImageMode = 0; // 0 = default
            liquidGetPlugValue( fnLightShaderNode, "liqDeepShadowsDisplayMode", displayImageMode, status );
            if ( displayImageMode ) thisJob.imageMode = MString( "deepprevdisttotal" );
              
            cerr << "dbg> liqDeepShadowsDisplayMode = " << displayImageMode << " thisJob.imageMode = " << thisJob.imageMode << endl;
          }

          // philippe : check the shadow rendering frequency
          //
          liquidGetPlugValue( fnLightShaderNode, "everyFrame", thisJob.everyFrame, status );
      
          // philippe : this is crucial, as we rely on the renderFrame to check
          // which frame the scene should be scanned for that job.
          // If the job is a shadow rendering once only at a given frame, we take the
          // renderAtFrame attribute, otherwise, the current time.
          //
          if ( !thisJob.everyFrame ) 
            liquidGetPlugValue( fnLightShaderNode, "renderAtFrame", thisJob.renderFrame, status );  

          // Check if the shadow aggregation option is used
		      liquidGetPlugValue( fnLightShaderNode, "aggregateShadowMaps", thisJob.shadowAggregation, status );  
 
          // philippe : check the shadow geometry set
          //
          liquidGetPlugValue( fnLightShaderNode, "geometrySet", thisJob.shadowObjectSet, status );
          liquidGetPlugValue( fnLightShaderNode, "shadingRateFactor", thisJob.shadingRateFactor, status );
        } 
        else 
        {
          /* Here we support the same options as those found on light shader nodes
             but we look for dynamic attributes, so we need a bit more error checking.
           */
          liquidGetPlugValue( fnLightNode, "deepShadows", thisJob.deepShadows, status );
          if ( thisJob.deepShadows ) 
          {
            liquidGetPlugValue( fnLightNode, "pixelSamples", thisJob.shadowPixelSamples, status );
            liquidGetPlugValue( fnLightNode, "volumeInterpretation", thisJob.shadowVolumeInterpretation, status );

            thisJob.imageMode    = liquidRenderer.dshImageMode;        //"deepopacity";
            thisJob.format       = liquidRenderer.dshDisplayName;    //"deepshad";

            int displayImageMode = 0; // 0 = default
            liquidGetPlugValue( fnLightNode, "liqDeepShadowsDisplayMode", displayImageMode, status );
            if ( displayImageMode ) thisJob.imageMode = MString( "deepprevdisttotal" );
          }
          liquidGetPlugValue( fnLightNode, "everyFrame", thisJob.everyFrame, status );
          if ( !thisJob.everyFrame ) liquidGetPlugValue( fnLightNode, "renderAtFrame", thisJob.renderFrame, status );  
          
          liquidGetPlugValue( fnLightNode, "geometrySet", thisJob.shadowObjectSet, status );  
          liquidGetPlugValue( fnLightNode, "shadingRateFactor", thisJob.shadingRateFactor, status ); 
        }
        // this will store the shadow camera path and the test's result
        bool lightHasShadowCam = false;
        MDagPathArray shadowCamPath;

        if ( lightPath.hasFn( MFn::kSpotLight ) || lightPath.hasFn( MFn::kDirectionalLight ) ) 
        {
          bool computeShadow = true;
          thisJob.hasShadowCam = false;
          MPlug liquidLightShaderNodeConnection;
          MStatus liquidLightShaderStatus;
          liquidLightShaderNodeConnection = fnLightNode.findPlug( "liquidLightShaderNode", &liquidLightShaderStatus );

          if ( liquidLightShaderStatus == MS::kSuccess && liquidLightShaderNodeConnection.isConnected() ) 
          {
            // a shader is connected to the light !
            MPlugArray liquidLightShaderNodePlugArray;
            liquidLightShaderNodeConnection.connectedTo( liquidLightShaderNodePlugArray, true, true );
            MFnDependencyNode fnLightShaderNode( liquidLightShaderNodePlugArray[0].node() );

            // has the main shadow been disabled ?
            liquidGetPlugValue( fnLightShaderNode, "generateMainShadow", computeShadow, status ); 

            // look for shadow cameras...
            MStatus stat;
            // at first, check if shadow main camera is specified
            // cerr << ">> at first, check if shadow main camera is specified for "  << lightPath.fullPathName().asChar() << endl;
            
            MString camName;
            liquidGetPlugValue( fnLightShaderNode, "shadowMainCamera", camName, status ); 
            if ( status == MS::kSuccess && camName != "" )
            {
              cerr << ">> Light node has main shadow camera : " << camName.asChar() << endl;
              MDagPath cameraPath;
              MSelectionList camList;
              camList.add( camName );
              camList.getDagPath( 0, cameraPath );
              if ( cameraPath.hasFn( MFn::kCamera ) )
              {
                cerr << ">> cameraPath : "<< cameraPath.fullPathName().asChar() << endl;
                thisJob.hasShadowCam = true;
                thisJob.shadowCamPath = cameraPath;
              }
              else
              {
                // cerr << ">> Invalid camera name " << endl;
                string err = "Invalid main shadow camera name " + string( camName.asChar() ) + " for light " + string( lightPath.fullPathName().asChar() );
                liquidMessage( err, messageError );
              }
            }
            
            // now we're looking for extra cameras 
            MPlug shadowCamPlug = fnLightShaderNode.findPlug( "shadowCameras", &stat );
            // find the multi message attribute...
            if ( stat == MS::kSuccess ) 
            {
              int numShadowCams = shadowCamPlug.numElements();
              //cout <<">> got "<<numShadowCams<<" shadow cameras"<<endl;
              // iterate through existing array elements
              for ( unsigned int i(0) ; i < numShadowCams; i++ ) 
              {
                stat.clear();
                MPlug camPlug = shadowCamPlug.elementByPhysicalIndex( i, &stat );
                if( stat != MS::kSuccess ) continue;
                MPlugArray shadowCamPlugArray;

                // if the element is connected, keep going...
                if ( camPlug.connectedTo( shadowCamPlugArray, true, false ) ) 
                {
                  MFnDependencyNode shadowCamDepNode = shadowCamPlugArray[0].node();
                  //cout <<"shadow camera plug "<<i<<" is connected to "<<shadowCamDepNode.name()<<endl;

                  MDagPath cameraPath;
                  cameraPath.getAPathTo( shadowCamPlugArray[0].node(), cameraPath);
                  //cout <<"cameraPath : "<<cameraPath.fullPathName()<<endl;
                  shadowCamPath.append( cameraPath );
                  lightHasShadowCam = true;
                }
              }
            }
          }
		      thisJob.path = lightPath;
		      thisJob.name = fnLightNode.name();
		      thisJob.texName = "";
		      thisJob.isShadow = true;
		      thisJob.isPoint = false;
		      thisJob.isShadowPass = false;

          // check to see if the minmax shadow option is used
          thisJob.isMinMaxShadow = false;
          liquidGetPlugValue( fnLightNode, "liquidMinMaxShadow", thisJob.isMinMaxShadow, status ); 
          // check to see if the midpoint shadow option is used
          thisJob.isMidPointShadow = false;
          liquidGetPlugValue( fnLightNode, "useMidDistDmap", thisJob.isMidPointShadow, status ); 
          // in lazy compute mode, we check if the map is already on disk first.
          if ( m_lazyCompute && computeShadow ) 
          {
            MString fileName( generateFileName( (fileGenMode) fgm_shadow_tex, thisJob ) );
            if ( fileExists( fileName ) ) computeShadow = false;
          }
          //
          // store the main shadow map    *****************************
          //
          if ( computeShadow ) jobList.push_back( thisJob );
          // We have to handle point lights differently as they need 6 shadow maps!
        } 
        else if ( lightPath.hasFn(MFn::kPointLight) ) 
        {
          for ( unsigned dirOn( 0 ); dirOn < 6; dirOn++ ) 
          {
            thisJob.hasShadowCam = false;
            thisJob.path = lightPath;
            thisJob.name = fnLightNode.name();
            thisJob.isShadow = true;
            thisJob.isShadowPass = false;
            thisJob.isPoint = true;
            thisJob.pointDir = ( PointLightDirection )dirOn;

            // check to see if the midpoint shadow option is used
            thisJob.isMidPointShadow = false;
            liquidGetPlugValue( fnLightNode, "useMidDistDmap", thisJob.isMidPointShadow, status );
            
            bool computeShadow = true;
            MStatus liquidLightShaderStatus;
            MPlug liquidLightShaderNodeConnection( fnLightNode.findPlug( "liquidLightShaderNode", &liquidLightShaderStatus ) );

            if ( liquidLightShaderStatus == MS::kSuccess && liquidLightShaderNodeConnection.isConnected() ) 
            {
              // a shader is connected to the light !
              MPlugArray liquidLightShaderNodePlugArray;
              liquidLightShaderNodeConnection.connectedTo( liquidLightShaderNodePlugArray, true, true );
              MFnDependencyNode fnLightShaderNode( liquidLightShaderNodePlugArray[0].node() );

              // has the main shadow been disabled ?
              liquidGetPlugValue( fnLightShaderNode, "generateMainShadow", computeShadow, status );

              // look for shadow cameras...
              MStatus stat;
              MPlug shadowCamPlug( fnLightShaderNode.findPlug( "shadowCameras", &stat ) );

              // find the multi message attribute...
              if ( stat == MS::kSuccess ) 
              {
                int numShadowCams = shadowCamPlug.numElements();
                //cout <<">> got "<<numShadowCams<<" shadow cameras"<<endl;
                // iterate through existing array elements
                for ( unsigned i( 0 ); i < numShadowCams; i++ ) 
                {
                  stat.clear();
                  MPlug camPlug = shadowCamPlug.elementByPhysicalIndex( i, &stat );
                  if( stat != MS::kSuccess ) continue;
                  MPlugArray shadowCamPlugArray;

                  // if the element is connected, keep going...
                  if ( camPlug.connectedTo( shadowCamPlugArray, true, false ) ) 
                  {
                    MFnDependencyNode shadowCamDepNode = shadowCamPlugArray[0].node();
                    //cout <<"shadow camera plug "<<i<<" is connected to "<<shadowCamDepNode.name()<<endl;

                    MDagPath cameraPath;
                    cameraPath.getAPathTo( shadowCamPlugArray[0].node(), cameraPath);
                    //cout <<"cameraPath : "<<cameraPath.fullPathName()<<endl;
                    shadowCamPath.append( cameraPath );
                    lightHasShadowCam = true;
                  }
                }
              }
            }
            if ( m_lazyCompute )
            {
              MString fileName( generateFileName( ( fileGenMode )fgm_shadow_tex, thisJob ) );
              if ( fileExists( fileName ) ) computeShadow = false;
            }
            if ( computeShadow ) jobList.push_back( thisJob );
          }
        }
			  // if the job has shadow cameras, we will store them here
			  //
			  if ( lightHasShadowCam )
			  {
				  int isAggregate = thisJob.shadowAggregation;
				  for ( unsigned i( 0 ); i < shadowCamPath.length(); i++ )
				  {
					  if ( !i && isAggregate )
						  thisJob.shadowAggregation = 0;
					  else if ( isAggregate )
						  thisJob.shadowAggregation = 1;
					  else
						  thisJob.shadowAggregation = 0;
					  thisJob.shadowCamPath = shadowCamPath[ i ];
					  thisJob.hasShadowCam = true;

					  MFnDependencyNode shadowCamDepNode( shadowCamPath[ i ].node() );
					  thisJob.name = shadowCamDepNode.name();
					  if ( isAggregate )
						  thisJob.texName = fnLightNode.name(); //MFnDependencyNode( shadowCamPath[ i ].node() ).name();
  //					else
  //						thisJob.texName = "";
  //						thisJob.name = shadowCamDepNode.name();
					  if ( liquidGetPlugValue( shadowCamDepNode, "liqShadowResolution", thisJob.width, status ) == MS::kSuccess )
						  thisJob.height = thisJob.width;
					  liquidGetPlugValue( shadowCamDepNode, "liqMidPointShadow", thisJob.isMidPointShadow, status );
            thisJob.midPointRatio = 0;
            liquidGetPlugValue( shadowCamDepNode, "liqMidPointRatio", thisJob.midPointRatio, status );
            liquidGetPlugValue( shadowCamDepNode, "liqDeepShadows", thisJob.deepShadows, status );
            liquidGetPlugValue( shadowCamDepNode, "liqPixelSamples", thisJob.shadowPixelSamples, status );
            liquidGetPlugValue( shadowCamDepNode, "liqVolumeInterpretation", thisJob.shadowVolumeInterpretation, status );
            
            int displayImageMode = 0; // 0 = default
            liquidGetPlugValue( shadowCamDepNode, "liqDeepShadowsDisplayMode", displayImageMode, status );
            if ( displayImageMode ) thisJob.imageMode = MString( "deepprevdisttotal" );

            liquidGetPlugValue( shadowCamDepNode, "liqEveryFrame", thisJob.everyFrame, status );
					  // as previously : this is important as thisJob.renderFrame corresponds to the
					  // scene scanning time.
					  if ( thisJob.everyFrame ) thisJob.renderFrame = liqglo_lframe;
					  else liquidGetPlugValue( shadowCamDepNode, "liqRenderAtFrame", thisJob.renderFrame, status );
            liquidGetPlugValue( shadowCamDepNode, "liqGeometrySet", thisJob.shadowObjectSet, status );
            liquidGetPlugValue( shadowCamDepNode, "liqShadingRateFactor", thisJob.shadingRateFactor, status );
					  // test if the file is already on disk...
					  if ( m_lazyCompute )
					  {
						  MString fileName( generateFileName( ( fileGenMode )fgm_shadow_tex, thisJob ) );
						  if ( fileExists( fileName ) )
							  continue;
					  }
					  jobList.push_back( thisJob );
				  }
			  }
		  } // useDepthMap
		//cout <<thisJob.name.asChar()<<" -> shd:"<<thisJob.isShadow<<" ef:"<<thisJob.everyFrame<<" raf:"<<thisJob.renderFrame<<" set:"<<thisJob.shadowObjectSet.asChar()<<endl;
	  } // light dagIterator

    MDagPath cameraPath;
    MItDag dagCameraIterator( MItDag::kDepthFirst, MFn::kCamera, &returnStatus );
    for ( ; !dagCameraIterator.isDone(); dagCameraIterator.next() ) 
    {
      if ( !dagCameraIterator.getPath(cameraPath) ) continue;
      bool usesDepthMap( false );
      MFnCamera fnCameraNode( cameraPath );
      liquidGetPlugValue( fnCameraNode, "useDepthMapShadows", usesDepthMap, status );
      if ( usesDepthMap && areObjectAndParentsVisible( cameraPath ) ) 
      {
        //
        // We have a shadow job, so find out if we need to use deep shadows,
        // and the pixel sample count
        //
        thisJob.deepShadows = false;
        thisJob.shadowPixelSamples = 1;
        thisJob.shadowVolumeInterpretation = 1;
        fnCameraNode.findPlug( "deepShadows" ).getValue( thisJob.deepShadows );
        // Only use the pixel samples and volume interpretation with deep shadows.
        //
        if ( thisJob.deepShadows )
        {
          fnCameraNode.findPlug( "pixelSamples" ).getValue( thisJob.shadowPixelSamples );
          fnCameraNode.findPlug( "volumeInterpretation" ).getValue( thisJob.shadowVolumeInterpretation );
          
          thisJob.imageMode    = liquidRenderer.dshImageMode;        //"deepopacity";
          thisJob.format       = liquidRenderer.dshDisplayName;    //"deepshad";

          int displayImageMode = 0; // 0 = default
          fnCameraNode.findPlug( "liqDeepShadowsDisplayMode" ).getValue( displayImageMode );
          if ( displayImageMode ) thisJob.imageMode = MString("deepprevdisttotal");
        }

        thisJob.hasShadowCam = true;
        thisJob.shadowCamPath = cameraPath;
        thisJob.path = cameraPath;
        thisJob.name = fnCameraNode.name();
        thisJob.isShadow = true;
        thisJob.isPoint = false;
        thisJob.isShadowPass = false;

        // check to see if the minmax shadow option is used
        thisJob.isMinMaxShadow = false;
        status.clear();
        MPlug liquidMinMaxShadow = fnCameraNode.findPlug( "liquidMinMaxShadow", &status );
        if ( status == MS::kSuccess ) liquidMinMaxShadow.getValue( thisJob.isMinMaxShadow );

        // check to see if the midpoint shadow option is used
        thisJob.isMidPointShadow = false;
        status.clear();
        MPlug liquidMidPointShadow = fnCameraNode.findPlug( "useMidDistDmap", &status );
        if ( status == MS::kSuccess ) liquidMidPointShadow.getValue( thisJob.isMidPointShadow );

        bool computeShadow( true );
        if ( m_lazyCompute ) 
        {
          MString fileName( generateFileName( ( fileGenMode )fgm_shadow_tex, thisJob ) );
          if ( fileExists( fileName ) ) continue;
        }

        if ( computeShadow ) jobList.push_back( thisJob );
      }
    } // camera dagIterator
  } // liqglo_doShadows

  // Determine which cameras to render
  // it will either traverse the dag and find all the renderable cameras or
  // grab the current view and render that as a camera - both get added to the
  // end of the renderable camera array
  MFnCamera fnCameraNode( m_camDagPath );
  thisJob.renderFrame   = liqglo_lframe;
  thisJob.everyFrame    = true;
  thisJob.isPoint       = false;
  thisJob.path          = m_camDagPath;
  thisJob.name          = fnCameraNode.name();
  thisJob.isShadow      = false;
  thisJob.skip          = false;
  thisJob.isShadowPass  = false;
  if ( m_outputShadowPass )
  {
    thisJob.name         += "SHADOWPASS";
    thisJob.isShadowPass  = true;
    jobList.push_back( thisJob );
  }
  
  if ( m_outputHeroPass ) jobList.push_back( thisJob );

  liqglo_shutterTime    = fnCameraNode.shutterAngle() * 0.5 / M_PI;

  // If we didn't find a renderable camera then give up
  if ( jobList.size() == 0 ) 
  {
    MString cError("No Renderable Camera Found!\n");
    throw( cError );
    return MS::kFailure;
  }

	// step through the jobs and setup their names
	vector<structJob>::iterator iter = jobList.begin();
	while ( iter != jobList.end() )
	{
		LIQ_CHECK_CANCEL_REQUEST;
		thisJob = *iter;

		MString frameFileName;
		//if ( thisJob.isShadow ) frameFileName = generateFileName( ( fileGenMode )fgm_shadow_rib, thisJob );
		//else 			              frameFileName = generateFileName( ( fileGenMode )fgm_beauty_rib, thisJob );
		
		frameFileName = generateFileName( ( thisJob.isShadow )? fgm_shadow_rib : fgm_beauty_rib, thisJob );

		iter->ribFileName = frameFileName;

		// set the skip flag for the job
		iter->skip   = false;
		thisJob.skip = false;

		if ( thisJob.isShadow )
		{
			if ( !liqglo_doShadows )
			{
				// shadow generation disabled
				iter->skip   = true;
				thisJob.skip = true;
			}
			else if ( !thisJob.everyFrame && ( liqglo_noSingleFrameShadows || liqglo_lframe > frameNumbers[ 0 ] && thisJob.renderFrame != liqglo_lframe ) )
			{
				// noSingleFrameShadows or rendering past the first frame of the sequence
				iter->skip   = true;
				thisJob.skip = true;
			}
			else if ( thisJob.everyFrame && liqglo_singleFrameShadowsOnly )
			{
				// singleFrameShadowsOnly on regular shadows
				iter->skip   = true;
				thisJob.skip = true;
			}
		}
		else if ( liqglo_singleFrameShadowsOnly )
		{
			// singleFrameShadowsOnly on hero pass
			iter->skip   = true;
			thisJob.skip = true;
		}

		MString outFileFmtString;

    if ( thisJob.isShadow ) 
    {
      MString varVal;
      MString userShadowName;
      MFnDagNode lightNode( thisJob.path );

      if ( liquidGetPlugValue( lightNode, "liquidShadowName", varVal, status ) == MS::kSuccess ) 
        userShadowName = parseString( varVal, false );
      //outFileFmtString = liqglo_textureDir;

      //MString outName;
      //if ( userShadowName.length() ) outName = userShadowName;
      // else outName = generateFileName( ( fileGenMode )fgm_shadow_tex, thisJob );

			MString outName = ( userShadowName.length() )? userShadowName : 
																										 generateFileName( ( fileGenMode )fgm_shadow_tex, thisJob );

      iter->imageName = outName;
      thisJob = *iter;
      if ( thisJob.isShadow ) shadowList.push_back( thisJob );
    } 
    else 
    {
      MString outName;
      outName = generateFileName( ( fileGenMode )fgm_image, thisJob );
      iter->imageName = outName;
    }
    ++iter;
  }
  // sort the shadow jobs to put the reference frames first
#ifndef _WIN32
  sort( jobList.begin(), jobList.end(), renderFrameSort );
  sort( shadowList.begin(), shadowList.end(), renderFrameSort );
#else
  sort( jobList.begin(), jobList.end(), renderFrameSort );
  sort( shadowList.begin(), shadowList.end(), renderFrameSort );
#endif
  ribStatus = kRibBegin;
  return MS::kSuccess;
}

/**
 * Write the prologue for the RIB file.
 * This includes all RI options but not the camera transformation.
 */
MStatus liqRibTranslator::ribPrologue()
{
  if ( !m_exportReadArchive ) 
  {
    LIQDEBUGPRINTF( "-> beginning to write prologue\n" );
    // general info for traceability
    //
    RiArchiveRecord( RI_COMMENT, "    Generated by Liquid v%s", LIQUIDVERSION );
    RiArchiveRecord( RI_COMMENT, "    Scene : %s", (liqglo_projectDir + liqglo_sceneName).asChar() );
#ifndef _WIN32
    uid_t userId = getuid();
    struct passwd *userPwd = getpwuid( userId );
    RiArchiveRecord( RI_COMMENT, "    User  : %s", userPwd->pw_name );
#else
    char* user = getenv("USERNAME");
    if( user )
        RiArchiveRecord( RI_COMMENT, "    User  : %s", user );
#endif
    time_t now;
    time( &now );
    char* theTime = ctime(&now);
    RiArchiveRecord( RI_COMMENT, "    Time  : %s", theTime );
    // set any rib options
    //
    if ( m_statistics != 0 )  
    {
      if ( m_statistics < 4 ) RiOption( "statistics", "endofframe", ( RtPointer ) &m_statistics, RI_NULL );
      else 
      {
        //cout <<"xml stats "<<endl;
        int stats = 1;
        RiOption( "statistics", "int endofframe", ( RtPointer ) &stats, RI_NULL );
        RiArchiveRecord( RI_VERBATIM, "Option \"statistics\" \"xmlfilename\" [\"%s\"]\n", const_cast< char* > ( m_statisticsFile.asChar() ) );
      }
    }
    if ( bucketSize != 0 )    RiOption( "limits", "bucketsize", ( RtPointer ) &bucketSize, RI_NULL );
    if ( gridSize != 0 )      RiOption( "limits", "gridsize", ( RtPointer ) &gridSize, RI_NULL );
    if ( textureMemory != 0 ) RiOption( "limits", "texturememory", ( RtPointer) &textureMemory, RI_NULL );
    if ( liquidRenderer.supports_EYESPLITS ) 
      RiOption( "limits", "eyesplits", ( RtPointer ) &eyeSplits, RI_NULL );
    
    if ( liquidRenderer.renderName == MString("PRMan") || liquidRenderer.renderName == MString("3Delight") )
    {
      RtColor othresholdC = {othreshold[0], othreshold[1], othreshold[2]};
      RiOption( "limits", "othreshold", &othresholdC, RI_NULL );
      RtColor zthresholdC = {zthreshold[0], zthreshold[1], zthreshold[2]};
      RiOption( "limits", "zthreshold", &zthresholdC, RI_NULL );
    }
    // set search paths
    //
    if ( m_dirmaps.length() )
    {
      using namespace std;
      using namespace boost;
      
      const string str( m_dirmaps.asChar() );
      stringstream ss;
      vector< string > names;
      typedef tokenizer< char_separator< char > > tokenizer;
      char_separator< char > sep("{ }");
      tokenizer tokens(str, sep);
      for ( tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter )
        names.push_back( *tok_iter );
      for ( unsigned i( 0 ); i < names.size(); i+= 3 )
      {
        // names.at(i+2)  - zone
        // names.at(i)    - from
        // names.at(i+1)  - to
        // [\"UNC\" \"/from_path/\" \"//comp/to_path/\"]
        #ifdef GENERIC_RIBLIB
        ss << "[\\\"" << names.at(i+2) << "\\\" \\\"" << names.at(i) << "\\\" \\\"" << names.at(i+1) << "\\\"] ";
        #else
        ss << "[\"" << names.at(i+2) << "\" \"" << names.at(i) << "\" \"" << names.at(i+1) << "\"] ";
        #endif
      }
      // cout << ss.str() << endl;
      string dirmapsPath ( ss.str() );
      RtString list = const_cast< char* > ( dirmapsPath.c_str() );
      RiOption( "searchpath", "string dirmap", &list, RI_NULL );
    }
    RtString list = const_cast< char* > ( liqglo_shaderPath.asChar() );
    RiOption( "searchpath", "shader", &list, RI_NULL );

    // MString texturePath = liqglo_texturePath  + ":" + liquidSanitizeSearchPath( liqglo_textureDir );
    list = const_cast< char* > ( liqglo_texturePath.asChar() );
    RiOption( "searchpath", "texture", &list, RI_NULL );

    // MString archivePath = liqglo_archivePath + ":" + liquidSanitizeSearchPath( liqglo_ribDir );
    list = const_cast< char* > ( liqglo_archivePath.asChar() );
    RiOption( "searchpath", "archive", &list, RI_NULL );

    list = const_cast< char* > ( liqglo_proceduralPath.asChar() );
    RiOption( "searchpath", "procedural", &list, RI_NULL );

    // if rendering to the renderview, add a path to the liqmaya display driver
    if ( m_renderView ) 
    {
      MString home( getenv( "LIQUIDHOME" ) );
      MString displaySearchPath;
      if ( (liquidRenderer.renderName == MString("Pixie")) || (liquidRenderer.renderName == MString("Air")) || (liquidRenderer.renderName == MString("3Delight")) )
        displaySearchPath = ".:@::" + liquidRenderer.renderHome + "/displays:" + liquidSanitizePath( home ) + "/displayDrivers/" + liquidRenderer.renderName + "/";
      else 
        displaySearchPath = ".:@:" + liquidRenderer.renderHome + "/etc:" + liquidSanitizePath( home ) +  "/displayDrivers/" + liquidRenderer.renderName + "/";
      
      list = const_cast< char* > ( displaySearchPath.asChar() );
      RiArchiveRecord( RI_VERBATIM, "Option \"searchpath\" \"display\" [\"%s\"]\n", list );
    }
	  //RiOrientation( RI_RH ); // Right-hand coordinates
    if ( liqglo_currentJob.isShadow ) 
    {
      RiPixelSamples( liqglo_currentJob.shadowPixelSamples, liqglo_currentJob.shadowPixelSamples );
      RiShadingRate( liqglo_currentJob.shadingRateFactor );
      // Need to use Box filter for deep shadows.
      RiPixelFilter( RiBoxFilter, 1, 1 );
      RtString option;
      if ( liqglo_currentJob.deepShadows ) option = "deepshadow";
      else                                 option = "shadow";
      RiOption( "user", "string pass", ( RtPointer )&option, RI_NULL );
    } 
    else 
    {
      RtString hiderName;
      switch ( liqglo_hider ) 
      {
        case htPhoton:
          hiderName = "photon";
          break;
        case htRaytrace:
          hiderName = "raytrace";
          break;
        case htOpenGL:
          hiderName = "OpenGL";
          break;
        case htZbuffer:
          hiderName = "zbuffer";
          break;
        case htDepthMask:
          hiderName = "depthmask";
          break;
        case htHidden:
        default:
          hiderName = "hidden";
      }
      MString hiderOptions = getHiderOptions( liquidRenderer.renderName, hiderName );
      RiArchiveRecord( RI_VERBATIM, "Hider \"%s\" %s\n", hiderName, ( char* )hiderOptions.asChar() );
      RiPixelSamples( pixelSamples, pixelSamples );
      RiShadingRate( shadingRate );
      if ( m_rFilterX > 1 || m_rFilterY > 1 ) 
      {
        switch ( m_rFilter ) 
        {
          case pfBoxFilter:
            RiPixelFilter( RiBoxFilter, m_rFilterX, m_rFilterY );
            break;
          case pfTriangleFilter:
            RiPixelFilter( RiTriangleFilter, m_rFilterX, m_rFilterY );
            break;
          case pfCatmullRomFilter:
            RiPixelFilter( RiCatmullRomFilter, m_rFilterX, m_rFilterY );
            break;
          case pfGaussianFilter:
            RiPixelFilter( RiGaussianFilter, m_rFilterX, m_rFilterY );
            break;
          case pfSincFilter:
            RiPixelFilter( RiSincFilter, m_rFilterX, m_rFilterY );
            break;
#if defined ( DELIGHT ) || defined ( PRMAN ) || defined ( GENERIC_RIBLIB )
          case pfBlackmanHarrisFilter:
            RiArchiveRecord( RI_VERBATIM, "PixelFilter \"blackman-harris\" %g %g\n", m_rFilterX, m_rFilterY);
            break;
          case pfMitchellFilter:
            RiArchiveRecord( RI_VERBATIM, "PixelFilter \"mitchell\" %g %g\n", m_rFilterX, m_rFilterY);
            break;
          case pfSepCatmullRomFilter:
            RiArchiveRecord( RI_VERBATIM, "PixelFilter \"separable-catmull-rom\" %g %g\n", m_rFilterX, m_rFilterY);
            break;
          case pfBesselFilter:
            RiArchiveRecord( RI_VERBATIM, "PixelFilter \"bessel\" %g %g\n", m_rFilterX, m_rFilterY);
            break;
#endif
#if defined ( PRMAN ) || defined ( GENERIC_RIBLIB )
          case pfLanczosFilter:
            RiArchiveRecord( RI_VERBATIM, "PixelFilter \"lanczos\" %g %g\n", m_rFilterX, m_rFilterY);
            break;
          case pfDiskFilter:
            RiArchiveRecord( RI_VERBATIM, "PixelFilter \"disk\" %g %g\n", m_rFilterX, m_rFilterY);
            break;
#endif
          default:
            RiArchiveRecord( RI_COMMENT, "Unknown Pixel Filter selected" );
            break;
        }
      }
      RtString option( "beauty" );
      RiOption( "user", "string pass", ( RtPointer )&option, RI_NULL );
    }

    // RAYTRACING OPTIONS
    if ( liquidRenderer.supports_RAYTRACE && rt_useRayTracing ) 
    {
      RiArchiveRecord( RI_COMMENT, "Ray Tracing : ON" );
      RiOption( "trace",   "int maxdepth",                ( RtPointer ) &rt_traceMaxDepth,            RI_NULL );
#if defined ( DELIGHT ) || defined ( PRMAN ) || defined ( GENERIC_RIBLIB )
      RiOption( "trace",   "float specularthreshold",     ( RtPointer ) &rt_traceSpecularThreshold, RI_NULL );
      RiOption( "trace",   "int continuationbydefault",   ( RtPointer ) &rt_traceRayContinuation,   RI_NULL );
      RiOption( "limits",  "int geocachememory",          ( RtPointer ) &rt_traceCacheMemory,       RI_NULL );
      RiOption( "user",    "float traceBreadthFactor",    ( RtPointer ) &rt_traceBreadthFactor,     RI_NULL );
      RiOption( "user",    "float traceDepthFactor",      ( RtPointer ) &rt_traceDepthFactor,       RI_NULL );
#endif
    } 
    else 
    {
      if ( !liquidRenderer.supports_RAYTRACE ) RiArchiveRecord( RI_COMMENT, "Ray Tracing : NOT SUPPORTED" );
      else 
      {
        RiArchiveRecord( RI_COMMENT, "Ray Tracing : OFF" );
        RtInt maxDepth = 0;
        RiOption( "trace",   "int maxdepth",                ( RtPointer ) &maxDepth,                  RI_NULL );
      }
    }
    // CUSTOM OPTIONS
	  MFnDependencyNode globalsNode( rGlobalObj );
	  MPlug prePostplug( globalsNode.findPlug( "preFrameBeginMel" ) );
	  MString melcommand( prePostplug.asString() );
    if ( m_preFrameRIB != "" || melcommand.length() )
	  {
		  RiArchiveRecord(RI_COMMENT,  " Pre-FrameBegin RIB from liquid globals" );
		  MGlobal::executeCommand( melcommand );
		  RiArchiveRecord(RI_VERBATIM, ( char* )m_preFrameRIB.asChar() );
		  RiArchiveRecord(RI_VERBATIM, "\n");
    }
    if ( m_bakeNonRasterOrient || m_bakeNoCullHidden || m_bakeNoCullBackface ) 
    {
      RiArchiveRecord( RI_COMMENT, "Bake Attributes" );
      RtInt zero( 0 );
      if ( m_bakeNonRasterOrient ) RiAttribute( "dice","int rasterorient", &zero, RI_NULL );
      if ( m_bakeNoCullBackface )  RiAttribute( "cull","int backfacing", &zero, RI_NULL );
      if ( m_bakeNoCullHidden )    RiAttribute( "cull","int hidden", &zero, RI_NULL );
    }
  }
  if ( liqglo_doMotion ) RiGeometricApproximation( "motionfactor", liqglo_motionFactor );
  ribStatus = kRibBegin;
  return MS::kSuccess;
}

/**
 * Write the epilogue for the RIB file.
 */
MStatus liqRibTranslator::ribEpilogue()
{
  if ( ribStatus == kRibBegin )  ribStatus = kRibOK;
  return ( ribStatus == kRibOK ? MS::kSuccess : MS::kFailure );
}

/**
 * Scan the DAG at the given frame number and record information about the scene for writing.
 */
MStatus liqRibTranslator::scanSceneNodes( MObject &currentNode, 
                                          MDagPath &path, 
                                          float lframe, 
                                          int sample, 
                                          int &count, 
                                          MStatus& returnStatus ) 
{
  MFnDagNode dagNode;
  returnStatus = dagNode.setObject( currentNode );
  if ( MS::kSuccess != returnStatus ) return returnStatus;

  LIQ_CHECK_CANCEL_REQUEST;
  
  bool useSamples( ( sample > 0 ) && isObjectMotionBlur( path ) );
  
//  if ( isObjectMotionBlur( path ) )
//    cout << ">>  isObjectMotionBlur == 1 " << path.fullPathName() << endl;

  // scanScene: check for a rib generator
  MStatus plugStatus;
  MPlug ribGenPlug = dagNode.findPlug( "liquidRibGen", &plugStatus );
  if ( plugStatus == MS::kSuccess )
  {
	  // scanScene: check the node to make sure it's not using the old ribGen assignment method, this is for backwards
	  // compatibility.  If it's a kTypedAttribute that it's more than likely going to be a string!
	  if ( ribGenPlug.attribute().apiType() == MFn::kTypedAttribute )
	  {
		  MString ribGenNode;
		  ribGenPlug.getValue( ribGenNode );
		  MSelectionList ribGenList;
		  MStatus ribGenAddStatus = ribGenList.add( ribGenNode );
		  if ( ribGenAddStatus == MS::kSuccess ) htable->insert( path, lframe, sample, MRT_RibGen, count++ );
	  }
	  else
	  {
		  if ( ribGenPlug.isConnected() ) htable->insert( path, lframe, sample, MRT_RibGen, count++ );
	  }

  }
  if ( currentNode.hasFn( MFn::kNurbsSurface )
	  || currentNode.hasFn( MFn::kMesh )
	  || currentNode.hasFn( MFn::kParticle )
	  || currentNode.hasFn( MFn::kLocator )
	  || currentNode.hasFn( MFn::kSubdiv )
	  || currentNode.hasFn( MFn::kPfxHair )
	  || currentNode.hasFn( MFn::kPfxToon )
	  || currentNode.hasFn( MFn::kImplicitSphere )
		|| currentNode.hasFn( MFn::kPluginShape ) ) // include plugin shapes as placeholders
  {
	  LIQDEBUGPRINTF( "==> inserting obj to htable %s\n", path.fullPathName().asChar() );
    htable->insert( path, lframe, ( useSamples )? sample : 0, MRT_Unknown, count++ );
    LIQDEBUGPRINTF( "==> %s inserted\n", path.fullPathName().asChar() );
  }
  // Alf: treat PFX as three separate entities so a separate shading group can
  // be assigned to each part
  if ( currentNode.hasFn( MFn::kPfxGeometry ) )
  {
	  LIQDEBUGPRINTF( "==> inserting kPfxGeometry\n" );
    MStatus status;
	  MRenderLineArray tube, leaf, petal;
	  MFnPfxGeometry pfx( path, &status );
	  if ( status == MS::kSuccess ) pfx.getLineData( tube, leaf, petal,1,0,0,0,0,0,0,0,0 );
	  if ( tube.length() ) htable->insert( path, lframe, ( useSamples )? sample : 0, MRT_PfxTube, count++ );
	  if ( leaf.length() ) htable->insert( path, lframe, ( useSamples )? sample : 0, MRT_PfxLeaf, count++ );
	  if ( petal.length() ) htable->insert( path, lframe, ( useSamples )? sample : 0, MRT_PfxPetal, count++ );

	  tube.deleteArray(); 
    leaf.deleteArray(); 
    petal.deleteArray();
  }
  if ( currentNode.hasFn( MFn::kNurbsCurve ) )
  {
	  LIQDEBUGPRINTF( "==> inserting kNurbsCurve\n" );

    MStatus plugStatus;
	  MPlug renderCurvePlug = dagNode.findPlug( "liquidCurve", &plugStatus );
	  if ( liqglo_renderAllCurves || ( plugStatus == MS::kSuccess ) )
	  {
		  bool renderCurve( false );
		  renderCurvePlug.getValue( renderCurve );
		  if ( renderCurve ) htable->insert( path, lframe, ( useSamples )? sample : 0, MRT_NuCurve, count++ );
	  }
  }
  return returnStatus;
}

/**
 * Scan the DAG at the given frame number and record information about the scene for writing.
 */
MStatus liqRibTranslator::scanScene( float lframe, int sample )
{
  int count =0;

  MTime mt( ( double )lframe, MTime::uiUnit() );
  if ( MGlobal::viewFrame( mt ) == MS::kSuccess ) 
  {
    // scanScene: execute pre-frame command
    if ( m_preFrameMel != "" ) 
    {
      MString preFrameMel( parseString( m_preFrameMel, false ) );
      if ( fileExists( preFrameMel  ) ) MGlobal::sourceFile( preFrameMel );
      else 
      {
        if ( MS::kSuccess == MGlobal::executeCommand( preFrameMel, false, false ) ) 
          liquidMessage( "pre-frame script executed successfully.", messageInfo );
        else 
          liquidMessage( "pre-frame script failed.", messageError );
      }
    }

    MStatus returnStatus;
    // scan the scene for lights
    {
      MItDag dagLightIterator( MItDag::kDepthFirst, MFn::kLight, &returnStatus);

      for ( ; !dagLightIterator.isDone() ; dagLightIterator.next() ) 
      {
        LIQ_CHECK_CANCEL_REQUEST;
        MDagPath path;
        MObject currentNode;
        currentNode = dagLightIterator.item();
        MFnDagNode dagNode;
        dagLightIterator.getPath( path );
        
        if ( MS::kSuccess != returnStatus ) continue;
        if ( !currentNode.hasFn( MFn::kDagNode ) ) continue;
        
        returnStatus = dagNode.setObject( currentNode );
        if ( MS::kSuccess != returnStatus ) continue;
          
        bool useSamples( ( sample > 0 ) && isObjectMotionBlur( path ) );

        // scanScene: if it's a light then insert it into the hash table
        if ( currentNode.hasFn( MFn::kLight ) ) 
        {
          if ( currentNode.hasFn( MFn::kAreaLight ) ) 
          {
            // add a coordSys node if necessary
            MStatus status;
            bool coordsysExists = false;
            // get the coordsys name
            MFnDependencyNode areaLightDep( currentNode );
            MString coordsysName = areaLightDep.name() + "CoordSys";
            // get the transform
            MObject transform = path.transform();
            // check the coordsys does not exist yet under the transform
            MFnDagNode transformDag( transform );
            int numChildren = transformDag.childCount();
            if ( numChildren > 1 ) 
            {
              for ( unsigned int i=0; i<numChildren; i++ ) 
              {
                MObject childObj = transformDag.child( i, &status );
                if ( status == MS::kSuccess && childObj.hasFn( MFn::kLocator ) ) 
                {
                  MFnDependencyNode test(childObj);
                  if ( test.name() == coordsysName ) coordsysExists = true;
                }
              }
            }
            if ( !coordsysExists ) 
            {
              // create the coordsys
              MDagModifier coordsysNode;
              MObject coordsysObj  = coordsysNode.createNode( "liquidCoordSys", transform, &status );
              if ( status == MS::kSuccess ) 
              {
                // rename node to match light name
                coordsysNode.doIt();
                if ( status == MS::kSuccess ) 
                {
                  MFnDependencyNode coordsysDep( coordsysObj );
                  coordsysDep.setName( coordsysName );
                }
              }
            }
          }
          htable->insert( path, lframe, ( useSamples )? sample : 0, MRT_Light,	count++ );
          continue;
        } // if ( currentNode.hasFn( MFn::kLight ) ) 
      }
    } // scan the scene for lights
    // scan CoordSys
		{
      MItDag dagCoordSysIterator( MItDag::kDepthFirst, MFn::kLocator, &returnStatus);

      for ( ; !dagCoordSysIterator.isDone() ; dagCoordSysIterator.next() ) 
      {
        LIQ_CHECK_CANCEL_REQUEST;
        MDagPath path;
        MObject currentNode;
        currentNode = dagCoordSysIterator.item();
        MFnDagNode dagNode;
        
        dagCoordSysIterator.getPath( path );
        
        if ( MS::kSuccess != returnStatus ) continue;
        if ( !currentNode.hasFn(MFn::kDagNode) ) continue;
        
        returnStatus = dagNode.setObject( currentNode );
        if ( MS::kSuccess != returnStatus ) continue;
          
        bool useSamples( ( sample > 0 ) && isObjectMotionBlur( path ) );

        // scanScene: if it's a coordinate system then insert it into the hash table
        if ( dagNode.typeName() == "liquidCoordSys" ) 
        {
          int coordType = 0;
          MPlug typePlug = dagNode.findPlug( "type", &returnStatus );
          if ( MS::kSuccess == returnStatus ) typePlug.getValue( coordType );
            
          htable->insert( path, 
                          lframe, 
                          ( useSamples )? sample : 0, 
                          ( coordType == 5 )? MRT_ClipPlane : MRT_Coord, 
                          count++ );
          continue;
        }
      }
    } // scan CoordSys

	  if ( !m_renderSelected && !m_exportSpecificList )
	  {
		  MItDag dagIterator( MItDag::kDepthFirst, MFn::kInvalid, &returnStatus);
      for ( ; !dagIterator.isDone() ; dagIterator.next() )
      {
        MDagPath path;
        dagIterator.getPath( path );
        MObject currentNode = dagIterator.item();
        if ( !currentNode.hasFn( MFn::kDagNode ) ) continue;
	      
        returnStatus = scanSceneNodes( currentNode, path, lframe, sample, count, returnStatus );
        
        if ( MS::kSuccess != returnStatus ) continue;
      }
        // scanScene: Now deal with all the particle-instanced objects (where a
      // particle is replaced by an object or group of objects).
      //
      MItInstancer instancerIter;
      while ( !instancerIter.isDone() )
      {
        MDagPath path( instancerIter.path() );
        MString instanceStr( MString( "|INSTANCE_" ) + 
		                        instancerIter.instancerId() + MString( "_" ) +
		                        instancerIter.particleId() + MString( "_" ) +
		                        instancerIter.pathId() );
        
        MMatrix instanceMatrix( instancerIter.matrix() );
        bool useSamples( ( sample > 0 ) && isObjectMotionBlur( path ) );
        
        htable->insert( path, lframe, 
												( useSamples )? sample : 0,
											 	MRT_Unknown, count++, 
											  &instanceMatrix, instanceStr, instancerIter.particleId() );
        
        instancerIter.next();
      }
	  }
	  else //  if ( !m_renderSelected && !m_exportSpecificList )
	  {
		  MSelectionList currentSelection;
			if ( m_renderSelected )
			{
				// scanScene: find out the current selection for possible selected object output
		  	MGlobal::getActiveSelectionList( currentSelection );
			}
			else   // m_exportSpecificList = 1
			{
				for ( unsigned int i = 0; i < m_objectListToExport.length() ; i++ )
					currentSelection.add( m_objectListToExport[i] );
			}
		  MItSelectionList selIterator( currentSelection );
		  MItDag dagIterator( MItDag::kDepthFirst, MFn::kInvalid, &returnStatus);
		  for ( ; !selIterator.isDone(); selIterator.next() )
		  {
			  MDagPath objectPath;
			  selIterator.getDagPath( objectPath );
			  dagIterator.reset (objectPath.node(), MItDag::kDepthFirst, MFn::kInvalid );
			  for ( ; !dagIterator.isDone() ; dagIterator.next() )
			  {
          MDagPath path;
          dagIterator.getPath( path );
          MObject currentNode = dagIterator.item();
          if ( !currentNode.hasFn(MFn::kDagNode) ) continue;
	        
          returnStatus = scanSceneNodes( currentNode, path, lframe, sample, count, returnStatus );
          if ( MS::kSuccess != returnStatus ) continue;
        }
      }
	    // scanScene: Now deal with all the particle-instanced objects (where a
	    // particle is replaced by an object or group of objects).
	    //
	    MItInstancer instancerIter;
	    while ( !instancerIter.isDone() )
	    {
		    MDagPath path( instancerIter.path() );
		    MString instanceStr( MString( "|INSTANCE_" ) + 
		                        instancerIter.instancerId() + MString( "_" ) +
		                        instancerIter.particleId() + MString( "_" ) +
		                        instancerIter.pathId() );

		    MMatrix instanceMatrix( instancerIter.matrix() );
		    bool useSamples( ( sample > 0 ) && isObjectMotionBlur( path ) );

			  htable->insert( path, lframe, 
											 ( useSamples )? sample :	0, 
					 						 MRT_Unknown, count++, 
											 &instanceMatrix, instanceStr, instancerIter.particleId() );
			  
		    instancerIter.next();
	    }
    } //  if ( !m_renderSelected && !m_exportSpecificList )

    vector<structJob>::iterator iter = jobList.begin();
    while ( iter != jobList.end() ) 
    {
      LIQ_CHECK_CANCEL_REQUEST;
      // scanScene: Get the camera/light info for this job at this frame
      MStatus status;
			stringstream err;;

      if ( !iter->isShadow ) 
      {
        MDagPath path;
        MFnCamera   fnCamera( iter->path );
        iter->gotJobOptions = false;
        status.clear();
        MPlug cPlug = fnCamera.findPlug( MString( "ribOptions" ), &status );
        if ( status == MS::kSuccess ) 
        {
          cPlug.getValue( iter->jobOptions );
          iter->gotJobOptions = true;
        }
				getCameraInfo( fnCamera, iter->camera[sample] );
				iter->width = iter->camera[sample].width;
				iter->height = iter->camera[sample].height;

        // scanScene: Renderman specifies shutter by time open
        // so we need to convert shutterAngle to time.
        // To do this convert shutterAngle to degrees and
        // divide by 360.
        //
        iter->camera[sample].shutter = fnCamera.shutterAngle() * 0.5 / M_PI;
        liqglo_shutterTime = iter->camera[sample].shutter;
        iter->camera[sample].motionBlur     = fnCamera.isMotionBlur();

			// scanScene: The camera's fov may not match the rendered image in Maya
			// if a film-fit is used. 'fov_ratio' is used to account for
			// this.
			//
			iter->camera[sample].hFOV = fnCamera.horizontalFieldOfView()/iter->camera[sample].fov_ratio;

			if ( fnCamera.isClippingPlanes() )
      {
				iter->camera[sample].neardb    = fnCamera.nearClippingPlane();
				iter->camera[sample].fardb    = fnCamera.farClippingPlane();
			}
			else
      {
				iter->camera[sample].neardb    = 0.001;    // TODO: these values are duplicated elsewhere in this file
				iter->camera[sample].fardb    = 250000.0; // TODO: these values are duplicated elsewhere in this file
			}

			iter->camera[sample].orthoWidth     = fnCamera.orthoWidth();
			iter->camera[sample].orthoHeight    = fnCamera.orthoWidth() * ((float)iter->camera[sample].height / (float)iter->camera[sample].width);
			iter->camera[sample].focalLength    = fnCamera.focalLength();
			iter->camera[sample].focalDistance  = fnCamera.focusDistance();
			iter->camera[sample].fStop          = fnCamera.fStop();
			iter->camera[sample].isOrtho		= fnCamera.isOrtho();

			getCameraFilmOffset( fnCamera, iter->camera[sample] );
			// convert focal length to scene units
			MDistance flenDist(iter->leftCamera[sample].focalLength, MDistance::kMillimeters);
			iter->leftCamera[sample].focalLength = flenDist.as(MDistance::uiUnit());
			getCameraTransform( fnCamera, iter->camera[sample] );
			// check stereo
			MString camType = fnCamera.typeName();
			bool isStereoCamera = false;
			if ( camType == "stereoRigCamera" )
			{
				isStereoCamera = true;
				structCamera centralCameraPath = iter->camera[sample];
				// look for right and left cams
				MObject camTransform = fnCamera.parent(0, &status);
				if ( status != MS::kSuccess )
				{
					err << "Cannot find transform for camera " << fnCamera.name().asChar() << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
        }
				MFnDagNode fnCamTransform(camTransform, &status);
				if ( status != MS::kSuccess )
				{
					err << "Cannot init MFnDagNode for camera " << fnCamera.name().asChar();
					err << "'s transform." << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
        } 
				// get left one
				cPlug = fnCamTransform.findPlug( MString( "leftCam" ), &status );
				if ( status != MS::kSuccess )
        {
					err << "Cannot find plug 'leftCam' on  " << fnCamera.name().asChar() << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
				}
				MPlugArray plugArray;
				cPlug.connectedTo(plugArray, 1, 0, &status);
				if ( plugArray.length() == 0 )
        {
					err << "Nothing connected in " << fnCamTransform.name().asChar() << ".leftCam" << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;          
        }
				MPlug leftCamPlug = plugArray[0];
				MObject leftCamTransformNode = leftCamPlug.node();
				MFnTransform fnLeftTrCam ( leftCamTransformNode, &status );
				if ( status != MS::kSuccess )
				{
					err << "cannot init MFnTransfrom for left camera '" << fnCamTransform.name().asChar() << "' ..." << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
				}
				MObject leftCamNode = fnLeftTrCam.child(0);
				MFnCamera fnLeftCam( leftCamNode, &status );
				if ( status != MS::kSuccess )
				{
					err << "cannot init MFnCamera for left camera '" << fnCamTransform.name().asChar() << "' ..." << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
        }
        
				// get right one
				cPlug = fnCamTransform.findPlug( MString( "rightCam" ), &status );
				if ( status != MS::kSuccess )
				{
					err << "Cannot find plug 'rightCam' on " << fnCamTransform.name().asChar() << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
				}
				cPlug.connectedTo(plugArray, 1, 0, &status);
				if ( plugArray.length() == 0 )
				{
					err << "Nothing connected in " << fnCamTransform.name().asChar() << ".rightCam" << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure; 
				}
	
				MPlug rightCamPlug = plugArray[0];
				MObject rightCamTransformNode = rightCamPlug.node();
				MFnTransform fnRightTrCam ( rightCamTransformNode, &status );
				if ( status != MS::kSuccess )
        {
					err << "cannot init MFnTransfrom for right camera '" << rightCamPlug.name().asChar() << "' ..." << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
        } 
				MObject rightCamNode = fnRightTrCam.child(0);
				MFnCamera fnRightCam(rightCamNode, &status);
				if ( status != MS::kSuccess )
        {
					err << "cannot init MFnCamera for right camera '" << fnRightTrCam.name().asChar() << "' ..." << ends;
					liquidMessage( err.str(), messageError );
					return MS::kFailure;
        }
        
				getCameraInfo( fnLeftCam, iter->leftCamera[sample] );
				iter->leftCamera[sample].orthoWidth     = fnLeftCam.orthoWidth();
				iter->leftCamera[sample].orthoHeight    = fnLeftCam.orthoWidth() * ((float)iter->camera[sample].height / (float)iter->camera[sample].width);
				iter->leftCamera[sample].focalLength    = fnLeftCam.focalLength();
				iter->leftCamera[sample].focalDistance  = fnLeftCam.focusDistance();
				iter->leftCamera[sample].fStop          = fnLeftCam.fStop();
				iter->leftCamera[sample].isOrtho		= fnLeftCam.isOrtho();
				iter->leftCamera[sample].name			= fnLeftCam.name();
				getCameraFilmOffset( fnLeftCam, iter->leftCamera[sample] );
				// convert focal length to scene units
				MDistance flenLDist(iter->leftCamera[sample].focalLength, MDistance::kMillimeters);
				iter->leftCamera[sample].focalLength = flenLDist.as(MDistance::uiUnit());
				getCameraTransform( fnLeftCam, iter->leftCamera[sample] );
				// scanScene: The camera's fov may not match the rendered image in Maya
				// if a film-fit is used. 'fov_ratio' is used to account for
				// this.
				//
				//iter->leftCamera[sample].hFOV = fnLeftCam.horizontalFieldOfView()/iter->leftCamera[sample].fov_ratio;
				iter->leftCamera[sample].hFOV = iter->camera[sample].hFOV;
				iter->leftCamera[sample].neardb = iter->camera[sample].neardb;
				iter->leftCamera[sample].fardb = iter->camera[sample].fardb;

				getCameraInfo( fnRightCam, iter->rightCamera[sample] );
				iter->rightCamera[sample].orthoWidth	= fnRightCam.orthoWidth();
				iter->rightCamera[sample].orthoHeight	= fnRightCam.orthoWidth() * ((float)iter->camera[sample].height / (float)iter->camera[sample].width);
				iter->rightCamera[sample].focalLength	= fnRightCam.focalLength();
				iter->rightCamera[sample].focalDistance	= fnRightCam.focusDistance();
				iter->rightCamera[sample].fStop			= fnRightCam.fStop();
				iter->rightCamera[sample].isOrtho		= fnRightCam.isOrtho();
				iter->rightCamera[sample].name			= fnRightCam.name();
				getCameraFilmOffset( fnRightCam, iter->rightCamera[sample] );
				// convert focal length to scene units
				MDistance flenRDist(iter->rightCamera[sample].focalLength, MDistance::kMillimeters);
				iter->rightCamera[sample].focalLength = flenRDist.as(MDistance::uiUnit());
				getCameraTransform( fnRightCam, iter->rightCamera[sample] );
        // scanScene: The camera's fov may not match the rendered image in Maya
        // if a film-fit is used. 'fov_ratio' is used to account for
        // this.
        //
				//iter->rightCamera[sample].hFOV = fnRightCam.horizontalFieldOfView()/iter->rightCamera[sample].fov_ratio;
				iter->rightCamera[sample].hFOV = iter->camera[sample].hFOV;
				iter->rightCamera[sample].neardb = iter->camera[sample].neardb;
				iter->rightCamera[sample].fardb = iter->camera[sample].fardb;
				iter->camera[sample].rightCam = &(iter->rightCamera[sample]);
				iter->camera[sample].leftCam = &(iter->leftCamera[sample]);
			}
			//else
			//{
			//}
  			iter->isStereoPass = isStereoCamera;
        iter->aspectRatio = aspectRatio;
  
        // scanScene: Determine what information to write out (RGB, alpha, zbuffer)
        //
        iter->imageMode.clear();
  
        bool isOn;
        MPlug boolPlug;
        boolPlug = fnCamera.findPlug( "image" );
  
        boolPlug.getValue( isOn );
        if ( isOn ) 
        {
          // We are writing RGB info
          //
          iter->imageMode = "rgb";
          iter->format = outFormat;
        }
        boolPlug = fnCamera.findPlug( "mask" );
        boolPlug.getValue( isOn );
        if ( isOn ) 
        {
          // We are writing alpha channel info
          //
          iter->imageMode += "a";
          iter->format = outFormat;
        }
        boolPlug = fnCamera.findPlug( "depth" );
        boolPlug.getValue( isOn );
  			if ( isOn )
  			{
          if ( !isStereoCamera  )
          { // We are writing z-buffer info
            //
            iter->imageMode = "z";
            iter->format = "zfile";
          }
  			  else
  				  liquidMessage( "Cannot render depth for stereo camera.", messageWarning );
  			} // isOn && !isStereoCamera
      } 
      else 
      {
        // scanScene: doing shadow render
        //
				MDagPath lightPath;
        MFnLight   fnLight( iter->path );
        status.clear();

        iter->gotJobOptions = false;
        if ( liquidGetPlugValue( fnLight, "ribOptions", iter->jobOptions, status ) ==  MS::kSuccess )
          iter->gotJobOptions = true;  
				iter->isStereoPass = false;

        // philippe: this block is obsolete as we now get the resolution when building the job list
        //
        /* MPlug lightPlug = fnLight.findPlug( "dmapResolution" );
        float dmapSize;
        lightPlug.getValue( dmapSize );
        iter->height = iter->width = (int)dmapSize; */

        if ( iter->hasShadowCam ) 
        {
          // scanScene: the light uses a shadow cam
          //
          
					MFnCamera fnCamera( iter->shadowCamPath );
					fnCamera.getPath(lightPath);
					MTransformationMatrix xform( lightPath.inclusiveMatrix() );

          cerr << "dbg> scanScene: the light uses a shadow cam " <<  lightPath.fullPathName() << endl;

          // the camera is pointing toward negative Z
          double scale[] = { 1, 1, -1 };
          xform.setScale( scale, MSpace::kTransform );

          iter->camera[sample].mat         = xform.asMatrixInverse();
          iter->camera[sample].neardb      = fnCamera.nearClippingPlane();
          iter->camera[sample].fardb       = fnCamera.farClippingPlane();
          iter->camera[sample].isOrtho     = fnCamera.isOrtho();
          iter->camera[sample].orthoWidth  = fnCamera.orthoWidth();
          iter->camera[sample].orthoHeight = fnCamera.orthoWidth();
        } 
        else // iter->hasShadowCam 
        {
          // scanScene: the light does not use a shadow cam
          //

          // get the camera world matrix
					fnLight.getPath(lightPath);
					MTransformationMatrix xform( lightPath.inclusiveMatrix() );

          // the camera is pointing toward negative Z
          double scale[] = { 1, 1, -1 };
          xform.setScale( scale, MSpace::kTransform );

          if ( iter->isPoint ) 
          {
            double ninty = M_PI/2;
            if( iter->pointDir == pPX ) { double rotation[] = { 0, -ninty, 0 }; xform.setRotation( rotation, MTransformationMatrix::kXYZ ); }
            if( iter->pointDir == pNX ) { double rotation[] = { 0, ninty, 0 }; xform.setRotation( rotation, MTransformationMatrix::kXYZ ); }
            if( iter->pointDir == pPY ) { double rotation[] = { ninty, 0, 0 }; xform.setRotation( rotation, MTransformationMatrix::kXYZ ); }
            if( iter->pointDir == pNY ) { double rotation[] = { -ninty, 0, 0 }; xform.setRotation( rotation, MTransformationMatrix::kXYZ ); }
            if( iter->pointDir == pPZ ) { double rotation[] = { 0, M_PI, 0 }; xform.setRotation( rotation, MTransformationMatrix::kXYZ ); }
            if( iter->pointDir == pNZ ) { double rotation[] = { 0, 0, 0 }; xform.setRotation( rotation, MTransformationMatrix::kXYZ ); }
          }
          iter->camera[sample].mat = xform.asMatrixInverse();

          MPlug shaderConnection( fnLight.findPlug( "liquidLightShaderNode", &status ) );
          if ( status == MS::kSuccess && shaderConnection.isConnected() ) 
          {
            MPlugArray LightShaderPlugArray;
            shaderConnection.connectedTo( LightShaderPlugArray, true, true );
            MFnDependencyNode fnLightShaderNode( LightShaderPlugArray[0].node() );
            fnLightShaderNode.findPlug( "nearClipPlane" ).getValue( iter->camera[sample].neardb );
            fnLightShaderNode.findPlug( "farClipPlane" ).getValue( iter->camera[sample].fardb );
          } 
          else 
          {
            iter->camera[sample].neardb   = 0.001;    // TODO: these values are duplicated elsewhere in this file
            iter->camera[sample].fardb    = 250000.0; // TODO: these values are duplicated elsewhere in this file
            liquidGetPlugValue( fnLight, "nearClipPlane", iter->camera[sample].neardb, status );
            liquidGetPlugValue( fnLight, "farClipPlane", iter->camera[sample].fardb, status );
          }

          if ( fnLight.dagPath().hasFn( MFn::kDirectionalLight ) ) 
          {
            iter->camera[sample].isOrtho = true;
            liquidGetPlugValue( fnLight, "dmapWidthFocus", iter->camera[sample].orthoWidth, status );
            liquidGetPlugValue( fnLight, "dmapWidthFocus", iter->camera[sample].orthoHeight, status );
          } 
          else 
          {
            iter->camera[sample].isOrtho = false;
            iter->camera[sample].orthoWidth = 0.0;
          }
        } // iter->hasShadowCam

        if ( iter->deepShadows )
        {
          iter->camera[sample].shutter = liqglo_shutterTime;
          iter->camera[sample].motionBlur = true;
        }
        else // iter->deepShadows
        {
          iter->camera[sample].shutter = 0;
          iter->camera[sample].motionBlur = false;
        } // iter->deepShadows
        iter->camera[sample].focalLength = 0;
        iter->camera[sample].focalDistance = 0;
        iter->camera[sample].fStop = 0;
        //doCameraMotion = 0;

        iter->aspectRatio = 1.0;

        // The camera's fov may not match the rendered image in Maya
        // if a film-fit is used. 'fov_ratio' is used to account for
        // this.
        //
        if ( iter->hasShadowCam ) 
        {
          MFnCamera fnCamera( iter->shadowCamPath );
          float camFov = fnCamera.horizontalFieldOfView();
          iter->camera[sample].hFOV = camFov;
        } 
        else // iter->hasShadowCam 
        {
          float angle = 0, penumbra = 0;
          if ( liquidGetPlugValue( fnLight, "coneAngle", angle, status ) ==  MS::kSuccess )
          {
            liquidGetPlugValue( fnLight, "penumbraAngle", penumbra, status );
            if ( penumbra > 0 ) angle += penumbra * 2;
            iter->camera[sample].hFOV = angle;
          }
          else 
            iter->camera[sample].hFOV = 95;
        } // iter->hasShadowCam
        /*
        // Determine what information to write out ( depth map or deep shadow )
        //
        iter->imageMode.clear();
        if ( iter->deepShadows )
        {
          iter->imageMode    += liquidRenderer.dshImageMode;        //"deepopacity";
          iter->format        =  liquidRenderer.dshDisplayName;    //"deepshad";
        }
        else
        {
          iter->imageMode += "z";
          iter->format = "shadow";
        }
        */
      }
      ++iter;
    } // while ( iter != jobList.end() ) 
    // post-frame script execution
    if ( m_postFrameMel != "" ) 
    {
      MString postFrameMel( parseString( m_postFrameMel, false ) );
	    if ( fileExists( postFrameMel  ) ) MGlobal::sourceFile( postFrameMel );
      else 
        if ( MS::kSuccess == MGlobal::executeCommand( postFrameMel, false, false ) ) 
          liquidMessage( "post-frame script executed successfully.", messageInfo );
        else 
          liquidMessage( "post-frame script failed.", messageError );
    }
    return MS::kSuccess;
  } // if ( MGlobal::viewFrame( mt ) == MS::kSuccess ) 
  return MS::kFailure;
}

/**
 * This method takes care of the blocking together of objects and their children in the DAG.
 * This method compares two DAG paths and figures out how many attribute levels to push and/or pop.
 * The intention seems clear but this method is not currently used -- anyone cares to comment? --Moritz.
 */
void liqRibTranslator::doAttributeBlocking( const MDagPath& newPath, const MDagPath& previousPath )
{
  int newDepth = newPath.length();
  int prevDepth = 0;
  MFnDagNode dagFn( newPath );
  MDagPath npath = newPath;
  MDagPath ppath = previousPath;

  if ( previousPath.isValid() ) 
  {
    // Recursive base case
    // If the paths are the same, then we don't have to write
    // any start/end attribute blocks.  So, just return
    //
    if ( newPath == previousPath ) return;
    prevDepth = previousPath.length();
    // End attribute block if necessary
    //
    if ( newDepth <= prevDepth ) 
    {
      // Write an attribute block end
      //
      RiAttributeEnd();
      attributeDepth--;
      if ( prevDepth > 1 ) ppath.pop();
    }
  }
  if ( ( newDepth >= prevDepth ) && ( newDepth > 1 ) ) npath.pop();
  // Recurse and process parents
  //
  if ( ( prevDepth > 1 ) || ( newDepth > 1 ) ) doAttributeBlocking( npath, ppath ); // Recurse
  // Write open for new attribute block if necessary
  //
  if ( newDepth >= prevDepth ) 
  {
    MString name( dagFn.name() );

    RiAttributeBegin();
    RiAttribute( "identifier", "name", &getLiquidRibName( name.asChar() ), RI_NULL );

    if ( newPath.hasFn( MFn::kTransform ) ) 
    {
      // We have a transform, so write out the info
      //
      RtMatrix ribMatrix;
      MObject transform = newPath.node();
      MFnTransform transFn( transform );
      MTransformationMatrix localTransformMatrix = transFn.transformation();
      MMatrix localMatrix = localTransformMatrix.asMatrix();
      localMatrix.get( ribMatrix );
      RiConcatTransform( ribMatrix );
    }
    attributeDepth++;
  }
}

/**
 * Write out the frame prologue.
 * This includes all pass-dependant options like shading interpolation, hider,
 * display driver and the camera transformation.
 */
MStatus liqRibTranslator::framePrologue( long lframe )
{
	LIQDEBUGPRINTF( "-> Beginning Frame Prologue\n" );
	ribStatus = kRibFrame;

	if ( !m_exportReadArchive )
	{
		RiFrameBegin( lframe );

		if ( liqglo_currentJob.isShadow == false && liqglo_rotateCamera  == true )
			// philippe : Rotated Camera Case
			RiFormat( liqglo_currentJob.height, liqglo_currentJob.width, liqglo_currentJob.aspectRatio );
		else
			RiFormat( liqglo_currentJob.width, liqglo_currentJob.height, liqglo_currentJob.aspectRatio );
		if ( !liqglo_currentJob.isShadow )
		{
			// Smooth Shading
			RiShadingInterpolation( "smooth" );
			// Quantization
			// overriden to floats when in rendering to Maya's renderView
			if ( !m_renderView && quantValue != 0 )
			{
				int whiteValue = (int) pow( 2.0, quantValue ) - 1;
				RiQuantize( RI_RGBA, whiteValue, 0, whiteValue, 0.5 );
			}
			else
				RiQuantize( RI_RGBA, 0, 0, 0, 0 );
			if ( m_rgain != 1.0 || m_rgamma != 1.0 ) RiExposure( m_rgain, m_rgamma );
		}
		if ( liqglo_currentJob.isShadow &&
			 ( !liqglo_currentJob.deepShadows ||
			   liqglo_currentJob.shadowPixelSamples == 1 ) )
		{
			if ( liquidRenderer.renderName == MString("Pixie") )
			{
				RtFloat zero = 0;
				RiHider( "hidden", "jitter", &zero, RI_NULL );
			}
			else
			{
				RtInt zero = 0;
				RiHider( "hidden", "int jitter", &zero, RI_NULL );
			}
		}
		if ( liqglo_currentJob.isShadow && liqglo_currentJob.isMidPointShadow && !liqglo_currentJob.deepShadows )
		{
			RtString midPoint = "midpoint";
			RtFloat midRatio = liqglo_currentJob.midPointRatio;

			RiHider( "hidden", "depthfilter", &midPoint, RI_NULL );
			
			if ( liqglo_currentJob.midPointRatio != 0 )
				RiHider( "hidden", "midpointratio", &midRatio, RI_NULL ); // Output to rib jami
		}

		LIQDEBUGPRINTF( "-> Setting Display Options\n" );
		if ( liqglo_currentJob.isShadow )
		{
			//MString relativeShadowName( liquidSanitizePath( liquidGetRelativePath( liqglo_relativeFileNames, liqglo_currentJob.imageName, liqglo_projectDir ) ) );
			if ( !liqglo_currentJob.isMinMaxShadow )
			{
				if ( liqglo_currentJob.deepShadows )
				{
					// RiDeclare( "volumeinterpretation", "string" );
					RtString viContinuous = "continuous";
					RtString viDiscrete   = "discrete";
					
					if ( liquidRenderer.renderName == MString("3Delight") )
					{
						RiDisplay( const_cast< char* >( liqglo_currentJob.imageName.asChar()),
							const_cast< char* >( liqglo_currentJob.format.asChar() ),
							(RtToken)liqglo_currentJob.imageMode.asChar(),
							"string volumeinterpretation",
							( liqglo_currentJob.shadowVolumeInterpretation == 1 ? &viContinuous : &viDiscrete ),
							RI_NULL );
					}
					else
					{
						// Deep shadows cannot be the primary output driver in PRMan & co.
						// We need to create a null output zfile first, and use the deep
						// shadows as a secondary output.
						//
						if ( liquidRenderer.renderName != MString("Pixie") ) RiDisplay( "null", "null", "z", RI_NULL );
						
						MString deepFileImageName = "+" + liqglo_currentJob.imageName;
					
						RiDisplay( const_cast< char* >( deepFileImageName.asChar() ),
							const_cast< char* >( liqglo_currentJob.format.asChar() ),
							(RtToken)liqglo_currentJob.imageMode.asChar(),
							"string volumeinterpretation",
							( liqglo_currentJob.shadowVolumeInterpretation == 1 ? &viContinuous : &viDiscrete ),
							RI_NULL );
					}
				}
				else
				{
					RtInt aggregate( liqglo_currentJob.shadowAggregation );
					RiDisplay( const_cast< char* >( liqglo_currentJob.imageName.asChar() ),
						const_cast< char* >( liqglo_currentJob.format.asChar() ),
						(RtToken)liqglo_currentJob.imageMode.asChar(),
						"int aggregate", &aggregate,
						RI_NULL );
				}
			}
			else
			{
				RiArchiveRecord( RI_COMMENT, "Display Driver:" );
				RtInt minmax = 1;
				RiDisplay( const_cast< char* >( liqglo_currentJob.imageName.asChar() ),
					const_cast< char* >(liqglo_currentJob.format.asChar()),
					(RtToken)liqglo_currentJob.imageMode.asChar(),
					"minmax", &minmax,
					RI_NULL );
			}
      exportJobCamera( liqglo_currentJob, liqglo_currentJob.camera );
		}
	  else
	  {
      if ( ( m_cropX1 != 0.0 ) || ( m_cropY1 != 0.0 ) || ( m_cropX2 != 1.0 ) || ( m_cropY2 != 1.0 ) ) 
      {
        // philippe : handle the rotated camera case
        if ( liqglo_rotateCamera == true ) RiCropWindow( m_cropY2, m_cropY1, 1 - m_cropX1, 1 - m_cropX2 );
        else                               RiCropWindow( m_cropX1, m_cropX2, m_cropY1, m_cropY2 );
      }
			if ( !liqglo_currentJob.isStereoPass ) 
        exportJobCamera( liqglo_currentJob, liqglo_currentJob.camera );
			else
			{
				// export right camera
				RiTransformBegin();
				exportJobCamera( liqglo_currentJob, liqglo_currentJob.rightCamera );
				RiCameraV( "right", 0, (char**)RI_NULL, (void**)RI_NULL );
				RiTransformEnd();
				// export left camera
				exportJobCamera( liqglo_currentJob, liqglo_currentJob.leftCamera );
			}
      // display channels
      if ( liquidRenderer.supports_DISPLAY_CHANNELS ) 
      {
        RiArchiveRecord( RI_COMMENT, "Display Channels:" );
        // philippe -> to do : move this to higher scope ?
        //MStringArray channeltype;
        //channeltype.append( "float" );
        //channeltype.append( "color" );
        //channeltype.append( "point" );
        //channeltype.append( "normal" );
        //channeltype.append( "vector" );
        string  channeltype[] = { "float", "color", "point", "normal", "vector" };

        vector<structChannel>::iterator m_channels_iterator;
        m_channels_iterator = m_channels.begin();

				bool isCiDeclared = 0;
				bool isADeclared = 0;
        while ( m_channels_iterator != m_channels.end() ) 
        {
          int       numTokens = 0;
          RtToken   tokens[5];
          RtPointer values[5];

          //MString channel;
          stringstream channel;
          char* filter;
          int quantize[4];
          float filterwidth[2];
          float dither;
          
// #if defined( GENERIC_RIBLIB )          
          stringstream quantize_str;
          stringstream dither_str;
          stringstream filter_str;
// #endif
          channel << channeltype[m_channels_iterator->type];
          if ( m_channels_iterator->arraySize > 0 ) 
            channel << "[" << m_channels_iterator->arraySize << "]";
          channel << " " << m_channels_iterator->name.asChar();

          if ( m_channels_iterator->quantize ) 
          {
            int max = ( int )pow( 2., m_channels_iterator->bitDepth ) - 1;
            dither = m_channels_iterator->dither;
            quantize[0] = quantize[2] = 0;
            quantize[1] = quantize[3] = max;
            tokens[ numTokens ] = "int[4] quantize";
            values[ numTokens ] = (RtPointer)quantize;
            numTokens++;
#if !defined( GENERIC_RIBLIB )               
          }
#else
             quantize_str << "\"int[4] quantize\" [ 0 " << max << " 0 " << max << " ]";
             dither_str << "\"float dither\" [" << dither << "]";
          }
          else
          {
            quantize_str << "\"int[4] quantize\" [ 0 0 0 0 ]"; 
            dither_str.clear();
          }
#endif
          
          if ( m_channels_iterator->filter ) 
          {
            MString pixFilter( liquidRenderer.pixelFilterNames[ m_channels_iterator->pixelFilter ] );
            filter = ( char* )pixFilter.asChar();
            
            tokens[ numTokens ] = "string filter";
            values[ numTokens ] = ( RtPointer )&filter;
            numTokens++;

            filterwidth[0] = m_channels_iterator->pixelFilterX;
            filterwidth[1] = m_channels_iterator->pixelFilterY;
            tokens[ numTokens ] = "float filterwidth[2]";
            values[ numTokens ] = ( RtPointer )filterwidth;
            numTokens++;
#if !defined( GENERIC_RIBLIB )               
          }
#else
             filter_str << "\"string filter\" [\"" << pixFilter.asChar() << "\"] \"float filterwidth[2]\" [" << filterwidth[0] << " " << filterwidth[1] << "]";
          } 
          else
            filter_str.clear();
#endif
          
#if defined ( DELIGHT ) ||  defined ( PRMAN ) || defined (PIXIE)
          //if( liquidRenderer.renderName == MString("PRMan") )
          RiDisplayChannelV( ( RtToken )channel.str().c_str(), numTokens, tokens, values );
					if ( channel.str().c_str() == "color Ci" ) 
            isCiDeclared = 1;
					else if ( channel.str().c_str() == "float a" ) 
            isADeclared = 1;
#else
  // defined ( GENERIC_RIBLIB ) ||
   RiArchiveRecord( RI_VERBATIM, "DisplayChannel \"%s\" %s %s %s", 
   const_cast< char* >( channel.str().c_str()), quantize_str.str().c_str(), dither_str.str().c_str(), filter_str.str().c_str() );
        
#endif
          m_channels_iterator++;
        }
#if defined ( DELIGHT ) || defined ( GENERIC_RIBLIB ) || defined ( PRMAN ) || defined (PIXIE)
				if ( m_isStereoCamera && !liqglo_currentJob.isShadow )
				{
					RtToken   *emptyTokens = NULL;
					RtPointer *emptyValues = NULL;
					if ( !isCiDeclared ) RiDisplayChannelV( ( RtToken )"color Ci", 0, emptyTokens, emptyValues );
					if ( !isADeclared ) RiDisplayChannelV( ( RtToken )"float a", 0, emptyTokens, emptyValues );
				}
#endif
      }
      // output display drivers
      RiArchiveRecord( RI_COMMENT, "Display Drivers:" );

      vector<structDisplay>::iterator m_displays_iterator;
      m_displays_iterator = m_displays.begin();
     
			// create right display for stereo
			if ( m_isStereoCamera )
			{
				structDisplay rightStereoAov;
				rightStereoAov.name = m_displays_iterator->name;
				rightStereoAov.name = m_displays_iterator->name;
				rightStereoAov.mode = "Ci,a";
				if ( m_displays_iterator->type == "it" )
					rightStereoAov.type = "tiff"; // should be optionnal ?....
				else
					rightStereoAov.type = m_displays_iterator->type;
				rightStereoAov.enabled = m_displays_iterator->enabled;
				rightStereoAov.doQuantize = m_displays_iterator->doQuantize;
				rightStereoAov.bitDepth = m_displays_iterator->bitDepth;
				rightStereoAov.dither = m_displays_iterator->dither;
				rightStereoAov.doFilter = m_displays_iterator->doFilter;
				rightStereoAov.filter = m_displays_iterator->filter;
				rightStereoAov.filterX = m_displays_iterator->filterX;
				rightStereoAov.filterY = m_displays_iterator->filterY;
				rightStereoAov.xtraParams = m_displays_iterator->xtraParams;
				rightStereoAov.xtraParams.num = rightStereoAov.xtraParams.num + 1;
				rightStereoAov.xtraParams.names.append("camera");
				rightStereoAov.xtraParams.data.append("right");
				rightStereoAov.xtraParams.type.append(0);  // string
				m_displays_iterator ++;
				m_displays.insert( m_displays_iterator, rightStereoAov );
				// replace iter at beginning
				m_displays_iterator = m_displays.begin();
			}
      string paramType[] = { "string ", "float ",  "int " };

      while ( m_displays_iterator != m_displays.end() ) 
      {
        stringstream parameterString;
        stringstream imageName;
        string imageType;
        string imageMode;
        stringstream quantizer;
        stringstream dither;
        stringstream filter;
        // check if additionnal displays are enabled
        // if not, call it off after the 1st iteration.
        if ( m_ignoreAOVDisplays && m_displays_iterator > m_displays.begin() ) 
          break;

        // This is the override for the primary DD
        // when you render to maya's renderview.
        if ( m_displays_iterator == m_displays.begin() && m_renderView ) 
        {
          imageName << generateImageName( "", liqglo_currentJob, liqglo_currentJob.format );
          
          // TODO: It doesn't work on windoze...
          //MString host = "localhost";
          //if( !m_renderViewLocal ) 
          //  MGlobal::executeCommand( "strip(system(\"echo $HOST\"));", host );

          RiArchiveRecord( RI_COMMENT, "Render To Maya renderView :" );
          RiArchiveRecord( RI_VERBATIM, "Display \"%s\" \"%s\" \"%s\" \"int merge\" [0] \"int mayaDisplayPort\" [%d] \"string host\" [\"%s\"]\n", 
          const_cast< char* >( imageName.str().c_str() ), "liqmaya", "rgba", m_renderViewPort, "localhost" );

          // in this case, override the launch render settings
          if ( launchRender == false ) 
            launchRender = true;
        } 
        else 
        {
          // check if display is enabled
          if ( !(*m_displays_iterator).enabled ) 
          {
            m_displays_iterator++;
            continue;
          }
/*
					// get display name
					// defaults to scenename.0001.tif if left empty
					imageName = (*m_displays_iterator).name;
					if( imageName == "" )
					{
						if( m_isStereoCamera && m_displays_iterator == m_displays.begin() )
						{
							imageName = liqglo_sceneName + ".left.#." + outExt;
						}
						else if( m_isStereoCamera )
						{
							imageName = liqglo_sceneName + ".right.#." + outExt;
						}
						else
						{
							imageName = liqglo_sceneName + ".#." + outExt;
						}
					}
					imageName = m_pixDir + parseString( imageName, false );
*/
          // we test for an absolute path before converting from rel to abs path in case the picture dir was overriden through the command line.
          //if( m_pixDir.index( '/' ) != 0 ) imageName = liquidGetRelativePath( liqglo_relativeFileNames, imageName, liqglo_projectDir );
          // get display type ( tiff, openexr, etc )
          
          imageType = ((*m_displays_iterator).type == "")? "framebuffer" : (*m_displays_iterator).type.asChar();
          // get display mode ( rgb, z or defined display channel )
          imageMode = ( (*m_displays_iterator).mode == "")? "rgba" : (*m_displays_iterator).mode.asChar();
          
          if ( m_displays_iterator == m_displays.begin() )
          {
            liqglo_currentJob.format = MString( imageType.c_str() );
            imageName << generateImageName( "", liqglo_currentJob, liqglo_currentJob.format );  
          }
          else
            imageName << "+" << generateImageName( (*m_displays_iterator).name, liqglo_currentJob, MString( imageType.c_str()) ) ;
          
          
          // get quantization params
          if ( (*m_displays_iterator).doQuantize && m_displays_iterator > m_displays.begin() ) 
          {
            if ( (*m_displays_iterator).bitDepth != 0 ) 
            {
            	int max = (int) pow( 2.0, (*m_displays_iterator).bitDepth ) - 1;
              quantizer << "\"float quantize[4]\" [ 0 " << max << " 0 " << max << " ]";
            } 
            else 
              quantizer << "\"float quantize[4]\" [ 0 0 0 0 ]";
            dither << "\"float dither\" [" << (*m_displays_iterator).dither <<"]";
          } 
          else 
          {
            quantizer.clear();
            dither.clear();
          }

          // get filter params
          if ( (*m_displays_iterator).doFilter && m_displays_iterator > m_displays.begin() )
          {
            filter << "\"string filter\" [\"" << liquidRenderer.pixelFilterNames[(*m_displays_iterator).filter].asChar() << "\"] ";
            filter << "\"float filterwidth[2]\" ";
            filter << "[" << (*m_displays_iterator).filterX << " " << (*m_displays_iterator).filterY << "]";
          } 
          else 
            filter.clear();

          // display driver specific arguments
          parameterString.clear();
          for ( int p = 0; p < (*m_displays_iterator).xtraParams.num; p++ ) 
          {
            parameterString << "\"";
            parameterString << paramType[ (*m_displays_iterator).xtraParams.type[p] ];
            parameterString << (*m_displays_iterator).xtraParams.names[p].asChar();
            parameterString << "\" [";
            parameterString << ( ((*m_displays_iterator).xtraParams.type[p] > 0)? "" : "\"" );
            parameterString << (*m_displays_iterator).xtraParams.data[p].asChar();
            parameterString << ( ((*m_displays_iterator).xtraParams.type[p] > 0)? "] " : "\"] ");
          }
          
          // output call
          RiArchiveRecord( RI_VERBATIM, "Display \"%s\" \"%s\" \"%s\" %s %s %s %s\n", const_cast< char* >( imageName.str().c_str() ), 
          imageType.c_str(), 
          imageMode.c_str(), 
          quantizer.str().c_str(), 
          dither.str().c_str(), 
          filter.str().c_str(), 
          parameterString.str().c_str() );
        }
        m_displays_iterator++;
      }
    }

    LIQDEBUGPRINTF( "-> Setting Resolution\n" );
        
    // Set up for camera motion blur
    /* doCameraMotion = liqglo_currentJob.camera[0].motionBlur && liqglo_doMotion; */
    float frameOffset = 0;
    if ( doCameraMotion || liqglo_doMotion || liqglo_doDef ) 
    {
      switch ( shutterConfig ) 
      {
        case OPEN_ON_FRAME:
        default:
          if ( liqglo_relativeMotion ) RiShutter( 0, liqglo_currentJob.camera[0].shutter );
          else                         RiShutter( lframe, lframe + liqglo_currentJob.camera[0].shutter );
          frameOffset = lframe;
          break;
          
        case CENTER_ON_FRAME:
          if ( liqglo_relativeMotion ) RiShutter(  - ( liqglo_currentJob.camera[0].shutter * 0.5 ),  
																										 ( liqglo_currentJob.camera[0].shutter * 0.5 ) );
          else                         RiShutter( ( lframe - ( liqglo_currentJob.camera[0].shutter * 0.5 ) ), 
																									( lframe + ( liqglo_currentJob.camera[0].shutter * 0.5 ) ) );
          frameOffset = lframe - ( liqglo_currentJob.camera[0].shutter * 0.5 );
          break;
          
        case CENTER_BETWEEN_FRAMES:
          if ( liqglo_relativeMotion ) RiShutter( + ( 0.5 * ( 1 - liqglo_currentJob.camera[0].shutter ) ), 
															                    + liqglo_currentJob.camera[0].shutter 
                                                  + ( 0.5 * ( 1 - liqglo_currentJob.camera[0].shutter ) ) );
          else                         RiShutter( lframe + ( 0.5 * ( 1 - liqglo_currentJob.camera[0].shutter ) ), 
                                                  lframe + liqglo_currentJob.camera[0].shutter 
																									+ ( 0.5 * ( 1 - liqglo_currentJob.camera[0].shutter ) ) );
          frameOffset = lframe + ( 0.5 * ( 1 - liqglo_currentJob.camera[0].shutter ) );
          break;
          
        case CLOSE_ON_NEXT_FRAME:
          if ( liqglo_relativeMotion ) RiShutter( + ( 1 - liqglo_currentJob.camera[0].shutter ),  1 );
          else                         RiShutter( lframe + ( 1 - liqglo_currentJob.camera[0].shutter ), lframe + 1 );
          frameOffset = lframe + ( 1 - liqglo_currentJob.camera[0].shutter );
          break;
      }
    } 
    else 
    {
      if ( liqglo_relativeMotion ) RiShutter( 0, 0);
      else                         RiShutter( lframe, lframe );
      frameOffset = lframe;
    }
    // relative motion
    if ( liqglo_relativeMotion ) RiOption( "shutter", "offset", &frameOffset, RI_NULL);

#ifdef DELIGHT
    RiOption( "shutter", "efficiency", &liqglo_shutterEfficiency, RI_NULL );
#endif
    if ( liqglo_currentJob.gotJobOptions ) 
      RiArchiveRecord( RI_COMMENT, "jobOptions: \n%s", liqglo_currentJob.jobOptions.asChar() );
    
    if ( ( liqglo_preRibBox.length() > 0 ) && !liqglo_currentJob.isShadow ) 
      for ( unsigned ii(0); ii < liqglo_preRibBox.length(); ii++ ) 
        RiArchiveRecord( RI_COMMENT, "Additional Rib:\n%s", liqglo_preRibBox[ii].asChar() );
    
    if ( ( liqglo_preReadArchive.length() > 0 ) && !liqglo_currentJob.isShadow ) 
      for ( unsigned ii(0); ii < liqglo_preReadArchive.length(); ii++ ) 
        RiArchiveRecord( RI_COMMENT, "Read Archive Data: \nReadArchive \"%s\"", liqglo_preReadArchive[ii].asChar() );

    if ( ( liqglo_preRibBoxShadow.length() > 0 ) && !liqglo_currentJob.isShadow ) 
      for ( unsigned ii(0); ii < liqglo_preRibBoxShadow.length(); ii++ ) 
        RiArchiveRecord( RI_COMMENT, "Additional Rib:\n%s", liqglo_preRibBoxShadow[ii].asChar() );

    if ( ( liqglo_preReadArchiveShadow.length() > 0 ) && liqglo_currentJob.isShadow ) 
      for ( unsigned ii(0); ii < liqglo_preReadArchiveShadow.length(); ii++ ) 
        RiArchiveRecord( RI_COMMENT, "Read Archive Data: \nReadArchive \"%s\"", liqglo_preReadArchiveShadow[ii].asChar() );

  }
  return MS::kSuccess;
}

/**
 * Write out the frame epilogue.
 * This basically calls RiFrameEnd().
 */
MStatus liqRibTranslator::frameEpilogue( long )
{
  if ( ribStatus == kRibFrame ) 
  {
    ribStatus = kRibBegin;
    if ( !m_exportReadArchive ) 
      RiFrameEnd();
  }
  return (ribStatus == kRibBegin ? MS::kSuccess : MS::kFailure);
}

/**
 * Write out the body of the frame.
 * This is a dump of the DAG to RIB with flattened transforms (MtoR-style).
 */
MStatus liqRibTranslator::objectBlock()
{
  MStatus returnStatus = MS::kSuccess;
  MStatus status;
  attributeDepth = 0;
  
  LIQDEBUGPRINTF( "-> objectBlock\n" );

  if ( m_ignoreSurfaces && !liqglo_skipDefaultMatte ) RiSurface( "matte", RI_NULL );

	// Moritz: Added Pre-Geometry RIB for insertion right before any primitives
	MFnDependencyNode globalsNode( rGlobalObj );
	MPlug prePostplug( globalsNode.findPlug( "preGeomMel" ) );
	MString melcommand( prePostplug.asString() );
	if ( m_preGeomRIB != "" || melcommand.length() )
	{
		RiArchiveRecord( RI_COMMENT,  " Pre-Geometry RIB from liquid globals");
		MGlobal::executeCommand( melcommand );
		RiArchiveRecord( RI_VERBATIM, ( char* ) m_preGeomRIB.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n");
	}

  // retrieve the shadow set object
  MObject shadowSetObj;
  if ( liqglo_currentJob.isShadow && liqglo_currentJob.shadowObjectSet != "" ) 
  {
    MObject tmp;
    tmp = getNodeByName( liqglo_currentJob.shadowObjectSet, &status );
    if ( status == MS::kSuccess ) shadowSetObj = tmp;
    else 
    {
   		stringstream warn;   
			warn << "Liquid : set " <<  liqglo_currentJob.shadowObjectSet.asChar();
			warn << " in shadow " << liqglo_currentJob.name.asChar() << " does not exist !" << ends;;
      liquidMessage( warn.str(), messageWarning );
    }
    status.clear();
  }
  MFnSet shadowSet( shadowSetObj, &status );

  MMatrix matrix;
  MDagPath path;
  MObject transform;
  MFnDagNode dagFn;

  for ( RNMAP::iterator rniter( htable->RibNodeMap.begin() ); rniter != htable->RibNodeMap.end(); rniter++ ) 
  {
    LIQ_CHECK_CANCEL_REQUEST;

    liqRibNodePtr ribNode( rniter->second );
    path = ribNode->path();
    transform = path.transform();

    if ( ( !ribNode ) || ( ribNode->object(0)->type == MRT_Light ) ) continue;
    if ( ribNode->object(0)->type == MRT_Coord || ribNode->object(0)->type == MRT_ClipPlane ) continue;
    if ( ( !liqglo_currentJob.isShadow ) && ( ribNode->object(0)->ignore ) ) continue;
    if ( ( liqglo_currentJob.isShadow ) && ( ribNode->object(0)->ignoreShadow ) ) continue;
    // test against the set
    if ( ( liqglo_currentJob.isShadow ) && ( liqglo_currentJob.shadowObjectSet != "" ) && 
			   ( !shadowSetObj.isNull() ) && ( !shadowSet.isMember( transform, &status ) ) ) 
    {
      //cout <<"SET FILTER : object "<<ribNode->name.asChar()<<" is NOT in "<<liqglo_currentJob.shadowObjectSet.asChar()<<endl;
      continue;
    }
    // if ( ribNode->object( 1 ) == 0 ) cout << ">> ribNode->object( 1 ) == 0 for " <<  ribNode->name << endl;     

	  if ( m_outputComments ) RiArchiveRecord( RI_COMMENT, "Name: %s", ribNode->name.asChar(), RI_NULL );

    RiAttributeBegin();
    RiAttribute( "identifier", "name", &getLiquidRibName( ribNode->name.asChar() ), RI_NULL );

 	  // Alf: preTransformMel
	  MFnDagNode fnTransform( transform );
	  MPlug prePostPlug = fnTransform.findPlug( "liqPreTransformMel" );
	  m_preTransformMel = prePostPlug.asString();
	  
    if ( m_preTransformMel != "" ) MGlobal::executeCommand( m_preTransformMel );

	  if ( !ribNode->grouping.membership.empty() ) 
    {
      RtString members( const_cast< char* >( ribNode->grouping.membership.c_str() ) );
      RiAttribute( "grouping", "membership", &members, RI_NULL );
    }

		if ( !m_skipShadingAttributes )
    	if ( ribNode->shading.matte || ribNode->mayaMatteMode ) RiMatte( RI_TRUE );

    // If this is a single sided object, then turn that on (RMan default is Sides 2)
    if ( !ribNode->doubleSided ) RiSides( 1 );
    if ( ribNode->reversedNormals ) RiReverseOrientation();

    LIQDEBUGPRINTF( "-> object name: %s\n", ribNode->name.asChar() );
    MObject object;

	  // Moritz: only write out light linking if we're not in a shadow pass
	  if ( !liqglo_currentJob.isShadow || 
				 liqglo_currentJob.deepShadows && m_outputLightsInDeepShadows && !m_ignoreLights )
	  {
		  MObjectArray linkLights;

		  // light linking mode - Alf
		  // inclusive - lights are off by default and objects list included lights
		  // exclusive - lights are on by default and objects list ignored lights
		  // liquid Light sets - ignores the maya light linker
		  if ( m_liquidSetLightLinking ) ribNode->getSetLights( linkLights );
		  else                      	   ribNode->getLinkLights( linkLights, m_illuminateByDefault );
		
		  for ( unsigned i( 0 ); i < linkLights.length(); i++ )
		  {
			  MFnDagNode lightFnDag( linkLights[i] );
			  MString nodeName = lightFnDag.fullPathName();
			  if ( htable )
			  {
				  //RibNode * ln = htable->find( light, MRT_Light );
				  MDagPath nodeDagPath;
				  lightFnDag.getPath( nodeDagPath );
				  liqRibNodePtr  ln( htable->find( lightFnDag.fullPathName(), nodeDagPath, MRT_Light ) );
				  if ( NULL != ln )
				  {
					  if ( m_illuminateByDefault ) RiIlluminate( ln->object(0)->lightHandle(), RI_FALSE );
					  else            						 RiIlluminate( ln->object(0)->lightHandle(), RI_TRUE );
				  }
			  }
		  }
    }

    if ( liqglo_doMotion &&
         ribNode->motion.transformationBlur &&
         ( ribNode->object( 1 ) ) &&
         //( ribNode->object(0)->type != MRT_Locator ) && // Why the fuck do we not allow motion blur for locators?
         ( !liqglo_currentJob.isShadow || liqglo_currentJob.deepShadows ) )
    {
      LIQDEBUGPRINTF( "-> writing matrix motion blur data\n" );
      // Moritz: replaced RiMotionBegin call with ..V version to allow for more than five motion samples
      if ( liqglo_relativeMotion ) RiMotionBeginV( liqglo_motionSamples, liqglo_sampleTimesOffsets );
      else                         RiMotionBeginV( liqglo_motionSamples, liqglo_sampleTimes );
    }
#if 1
    RtMatrix ribMatrix;
    matrix = ribNode->object( 0 )->matrix( path.instanceNumber() );
    matrix.get( ribMatrix );

    if ( liqglo_relativeTransforms ) RiConcatTransform( ribMatrix ); 
	  else                             RiTransform( ribMatrix );
#elif 0  // Bat : a way to have double transforms :
	double doubleTransformMatrix[4][4];
	matrix = ribNode->object( 0 )->matrix( path.instanceNumber() );
	matrix.get( doubleTransformMatrix );
    
	int txIntPart = (int)(doubleTransformMatrix[3][0]);
	float txFloatPart = doubleTransformMatrix[3][0] - txIntPart;

	int tyIntPart = (int)(doubleTransformMatrix[3][1]);
	float tyFloatPart = doubleTransformMatrix[3][1] - tyIntPart;

	int tzIntPart = (int)(doubleTransformMatrix[3][2]);
	float tzFloatPart = doubleTransformMatrix[3][2] - tzIntPart;

	RtFloat floatTransformMatrixWithIntegerTranslatePart[4][4];
	matrix.get( floatTransformMatrixWithIntegerTranslatePart );
	RtFloat floatIdentityMatrixWithFloatingTranslatePart[4][4] = { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} };

	floatTransformMatrixWithIntegerTranslatePart[3][0] = txIntPart;
	floatTransformMatrixWithIntegerTranslatePart[3][1] = tyIntPart;
	floatTransformMatrixWithIntegerTranslatePart[3][2] = tzIntPart;

	floatIdentityMatrixWithFloatingTranslatePart[3][0] = txFloatPart;
	floatIdentityMatrixWithFloatingTranslatePart[3][1] = tyFloatPart;
	floatIdentityMatrixWithFloatingTranslatePart[3][2] = tzFloatPart;

	if( liqglo_relativeTransforms )
	{
		RiConcatTransform( floatIdentityMatrixWithFloatingTranslatePart );
		RiConcatTransform( floatTransformMatrixWithIntegerTranslatePart );
	}
	else
	{
		RiTransform( floatIdentityMatrixWithFloatingTranslatePart );
		RiConcatTransform( floatTransformMatrixWithIntegerTranslatePart );
	}
#endif

    // Output the world matrices for the motionblur
    // This will override the current transformation setting
    if ( liqglo_doMotion &&
         ribNode->motion.transformationBlur &&
         ( ribNode->object( 1 ) ) &&
         //( ribNode->object( 0 )->type != MRT_Locator ) && // Why the fuck do we not allow motion blur for locators?
         ( !liqglo_currentJob.isShadow || liqglo_currentJob.deepShadows ) )
    {
      path = ribNode->path();
      RtMatrix ribMatrix;
      for ( unsigned mm( 1 ); mm < liqglo_motionSamples; mm++ ) 
      {
        matrix = ribNode->object( mm )->matrix( path.instanceNumber() );
        matrix.get( ribMatrix );
        if ( liqglo_relativeTransforms ) RiConcatTransform( ribMatrix ); 
				else 														 RiTransform( ribMatrix );
      }
      RiMotionEnd();
    }

	  // Alf: postTransformMel
	  prePostPlug = fnTransform.findPlug( "liqPostTransformMel" );
	  m_postTransformMel = prePostPlug.asString();
	  if ( m_postTransformMel != "" )
		  MGlobal::executeCommand( m_postTransformMel );

    bool hasSurfaceShader( false );
    typedef enum {
        liqRegularShaderNode = 0,     // A regular Liquid node, keep it 0 to evaluate to false in conditions
        liqCustomPxShaderNode = 1,     // A custom MPxNode inheriting from liqCustomNode
        liqRibBoxShader = 2          // A rib box attached to the shader
    } liqDetailShaderKind;
    
    liqDetailShaderKind hasCustomSurfaceShader( liqRegularShaderNode );
    MString shaderRibBox( "" );
    bool hasDisplacementShader( false );
    bool hasVolumeShader( false );

    MPlug rmanShaderPlug;
    // Check for surface shader
    status.clear();
    MFnDagNode fnDagNode( path );
    rmanShaderPlug = fnDagNode.findPlug( MString( "liquidSurfaceShaderNode" ), &status );
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedShadingGroup.findPlug( MString( "liquidSurfaceShaderNode" ), &status ); 
    }
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedShader.findPlug( MString( "liquidSurfaceShaderNode" ), &status ); 
    }
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedShadingGroup.findPlug( MString( "surfaceShader" ), &status ); 
    }
    if ( status == MS::kSuccess && !rmanShaderPlug.isNull() ) 
    {
      if ( rmanShaderPlug.isConnected() ) 
      {
        MPlugArray rmShaderNodeArray;
        rmanShaderPlug.connectedTo( rmShaderNodeArray, true, true );
        MObject rmShaderNodeObj;
        rmShaderNodeObj = rmShaderNodeArray[0].node();
        MFnDependencyNode shaderDepNode( rmShaderNodeObj );
        // philippe : we must check the node type to avoid checking in regular maya shaders
        if ( shaderDepNode.typeName() == "liquidSurface" || 
						 shaderDepNode.typeName() == "liquidSurfaceSwitcher" ||
						 shaderDepNode.typeName() == "oldBlindDataBase" ) 
        { 
					//cout <<"setting shader"<<endl;
          ribNode->assignedShader.setObject( rmShaderNodeObj );
          hasSurfaceShader = true;
        } 
        else 
        { 
					// Is it a custom shading node ?
          MPxNode *mpxNode = shaderDepNode.userNode();
          liqCustomNode *customNode = NULL;
          if ( mpxNode && ( customNode = dynamic_cast<liqCustomNode*>(mpxNode) ) ) 
          { 
						// customNode will be null if can't be casted to a liqCustomNode
            ribNode->assignedShader.setObject( rmShaderNodeObj );
            hasSurfaceShader = true;
            hasCustomSurfaceShader = liqCustomPxShaderNode;
          } 
          else 
          { 
						// Try to find a liqRIBBox attribute
            MPlug ribbPlug = shaderDepNode.findPlug( MString( "liqRIBBox" ), &status );
            if ( status == MS::kSuccess ) 
            {
              ribbPlug.getValue( shaderRibBox );
              if ( shaderRibBox.substring(0,2) == "*H*" ) 
              {
                MString parseThis = shaderRibBox.substring(3, shaderRibBox.length() - 1 );
                shaderRibBox = parseThis;
              } 
              else if ( shaderRibBox.substring(0,3) == "*SH*" ) 
              {
                MString parseThis = shaderRibBox.substring(3, shaderRibBox.length() - 1 );
                shaderRibBox = parseThis;
              }
              hasSurfaceShader = true;
              hasCustomSurfaceShader = liqRibBoxShader;
            }
            // else {
            //liquidMessage( "Do noy know how to handle " + string( shaderDepNode.typeName().asChar() ), messageError );
            //}
          }
        }
      }
    }
    // Check for displacement shader
    status.clear();
    rmanShaderPlug = fnDagNode.findPlug( MString( "liquidDispShaderNode" ), &status );
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedShadingGroup.findPlug( MString( "liquidDispShaderNode" ), &status ); 
    }
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedDisp.findPlug( MString( "liquidDispShaderNode" ), &status ); 
    }
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedShadingGroup.findPlug( MString( "displacementShader" ), &status ); 
    }
    if ( ( status == MS::kSuccess ) && !rmanShaderPlug.isNull() && !m_ignoreDisplacements ) 
    {
      if ( rmanShaderPlug.isConnected() ) 
      {
        MPlugArray rmShaderNodeArray;
        rmanShaderPlug.connectedTo( rmShaderNodeArray, true, true );
        MObject rmShaderNodeObj;
        rmShaderNodeObj = rmShaderNodeArray[0].node();
        MFnDependencyNode shaderDepNode( rmShaderNodeObj );
        // philippe : we must check the node type to avoid checking in regular maya shaders
				if (	shaderDepNode.typeName() == "liquidDisplacement" ||
							shaderDepNode.typeName() == "liquidDisplacementSwitcher" ||
							shaderDepNode.typeName() == "oldBlindDataBase"
				)
        {
          ribNode->assignedDisp.setObject( rmShaderNodeObj );
          hasDisplacementShader = true;
        }
      }
    }
    // Check for volume shader
    status.clear();
    rmanShaderPlug = fnDagNode.findPlug( MString( "liquidVolumeShaderNode" ), &status );
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedShadingGroup.findPlug( MString( "liquidVolumeShaderNode" ), &status ); 
    }
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedVolume.findPlug( MString( "liquidVolumeShaderNode" ), &status ); 
    }
    if ( ( status != MS::kSuccess ) || ( !rmanShaderPlug.isConnected() ) ) 
    { 
      status.clear(); 
      rmanShaderPlug = ribNode->assignedShadingGroup.findPlug( MString( "volumeShader" ), &status ); 
    }
    if ( ( status == MS::kSuccess ) && !rmanShaderPlug.isNull() && !m_ignoreVolumes ) 
    {
      if ( rmanShaderPlug.isConnected() ) 
      {
        MPlugArray rmShaderNodeArray;
        rmanShaderPlug.connectedTo( rmShaderNodeArray, true, true );
        MObject rmShaderNodeObj;
        rmShaderNodeObj = rmShaderNodeArray[0].node();
        MFnDependencyNode shaderDepNode( rmShaderNodeObj );
        // philippe : we must check the node type to avoid checking in regular maya shaders
        if ( shaderDepNode.typeName() == "liquidVolume" || shaderDepNode.typeName() == "oldBlindDataBase" ) 
        {
          ribNode->assignedVolume.setObject( rmShaderNodeObj );
          hasVolumeShader = true;
        }
      }
    }
/*
    // displacement bounds
    float surfaceDisplacementBounds = 0.0;
    MString surfaceDisplacementBoundsSpace = "shader";
    MString tmpSpace = "";
    status.clear();
    if ( !ribNode->assignedShader.object().isNull() ) 
    {
      MPlug sDBPlug = ribNode->assignedShader.findPlug( MString( "displacementBound" ), &status );
      if ( status == MS::kSuccess ) 
        sDBPlug.getValue( surfaceDisplacementBounds );
      MPlug sDBSPlug = ribNode->assignedShader.findPlug( MString( "displacementBoundSpace" ), &status );
      if ( status == MS::kSuccess ) 
        sDBSPlug.getValue( tmpSpace );
      if ( tmpSpace != "" ) 
        surfaceDisplacementBoundsSpace = tmpSpace;
    }
    float dispDisplacementBounds = 0.0;
    MString dispDisplacementBoundsSpace = "shader";
    tmpSpace = "";
    status.clear();
    if ( !ribNode->assignedDisp.object().isNull() ) 
    {
      MPlug dDBPlug = ribNode->assignedDisp.findPlug( MString( "displacementBound" ), &status );
      if ( status == MS::kSuccess ) 
        dDBPlug.getValue( dispDisplacementBounds );
      MPlug sDBSPlug = ribNode->assignedDisp.findPlug( MString( "displacementBoundSpace" ), &status );
      if ( status == MS::kSuccess ) 
        sDBSPlug.getValue( tmpSpace );
      if ( tmpSpace != "" ) 
        dispDisplacementBoundsSpace = tmpSpace;
    }
    if ( ( dispDisplacementBounds != 0.0 ) && ( dispDisplacementBounds > surfaceDisplacementBounds ) ) 
    {
      RtString coordsys( const_cast< char* >( dispDisplacementBoundsSpace.asChar() ) );
      RiAttribute( "displacementbound", (RtToken) "sphere", &dispDisplacementBounds, "coordinatesystem", &coordsys, RI_NULL );
    } 
    else if ( ( surfaceDisplacementBounds != 0.0 ) ) 
    {
      RtString coordsys( const_cast< char* >( surfaceDisplacementBoundsSpace.asChar() ) );
      RiAttribute( "displacementbound", (RtToken) "sphere", &surfaceDisplacementBounds, "coordinatesystem", &coordsys, RI_NULL );
    }
*/
    LIQDEBUGPRINTF( "-> writing node attributes\n" );

    // if the node's shading rate == -1,
    // it means it hasn't been overriden by a liqShadingRate attribute.
    // No need to output it then.
		if ( !m_skipShadingAttributes )
    	if ( ribNode->shading.shadingRate > 0 )
     		RiShadingRate ( ribNode->shading.shadingRate );

    bool writeShaders( true );
    RtString mode;
    RtInt off( 0 );
    RtInt on( 1 );
		// END => EOF NOT SHADOW
    if ( !liqglo_currentJob.isShadow ) 
    {
      
      
			
			if ( !m_skipShadingAttributes )
			{
      	if ( !ribNode->shading.diceRasterOrient ) RiAttribute( "dice", (RtToken) "rasterorient", &off, RI_NULL );
      	if ( ribNode->shading.doubleShaded )      RiAttribute( "sides", (RtToken) "doubleshaded", &on, RI_NULL );
   		}
   		if ( !m_skipRayTraceAttributes )
			{ 
				if ( liquidRenderer.supports_RAYTRACE ) 
      	{
        	if ( ribNode->trace.sampleMotion ) RiAttribute( "trace", (RtToken) "samplemotion", &on, RI_NULL );
        	if ( ribNode->trace.displacements ) RiAttribute( "trace", (RtToken) "displacements", &on, RI_NULL );
        	if ( ribNode->trace.bias != 0.01f ) 
        	{
          	RtFloat bias( ribNode->trace.bias );
          	RiAttribute( "trace", (RtToken) "bias", &bias, RI_NULL );
        	}
        	if ( ribNode->trace.maxDiffuseDepth != 1 ) 
        	{
          	RtInt mddepth( ribNode->trace.maxDiffuseDepth );
          	RiAttribute( "trace", (RtToken) "maxdiffusedepth", &mddepth, RI_NULL );
        	}
        	if ( ribNode->trace.maxSpecularDepth != 2 ) 
        	{
          	RtInt msdepth( ribNode->trace.maxSpecularDepth );
          	RiAttribute( "trace", (RtToken) "maxspeculardepth", &msdepth, RI_NULL );
        	}
      	}
			}
      if ( !ribNode->visibility.camera ) 
        RiAttribute( "visibility", (RtToken) "camera", &off, RI_NULL );
#ifdef GENERIC_RIBLIB      
      extern int useAdvancedVisibilityAttributes;
      useAdvancedVisibilityAttributes = false;
#endif
      // old-style raytracing visibility support
      // philippe: if raytracing is off in the globals, trace visibility is turned off for all objects, transmission is set to TRANSPARENT for all objects
      if ( liquidRenderer.supports_RAYTRACE )
      {
        if ( !liquidRenderer.supports_ADVANCED_VISIBILITY ) 
        {
          if ( rt_useRayTracing )
          {
            if ( ribNode->visibility.trace ) RiAttribute( "visibility", (RtToken) "trace", &on, RI_NULL );
            else                             RiAttribute( "visibility", (RtToken) "trace", &off, RI_NULL );

            if ( ribNode->visibility.transmission != liqRibNode::visibility::TRANSMISSION_TRANSPARENT ) 
            {
              RtString trans;
              switch ( ribNode->visibility.transmission ) 
              {
                case liqRibNode::visibility::TRANSMISSION_OPAQUE:
                  trans = "opaque";
                  break;
                case liqRibNode::visibility::TRANSMISSION_OS:
                  trans = "Os";
                  break;
                case liqRibNode::visibility::TRANSMISSION_SHADER:
                default:
                  trans = "shader";
              }
              RiAttribute( "visibility", (RtToken) "string transmission", &trans, RI_NULL );
            }
          }
        }
        else  // philippe : prman 12.5 visibility support
        {
          #ifdef GENERIC_RIBLIB         
          useAdvancedVisibilityAttributes = true;
          #endif
          if ( rt_useRayTracing )
          {
       			if ( !m_skipVisibilityAttributes )
						{     
              if ( ribNode->visibility.diffuse ) RiAttribute( "visibility", (RtToken) "int diffuse", &on, RI_NULL );
              if ( ribNode->visibility.specular ) RiAttribute( "visibility", (RtToken) "int specular", &on, RI_NULL );
              if ( ribNode->visibility.newtransmission ) RiAttribute( "visibility", (RtToken) "int transmission", &on, RI_NULL );
						}
						if ( !m_skipRayTraceAttributes )
						{
            	if ( ribNode->hitmode.diffuse != liqRibNode::hitmode::DIFFUSE_HITMODE_PRIMITIVE ) 
            	{
              	switch ( ribNode->hitmode.diffuse ) 
              	{
                	case liqRibNode::hitmode::DIFFUSE_HITMODE_SHADER:
                  	mode = "shader";
                  	break;
                  case liqRibNode::hitmode::DIFFUSE_HITMODE_CACHE:
                  	mode = "cache";
                  	break;
                	case liqRibNode::hitmode::DIFFUSE_HITMODE_PRIMITIVE:
                	default:
                  	mode = "primitive";
              	}
              	RiAttribute( "shade", (RtToken) "string diffusehitmode", &mode, RI_NULL );
            	}
            
              if ( ribNode->hitmode.specular != liqRibNode::hitmode::SPECULAR_HITMODE_SHADER ) 
              {
                switch ( ribNode->hitmode.specular ) 
                {
                  case liqRibNode::hitmode::SPECULAR_HITMODE_PRIMITIVE:
                    mode = "primitive";
                    break;
                  case liqRibNode::hitmode::SPECULAR_HITMODE_CACHE:
                    mode = "cache";
                    break;
                  case liqRibNode::hitmode::SPECULAR_HITMODE_SHADER:
                  default:
                    mode = "shader";
                }
                RiAttribute( "shade", (RtToken) "string specularhitmode", &mode, RI_NULL );
              }
  
              if ( ribNode->hitmode.transmission != liqRibNode::hitmode::TRANSMISSION_HITMODE_SHADER ) 
              {
                switch ( ribNode->hitmode.transmission ) 
                {
                  case liqRibNode::hitmode::TRANSMISSION_HITMODE_PRIMITIVE:
                    mode = "primitive";
                    break;
                  case liqRibNode::hitmode::TRANSMISSION_HITMODE_CACHE:
                    mode = "cache";
                    break;
                  case liqRibNode::hitmode::TRANSMISSION_HITMODE_SHADER:
                  default:
                    mode = "shader";
                }
                RiAttribute( "shade", (RtToken) "string transmissionhitmode", &mode, RI_NULL );
              }
            }
					}
					if ( !m_skipShadingAttributes )
					{
          	if ( ribNode->hitmode.camera != liqRibNode::hitmode::CAMERA_HITMODE_SHADER ) 
          	{
            	switch ( ribNode->hitmode.camera ) 
            	{
              	case liqRibNode::hitmode::CAMERA_HITMODE_PRIMITIVE:
                	mode = "primitive";
                	break;
                case liqRibNode::hitmode::CAMERA_HITMODE_CACHE:
                	mode = "cache";
                	break;
              	case liqRibNode::hitmode::CAMERA_HITMODE_SHADER:
              	default:
                	mode = "shader";
            	}
            	RiAttribute( "shade", (RtToken) "string camerahitmode", &mode, RI_NULL );
          	}
					}
        }

        // irradiance attributes
        if ( ribNode->irradiance.shadingRate != 1.0f ) 
        {
          RtFloat rate = ribNode->irradiance.shadingRate;
          RiAttribute( "irradiance", (RtToken) "shadingrate", &rate, RI_NULL );
        }

        if ( ribNode->irradiance.nSamples != 64 ) 
        {
          RtInt samples = ribNode->irradiance.nSamples;
          RiAttribute( "irradiance", (RtToken) "nsamples", &samples, RI_NULL );
        }

        if ( ribNode->irradiance.maxError != 0.5f ) 
        {
          RtFloat merror = ribNode->irradiance.maxError;
          RiAttribute( "irradiance", (RtToken) "float maxerror", &merror, RI_NULL );
        }

        if ( ribNode->irradiance.maxPixelDist != 30.0f ) 
        {
          RtFloat mpd = ribNode->irradiance.maxPixelDist;
          RiAttribute( "irradiance", (RtToken) "float maxpixeldist", &mpd, RI_NULL );
        }

        if ( ribNode->irradiance.handle != "" ) 
        {
          RtString handle = const_cast< char* >( ribNode->irradiance.handle.asChar() );
          RiAttribute( "irradiance", (RtToken) "handle", &handle, RI_NULL );
        }

        if ( ribNode->irradiance.fileMode != liqRibNode::irradiance::FILEMODE_NONE ) 
        {
          RtString mode;
          switch ( ribNode->irradiance.fileMode ) 
          {
            case liqRibNode::irradiance::FILEMODE_READ:
              mode = "r";
              break;
            case liqRibNode::irradiance::FILEMODE_WRITE:
              mode = "w";
              break;
            case liqRibNode::irradiance::FILEMODE_READ_WRITE:
            default:
              mode = "rw";
          }
          RiAttribute( "irradiance", (RtToken) "filemode", &mode, RI_NULL );
        }

        // ymesh: photon visibility support
        if ( rt_useRayTracing && ribNode->visibility.photon ) 
          RiAttribute( "visibility", (RtToken) "int photon", &on, RI_NULL );

        if ( ribNode->photon.globalMap != "" ) 
        {
          MString parsedName = parseString( ribNode->photon.globalMap, false );  //  doEscaped = false
          RtString map = const_cast< char* >( parsedName.asChar() );
          RiAttribute( "photon", (RtToken) "globalmap", &map, RI_NULL );
        }

        if ( ribNode->photon.causticMap != "" ) 
        {
          MString parsedName = parseString( ribNode->photon.causticMap, false );  //  doEscaped = false
          RtString map = const_cast< char* >( parsedName.asChar() );
          RiAttribute( "photon", (RtToken) "causticmap", &map, RI_NULL );
        }

        if ( ribNode->photon.shadingModel != liqRibNode::photon::SHADINGMODEL_MATTE ) 
        {
          switch ( ribNode->photon.shadingModel  ) 
          {
            case liqRibNode::photon::SHADINGMODEL_GLASS:
              mode = "glass";
              break;
            case liqRibNode::photon::SHADINGMODEL_WATER:
              mode = "water";
              break;
            case liqRibNode::photon::SHADINGMODEL_CHROME:
              mode = "chrome";
              break;
            case liqRibNode::photon::SHADINGMODEL_TRANSPARENT:
              mode = "chrome";
              break;
            case liqRibNode::photon::SHADINGMODEL_DIALECTRIC:
              mode = "dielectric";
              break;
            case liqRibNode::photon::SHADINGMODEL_MATTE:
            default:
              mode = "matte";
          }
          RiAttribute( "photon", (RtToken) "shadingmodel", &mode, RI_NULL );
        }

        if ( ribNode->photon.estimator != 100 ) 
        {
          RtInt estimator = ribNode->photon.estimator;
          RiAttribute( "photon", (RtToken) "estimator", &estimator, RI_NULL );
        }
      } // liquidRenderer.supports_RAYTRACE
      
		  // 3Delight sss group
		  if ( ribNode->delightSSS.doScatter && liquidRenderer.renderName == MString("3Delight") )
		  {
#ifdef GENERIC_RIBLIB         
        useAdvancedVisibilityAttributes = true;
#endif			  
        RtString groupName = const_cast< char* >( ribNode->delightSSS.groupName.asChar() );
			  RiAttribute( "visibility", (RtToken) "string subsurface", &groupName, RI_NULL );

			  RtColor scattering, absorption;
			  scattering[0] = ribNode->delightSSS.scattering.r;
			  scattering[1] = ribNode->delightSSS.scattering.g;
			  scattering[2] = ribNode->delightSSS.scattering.b;

			  absorption[0] = ribNode->delightSSS.absorption.r;
			  absorption[1] = ribNode->delightSSS.absorption.g;
			  absorption[2] = ribNode->delightSSS.absorption.b;
        
        if ( scattering[0] && scattering[1] && scattering[2] )
          RiAttribute( "subsurface",
				    (RtToken) "scattering", &scattering,
				    (RtToken) "absorption", &absorption,
            RI_NULL );

        RtFloat refractionindex = ribNode->delightSSS.refraction;
			  RtFloat shadingrate = ribNode->delightSSS.shadingRate;
			  RtFloat scale = ribNode->delightSSS.scale;

			  RiAttribute( "subsurface",
          (RtToken) "refractionindex", &refractionindex,
				  (RtToken) "shadingrate", &shadingrate,
				  (RtToken) "scale", &scale, 
          RI_NULL );

        RtColor meanfreepath, reflectance;
        meanfreepath[0] = ribNode->delightSSS.meanfreepath.r;
			  meanfreepath[1] = ribNode->delightSSS.meanfreepath.g;
			  meanfreepath[2] = ribNode->delightSSS.meanfreepath.b;

			  reflectance[0] = ribNode->delightSSS.reflectance.r;
			  reflectance[1] = ribNode->delightSSS.reflectance.g;
			  reflectance[2] = ribNode->delightSSS.reflectance.b;
        
        if ( reflectance[0] && reflectance[1] && reflectance[2] )
          RiAttribute( "subsurface",
				    (RtToken) "meanfreepath", &meanfreepath,
				    (RtToken) "reflectance", &reflectance, 
            RI_NULL );

        if ( ribNode->delightSSS.referencecamera != "" )
        {
          RtString referenceCamera = const_cast< char* >( ribNode->delightSSS.referencecamera.asChar() );
          RiAttribute( "subsurface", (RtToken) "string referencecamera", &referenceCamera, RI_NULL );
        }
		  }

      if ( liqglo_doMotion && 
           ribNode->motion.factor != 1.0f && 
           ( ribNode->motion.deformationBlur || ribNode->motion.transformationBlur )  ) 
        RiGeometricApproximation( "motionfactor", ribNode->motion.factor );
      
      ribNode->writeUserAttributes();
    } // !liqglo_currentJob.isShadow
    else
    {
      if ( ( !liqglo_currentJob.deepShadows && !m_outputShadersInShadows ) || 
					( liqglo_currentJob.deepShadows && !m_outputShadersInDeepShadows ) )
        writeShaders = false;
    }
    
    // new prman 16.x shade attributes group 
    if ( ribNode->shade.strategy != liqRibNode::shade::SHADE_STRATEGY_GRIDS )
    {
      mode = "vpvolumes"; 
      RiAttribute( "shade", (RtToken) "strategy", &mode, RI_NULL );
    }
    if ( ribNode->shade.volumeIntersectionStrategy != liqRibNode::shade::SHADE_VOLUMEINTERSECTIONSTRATEGY_EXCLUSIVE )
    {
      mode = "additive"; 
      RiAttribute( "shade", (RtToken) "volumeintersectionstrategy", &mode, RI_NULL );
    }
    if ( ribNode->shade.volumeIntersectionPriority != 0.0 )
    {
      RtFloat value= ribNode->shade.volumeIntersectionPriority; 
      RiAttribute( "shade", (RtToken) "volumeintersectionpriority", &value, RI_NULL );
    }

    if ( writeShaders ) 
    {
      if ( hasVolumeShader && !m_ignoreVolumes ) 
      {
  			//liqShader& currentShader( liqGetShader( ribNode->assignedVolume.object() ) );
				liqGenericShader& currentShader = liqShaderFactory::instance().getShader( ribNode->assignedVolume.object(), liqglo_exportAllShadersParams );
  			// per shader shadow pass override
  			if ( !liqglo_currentJob.isShadow || currentShader.outputInShadow )
  				currentShader.write(liqglo_shortShaderNames, 0);
  		}
	    if ( hasSurfaceShader && !m_ignoreSurfaces )
	    {
		    if ( hasCustomSurfaceShader )
		    {
			    if ( hasCustomSurfaceShader == liqCustomPxShaderNode )
			    {  // Just call the write method of the custom shader
				    MFnDependencyNode customShaderDepNode( ribNode->assignedShader.object() );
				    MPxNode *mpxNode = customShaderDepNode.userNode();
				    liqCustomNode *customNode( NULL );
				    if ( mpxNode && ( customNode = dynamic_cast<liqCustomNode*>( mpxNode ) ) )
					    customNode->liquidWrite();
				    else
					    ;// Should never happen in theory ... but what is the way to report a problem ???
			    }
			    else
			    { 
				    // Default : just write the contents of the rib box
				    RiArchiveRecord( RI_VERBATIM, ( char* )shaderRibBox.asChar() );
				    RiArchiveRecord( RI_VERBATIM, "\n" );
			    }
		    }
		    else
		    {
				  liqGenericShader& currentShader = liqShaderFactory::instance().getShader( ribNode->assignedShader.object(), liqglo_exportAllShadersParams );
				

			    // Output color overrides or color   ====>>>>>  WILL BE DONE IN liqShader::write
				//if(ribNode->shading.color.r != -1.0)
				//{
				//	RtColor rColor;
				//	rColor[0] = ribNode->shading.color[0];
				//	rColor[1] = ribNode->shading.color[1];
				//	rColor[2] = ribNode->shading.color[2];
				//	RiColor( rColor );
				//}
				//else
				//{
				//	RiColor( currentShader.getColor() );
				//}

				//if(ribNode->shading.opacity.r != -1.0)
				//{
				//	RtColor rOpacity;
				//	rOpacity[0] = ribNode->shading.opacity[0];
				//	rOpacity[1] = ribNode->shading.opacity[1];
				//	rOpacity[2] = ribNode->shading.opacity[2];
				//	RiOpacity( rOpacity );
				//}
				//else
				//{
				//	RiOpacity( currentShader.getOpacity() );
				//}

			    // per shader shadow pass override
  				if ( !liqglo_currentJob.isShadow || currentShader.outputInShadow )
  				{
  					currentShader.write(liqglo_shortShaderNames, 0);
  				}
		    }
	    } 
      else 
      {
        RtColor rColor,rOpacity;
        if ( m_shaderDebug ) 
        {
          // shader debug on !! everything goes red and opaque !!!
          rColor[0] = 1.;
          rColor[1] = 0.;
          rColor[2] = 0.;
          RiColor( rColor );

          rOpacity[0] = 1.;
          rOpacity[1] = 1.;
          rOpacity[2] = 1.;
          RiOpacity( rOpacity );
        } 
        else 
        {
          if ( ribNode->shading.color.r != -1.0 ) 
          {
            rColor[0] = ribNode->shading.color[0];
            rColor[1] = ribNode->shading.color[1];
            rColor[2] = ribNode->shading.color[2];
            RiColor( rColor );
          } 
          else if ( ( ribNode->color.r != -1.0 ) ) 
          {
            rColor[0] = ribNode->color[0];
            rColor[1] = ribNode->color[1];
            rColor[2] = ribNode->color[2];
            RiColor( rColor );
          }
          if ( ribNode->shading.opacity.r != -1.0 ) 
          {
            rOpacity[0] = ribNode->shading.opacity[0];
            rOpacity[1] = ribNode->shading.opacity[1];
            rOpacity[2] = ribNode->shading.opacity[2];
            RiOpacity( rOpacity );
          } 
          else if ( ( ribNode->opacity.r != -1.0 ) ) 
          {
            rOpacity[0] = ribNode->opacity[0];
            rOpacity[1] = ribNode->opacity[1];
            rOpacity[2] = ribNode->opacity[2];
            RiOpacity( rOpacity );
          }
        }

        if ( !m_ignoreSurfaces ) 
        {
          MObject shadingGroup = ribNode->assignedShadingGroup.object();
				  MObject shader = ribNode->findShader();
          //
          // here we check for regular shader nodes first
          // and assign default shader to shader-less nodes.
          //
          if ( m_shaderDebug )                          RiSurface( "constant", RI_NULL );
          else if ( shader.apiType() == MFn::kLambert ) RiSurface( "matte", RI_NULL );
          else if ( shader.apiType() == MFn::kPhong )   RiSurface( "plastic", RI_NULL );
          else if ( path.hasFn( MFn::kPfxHair ) ) 
          {
            // get some of the hair system parameters
            RtFloat translucence = 0, specularPower = 0;
            RtColor specularColor;
            //cout <<"getting pfxHair shading params..."<<endl;
            MObject hairSystemObj;
            MFnDependencyNode pfxHairNode( path.node() );
            MPlug plugToHairSystem = pfxHairNode.findPlug( "renderHairs", &status );
            MPlugArray hsPlugs;
            status.clear();
            if ( plugToHairSystem.connectedTo( hsPlugs, true, false, &status ) ) 
              if ( status == MS::kSuccess ) hairSystemObj = hsPlugs[0].node();
            
            if ( hairSystemObj != MObject::kNullObj ) 
            {
              MFnDependencyNode hairSystemNode(hairSystemObj);
              MPlug paramPlug;
              status.clear();
              paramPlug = hairSystemNode.findPlug( "translucence", &status );
              if ( status == MS::kSuccess ) paramPlug.getValue( translucence );
              status.clear();
              paramPlug = hairSystemNode.findPlug( "specularColor", &status );
              if ( status == MS::kSuccess ) 
              {
                paramPlug.child(0).getValue( specularColor[0] );
                paramPlug.child(1).getValue( specularColor[1] );
                paramPlug.child(2).getValue( specularColor[2] );
              }
              status.clear();
              paramPlug = hairSystemNode.findPlug( "specularPower", &status );
              if ( status == MS::kSuccess ) paramPlug.getValue( specularPower );
            }
            RiSurface(  "liquidpfxhair",
                        "float specularpower", &specularPower,
                        "float translucence",  &translucence,
                        "color specularcolor", &specularColor,
                        RI_NULL );
          } 
          else if ( path.hasFn( MFn::kPfxToon ) )     RiSurface( "liquidpfxtoon", RI_NULL );
		      else if ( path.hasFn( MFn::kPfxGeometry ) ) RiSurface( "liquidpfx", RI_NULL );
		      else                                        RiSurface( "plastic", RI_NULL );
        }
      }
    } 
    else if ( liqglo_currentJob.deepShadows ) 
    {
      // if the current job is a deep shadow,
      // we still want to output primitive color and opacity.
      // In case of custom shaders, what should we do ? Stephane.
      if ( hasSurfaceShader && ! hasCustomSurfaceShader ) 
      {
        //liqShader & currentShader = liqGetShader( ribNode->assignedShader.object());
				liqGenericShader &currentShader = liqShaderFactory::instance().getShader( ribNode->assignedShader.object(), liqglo_exportAllShadersParams );

        // Output color overrides or color
        if ( ribNode->shading.color.r != -1.0 ) 
        {
          RtColor rColor;
          rColor[0] = ribNode->shading.color[0];
          rColor[1] = ribNode->shading.color[1];
          rColor[2] = ribNode->shading.color[2];
          RiColor( rColor );
        } 
        else 
          RiColor( currentShader.getColor() );

        if ( ribNode->shading.opacity.r != -1.0 ) 
        {
          RtColor rOpacity;
          rOpacity[0] = ribNode->shading.opacity[0];
          rOpacity[1] = ribNode->shading.opacity[1];
          rOpacity[2] = ribNode->shading.opacity[2];
          RiOpacity( rOpacity );
        } 
        else 
          RiOpacity( currentShader.getOpacity() );
      } 
      else 
      {
        RtColor rColor,rOpacity;

        if ( ribNode->shading.color.r != -1.0 ) 
        {
          rColor[0] = ribNode->shading.color[0];
          rColor[1] = ribNode->shading.color[1];
          rColor[2] = ribNode->shading.color[2];
          RiColor( rColor );
        } 
        else if( ( ribNode->color.r != -1.0 ) ) 
        {
          rColor[0] = ribNode->color[0];
          rColor[1] = ribNode->color[1];
          rColor[2] = ribNode->color[2];
          RiColor( rColor );
        }
        if ( ribNode->shading.opacity.r != -1.0 ) 
        {
          rOpacity[0] = ribNode->shading.opacity[0];
          rOpacity[1] = ribNode->shading.opacity[1];
          rOpacity[2] = ribNode->shading.opacity[2];
          RiOpacity( rOpacity );
        } 
        else if( ( ribNode->opacity.r != -1.0 ) ) 
        {
          rOpacity[0] = ribNode->opacity[0];
          rOpacity[1] = ribNode->opacity[1];
          rOpacity[2] = ribNode->opacity[2];
          RiOpacity( rOpacity );
        }
        if ( path.hasFn( MFn::kPfxHair ) ) 
        {
          // get some of the hair system parameters
          float translucence = 0, specularPower = 0;
          float specularColor[3];
          //cout <<"getting pfxHair shading params..."<<endl;
          MObject hairSystemObj;
          MFnDependencyNode pfxHairNode( path.node() );
          MPlug plugToHairSystem = pfxHairNode.findPlug( "renderHairs", &status );
          MPlugArray hsPlugs;
          status.clear();
          if ( plugToHairSystem.connectedTo( hsPlugs, true, false, &status ) ) 
            if ( status == MS::kSuccess ) hairSystemObj = hsPlugs[0].node();
          if ( hairSystemObj != MObject::kNullObj ) 
          {
            MFnDependencyNode hairSystemNode(hairSystemObj);
            MPlug paramPlug;
            status.clear();
            paramPlug = hairSystemNode.findPlug( "translucence", &status );
            if ( status == MS::kSuccess ) paramPlug.getValue( translucence );
            status.clear();
            paramPlug = hairSystemNode.findPlug( "specularColor", &status );
            if ( status == MS::kSuccess ) 
            {
              paramPlug.child(0).getValue( specularColor[0] );
              paramPlug.child(1).getValue( specularColor[1] );
              paramPlug.child(2).getValue( specularColor[2] );
              //cout <<"specularColor = "<<specularColor<<endl;
            }
            status.clear();
            paramPlug = hairSystemNode.findPlug( "specularPower", &status );
            if ( status == MS::kSuccess ) paramPlug.getValue( specularPower );
          }
          RiSurface(  "liquidPfxHair",
                      "float specularPower", &specularPower,
                      "float translucence",  &translucence,
                      "color specularColor", &specularColor,
                      RI_NULL );
        }
      }
    } 
    else 
      RiSurface( "null", RI_NULL );

    if ( hasDisplacementShader && !m_ignoreDisplacements ) 
    {
      //liqShader & currentShader = liqGetShader( ribNode->assignedDisp.object() );
			liqGenericShader &currentShader = liqShaderFactory::instance().getShader( ribNode->assignedDisp.object(), liqglo_exportAllShadersParams );

      // per shader shadow pass override
  		if ( !liqglo_currentJob.isShadow || currentShader.outputInShadow )
  		{
  			currentShader.write(liqglo_shortShaderNames, 0);
  		}
    }
    if ( ribNode->rib.box != "" && ribNode->rib.box != "-" ) 
      RiArchiveRecord( RI_COMMENT, " RIB Box:\n%s", ribNode->rib.box.asChar() );

    if ( ribNode->rib.readArchive != "" && ribNode->rib.readArchive != "-" ) 
      RiArchiveRecord( RI_VERBATIM, " ReadArchive \"%s\" \n", ribNode->rib.readArchive.asChar() );
    if ( ribNode->rib.delayedReadArchive != "" && ribNode->rib.delayedReadArchive != "-" ) 
    {
      RiArchiveRecord( RI_VERBATIM, " Procedural \"DelayedReadArchive\" [ \"%s\" ] [ %f %f %f %f %f %f ] \n", ribNode->rib.delayedReadArchive.asChar(), ribNode->bound[0],ribNode->bound[3],ribNode->bound[1],ribNode->bound[4],ribNode->bound[2],ribNode->bound[5] );
      // should be using the bounding box node - Alf
      /* {
        // this is a visual display of the archive's bounding box
        RiAttributeBegin();
        RtColor debug;
        debug[0] = debug[1] = 1;
        debug[2] = 0;
        RiColor( debug );
        debug[0] = debug[1] = debug[2] = 0.250;
        RiOpacity( debug );
        RiSurface( "defaultsurface", RI_NULL );
        RiArchiveRecord( RI_VERBATIM, "Attribute \"visibility\" \"int diffuse\" [0]\n" );
        RiArchiveRecord( RI_VERBATIM, "Attribute \"visibility\" \"int specular\" [0]\n" );
        RiArchiveRecord( RI_VERBATIM, "Attribute \"visibility\" \"int transmission\" [0]\n" );
        float xmin = ribNode->bound[0];
        float ymin = ribNode->bound[1];
        float zmin = ribNode->bound[2];
        float xmax = ribNode->bound[3];
        float ymax = ribNode->bound[4];
        float zmax = ribNode->bound[5];
        RiSides( 2 );
        RiArchiveRecord( RI_VERBATIM, "Polygon \"P\" [ %f %f %f  %f %f %f  %f %f %f  %f %f %f ]\n", xmin,ymax,zmin, xmax,ymax,zmin, xmax,ymax,zmax, xmin,ymax,zmax );
        RiArchiveRecord( RI_VERBATIM, "Polygon \"P\" [ %f %f %f  %f %f %f  %f %f %f  %f %f %f ]\n", xmin,ymin,zmin, xmax,ymin,zmin, xmax,ymin,zmax, xmin,ymin,zmax );
        RiArchiveRecord( RI_VERBATIM, "Polygon \"P\" [ %f %f %f  %f %f %f  %f %f %f  %f %f %f ]\n", xmin,ymax,zmax, xmax,ymax,zmax, xmax,ymin,zmax, xmin,ymin,zmax );
        RiArchiveRecord( RI_VERBATIM, "Polygon \"P\" [ %f %f %f  %f %f %f  %f %f %f  %f %f %f ]\n", xmin,ymax,zmin, xmax,ymax,zmin, xmax,ymin,zmin, xmin,ymin,zmin );
        RiAttributeEnd();
      } */
    }
	  // Alf: preShapeMel
	  prePostPlug = fnTransform.findPlug( "liqPreShapeMel" );
	  m_preShapeMel = prePostPlug.asString();
	  if ( m_preShapeMel != "" )  MGlobal::executeCommand( m_preShapeMel );
		
		// receive shadows ?   =>   Attribute "user" "int receivesShadows" [0/1]
		//if( !ribNode->object(0)->receiveShadow )
		{
			int receiveShadows = ribNode->object(0)->receiveShadow;
			RiAttribute("user", (RtToken)"int receivesShadows", &receiveShadows, RI_NULL);
		}
  
  	if ( !ribNode->ignoreShapes ) 
    {
      // check to see if we are writing a curve to set the proper basis
      if ( ribNode->object(0)->type == MRT_NuCurve
        || ribNode->object(0)->type == MRT_PfxHair
        || ribNode->object(0)->type == MRT_PfxTube
			  || ribNode->object(0)->type == MRT_PfxLeaf
			  || ribNode->object(0)->type == MRT_PfxPetal 
        || ribNode->object(0)->type == MRT_Curves )
      
      {
        RiBasis( RiBSplineBasis, 1, RiBSplineBasis, 1 );
      } 

      bool doMotion ( liqglo_doDef && 
											ribNode->motion.deformationBlur &&
                    ( ribNode->object(1) ) &&
                    ( ribNode->object(0)->type != MRT_RibGen ) &&
                    //( ribNode->object(0)->type != MRT_Locator ) &&
                    ( !liqglo_currentJob.isShadow || liqglo_currentJob.deepShadows ) );

      if ( doMotion )
      {
        // For each grain, open a new motion block...
        for ( unsigned i( 0 ); i < ribNode->object( 0 )->granularity(); i++ ) 
        {
          if ( ribNode->object( 0 )->isNextObjectGrainAnimated() ) 
          {
            RiMotionBeginV( liqglo_motionSamples, ( liqglo_relativeMotion )? liqglo_sampleTimesOffsets : liqglo_sampleTimes);

            for ( unsigned msampleOn( 0 ); msampleOn < liqglo_motionSamples; msampleOn++ ) 
              ribNode->object( msampleOn )->writeNextObjectGrain();

            RiMotionEnd();
          } 
          else 
            ribNode->object( 0 )->writeNextObjectGrain();
        }
      } 
      else 
        ribNode->object( 0 )->writeObject();
    
	    // Alf: postShapeMel
	    prePostPlug = fnTransform.findPlug( "liqPostShapeMel" );
	    m_postShapeMel = prePostPlug.asString();
	    if ( m_postShapeMel != "" ) MGlobal::executeCommand( m_postShapeMel );

    } // else RiArchiveRecord( RI_COMMENT, " Shapes Ignored !!" );
    RiAttributeEnd();
  }
  while ( attributeDepth > 0 ) 
  {
    RiAttributeEnd();
    attributeDepth--;
  }
  return returnStatus;
}

/**
 * Write the world prologue.
 * This includes the pre- and post-world begin RIB boxes and the definition of
 * any default coordinate systems.
 */
MStatus liqRibTranslator::worldPrologue()
{
  MStatus returnStatus = MS::kSuccess;
  LIQDEBUGPRINTF( "-> Writing world prologue.\n" );
  // if this is a readArchive that we are exporting then ingore this header information
	if ( !m_exportReadArchive )
	{
		MFnDependencyNode globalsNode( rGlobalObj );
		MPlug prePostplug( globalsNode.findPlug( "preWorldMel" ) );
		MString melcommand( prePostplug.asString() );
		// put in pre-worldbegin statements
		if ( m_preWorldRIB != "" || melcommand.length() )
		{
			RiArchiveRecord(RI_COMMENT,  " Pre-WorldBegin RIB from liquid globals");
			MGlobal::executeCommand( melcommand );
			RiArchiveRecord(RI_VERBATIM, ( char* )m_preWorldRIB.asChar());
			RiArchiveRecord(RI_VERBATIM, "\n");
		}
    // output the arbitrary clipping planes here /////////////
    //
    {
      for ( RNMAP::iterator rniter( htable->RibNodeMap.begin() ); rniter != htable->RibNodeMap.end(); rniter++ ) 
      {
        LIQ_CHECK_CANCEL_REQUEST;
        liqRibNodePtr   ribNode = (*rniter).second;
        if ( ribNode->object(0)->ignore || ribNode->object(0)->type != MRT_ClipPlane ) continue;
        RiTransformBegin();
        if ( m_outputComments ) RiArchiveRecord( RI_COMMENT, "Clipping Plane: %s", ribNode->name.asChar(), RI_NULL );
        RtMatrix ribMatrix;
        MMatrix matrix;
        MDagPath path;

        matrix = ribNode->object(0)->matrix( path.instanceNumber() );
        matrix.get( ribMatrix );
        RiConcatTransform( ribMatrix );

        ribNode->object(0)->writeObject();
        ribNode->object(0)->written = 1;

        RiTransformEnd();
      }
    }
    RiWorldBegin();
    // set attributes from the globals
#ifdef GENERIC_RIBLIB      
      extern int useAdvancedVisibilityAttributes;
      useAdvancedVisibilityAttributes = false;
#endif
    if ( rt_useRayTracing )
    {
      RiArchiveRecord(RI_COMMENT,  " Ray-Tracing Attributes from liquid globals");
      RtInt on( 1 );
      
      if ( !liquidRenderer.supports_ADVANCED_VISIBILITY )
      {
        RtString trans = "shader";
        RiAttribute( "visibility", "int trace", &on, RI_NULL );
        RiAttribute( "visibility", "string transmission", &trans, RI_NULL );
      }
      else
      {
        #ifdef GENERIC_RIBLIB         
        useAdvancedVisibilityAttributes = true;
        #endif

        RiAttribute( "visibility", "int diffuse", &on, RI_NULL );
        RiAttribute( "visibility", "int specular", &on, RI_NULL );
        RiAttribute( "visibility", "int transmission", &on, RI_NULL );
      }

      if ( rt_traceDisplacements ) RiAttribute("trace", "int displacements", &on, RI_NULL );
      if ( rt_traceSampleMotion )  RiAttribute("trace", "int samplemotion", &on, RI_NULL );
      if ( rt_traceBias != 0 )     RiAttribute("trace", "float bias", &rt_traceBias, RI_NULL );
      RiAttribute("trace", "int maxdiffusedepth", &rt_traceMaxDiffuseDepth, RI_NULL);
      RiAttribute("trace", "int maxspeculardepth", &rt_traceMaxSpecularDepth, RI_NULL);
      if ( rt_irradianceMaxError != -1.0 )     RiAttribute( "irradiance", (RtToken) "float maxerror", &rt_irradianceMaxError, RI_NULL );
      if ( rt_irradianceMaxPixelDist != -1.0 ) RiAttribute( "irradiance", (RtToken) "float maxpixeldist", &rt_irradianceMaxPixelDist, RI_NULL );

      // ymesh: add photon/caustic map attribites
      if (  rt_photonGlobalHandle != "" || rt_causticGlobalHandle != "") 
      {
        MString parsedName = parseString( rt_photonGlobalHandle, false );  //  doEscaped = false

        RtString photon_map = const_cast< char* >( parsedName.asChar() );
        RiAttribute( "photon", (RtToken) "globalmap", &photon_map, RI_NULL );
        
        parsedName = parseString( rt_causticGlobalHandle, false );  //  doEscaped = false
        RtString caustic_map = const_cast< char* >( parsedName.asChar() );
        RiAttribute( "photon", (RtToken) "causticmap", &caustic_map, RI_NULL );
      
        RtString model;
        switch ( rt_photonShadingModel  ) 
        {
          case liqRibNode::photon::SHADINGMODEL_GLASS:
            model = "glass";
            break;
          case liqRibNode::photon::SHADINGMODEL_WATER:
            model = "water";
            break;
          case liqRibNode::photon::SHADINGMODEL_CHROME:
            model = "chrome";
            break;
          case liqRibNode::photon::SHADINGMODEL_TRANSPARENT:
            model = "chrome";
            break;
          case liqRibNode::photon::SHADINGMODEL_DIALECTRIC:
            model = "dielectric";
            break;
          case liqRibNode::photon::SHADINGMODEL_MATTE:
          default:
            model = "matte";
        }
        RiAttribute( "photon", (RtToken) "shadingmodel", &model, RI_NULL );
      
        RtInt estimator = rt_photonEstimator;
        RiAttribute( "photon", (RtToken) "estimator", &estimator, RI_NULL );
      }
    }
    
    // put in post-worldbegin statements
	  prePostplug = globalsNode.findPlug( "postWorldMel" );
	  melcommand = prePostplug.asString();
    if ( m_postWorldRIB != "" || melcommand.length() )
	  {
		  RiArchiveRecord( RI_COMMENT,  " Post-WorldBegin RIB from liquid globals" );
		  MGlobal::executeCommand( melcommand );
		  RiArchiveRecord( RI_VERBATIM, ( char* )m_postWorldRIB.asChar() );
		  RiArchiveRecord( RI_VERBATIM, "\n");
    }
    RiTransformBegin();
    RiCoordinateSystem( "worldspace" );
    RiTransformEnd();

    RiTransformBegin();
    RiRotate( -90., 1., 0., 0. );
    RiCoordinateSystem( "_environment" );
    RiTransformEnd();
    RiReverseOrientation(); // ???? 
  }
  return returnStatus;
}
/**
 * Write the world epilogue.
 * This basically calls RiWorldEnd().
 */
MStatus liqRibTranslator::worldEpilogue()
{
  MStatus returnStatus = MS::kSuccess;
  LIQDEBUGPRINTF( "-> Writing world epilogue.\n" );
  // If this is a readArchive that we are exporting there's no world block
  if ( !m_exportReadArchive ) RiWorldEnd();
  return returnStatus;
}
/**
 * Write all coordinate systems.
 * This writes all user-defined coordinate systems as well as those required
 * for environment/reflection map lookup and texture projection.
 */
MStatus liqRibTranslator::coordSysBlock()
{
  MStatus returnStatus = MS::kSuccess;
  LIQDEBUGPRINTF( "-> Writing coordinate systems.\n" );
  if ( !m_exportReadArchive )
	{
    RNMAP::iterator rniter;
    for ( rniter = htable->RibNodeMap.begin(); rniter != htable->RibNodeMap.end(); rniter++ ) 
    {
      LIQ_CHECK_CANCEL_REQUEST;
      liqRibNodePtr   ribNode = (*rniter).second;
      if ( ribNode->object(0)->ignore || ribNode->object(0)->type != MRT_Coord ) continue;
      if ( m_outputComments ) RiArchiveRecord( RI_COMMENT, "Name: %s", ribNode->name.asChar(), RI_NULL );

      RiAttributeBegin();
      RiAttribute( "identifier", "name", &getLiquidRibName( ribNode->name.asChar() ), RI_NULL );

      RtMatrix ribMatrix;
      MMatrix matrix;
      MDagPath path;

      matrix = ribNode->object(0)->matrix( path.instanceNumber() );
      matrix.get( ribMatrix );
      if ( liqglo_relativeTransforms ) RiConcatTransform( ribMatrix ); 
      else                             RiTransform( ribMatrix );

      ribNode->object(0)->writeObject();
      ribNode->object(0)->written = 1;
      RiAttributeEnd();
    }
  }
  return returnStatus;
}
/**
 * Write all lights.
 * This writes all lightsource shaders with their attributes and switches them
 * on afterwards.
 */
MStatus liqRibTranslator::lightBlock()
{
	MStatus returnStatus = MS::kSuccess;
	LIQDEBUGPRINTF( "-> Writing lights.\n" );
	// If this is a readArchive that we are exporting then ignore this header information
	if ( !m_exportReadArchive )
	{
		RNMAP::iterator rniter;
		int nbLight = 0;
		for ( rniter = htable->RibNodeMap.begin(); rniter != htable->RibNodeMap.end(); rniter++ )
		{
			RtInt on( 1 );
      LIQ_CHECK_CANCEL_REQUEST;
			liqRibNodePtr   ribNode = (*rniter).second;

			if ( ribNode->object(0)->ignore || ribNode->object(0)->type != MRT_Light ) continue;
			// We need to enclose lights in attribute blocks because of the
			// new added attribute support
			RiAttributeBegin();

			// All this stuff below should be handled by a new attribute class
			LIQDEBUGPRINTF( "-> RibNodeName " );
			RtString RibNodeName = getLiquidRibName( ribNode->name.asChar() );
			LIQDEBUGPRINTF( "= %s.\n", (char *)RibNodeName  );
			RiAttribute( "identifier", "name", &RibNodeName, RI_NULL );
			if ( ribNode->trace.sampleMotion ) RiAttribute( "trace", (RtToken) "samplemotion", &on, RI_NULL );
			if ( ribNode->trace.displacements ) RiAttribute( "trace", (RtToken) "displacements", &on, RI_NULL );
			if ( ribNode->trace.bias != 0.01f )
			{
				RtFloat bias = ribNode->trace.bias;
				RiAttribute( "trace", (RtToken) "bias", &bias, RI_NULL );
			}
			if ( ribNode->trace.maxDiffuseDepth != 1 )
			{
				RtInt mddepth = ribNode->trace.maxDiffuseDepth;
				RiAttribute( "trace", (RtToken) "maxdiffusedepth", &mddepth, RI_NULL );
			}
			if ( ribNode->trace.maxSpecularDepth != 2 )
			{
				RtInt msdepth = ribNode->trace.maxSpecularDepth;
				RiAttribute( "trace", (RtToken) "maxspeculardepth", &msdepth, RI_NULL );
			}
 
			ribNode->object(0)->writeObject();
			ribNode->object(0)->written = 1;
			// The next line pops the light...
			RiAttributeEnd();
			// ...so we have to switch it on again explicitly
			// if exclusive Lightlinking is set
			if ( m_illuminateByDefault ) RiIlluminate( ribNode->object(0)->lightHandle(), 1 );
			else                  				RiIlluminate( ribNode->object(0)->lightHandle(), 0 );
			nbLight++;
		}
	}
	return returnStatus;
}

void liqRibTranslator::setOutDirs()
{
  MStatus gStatus;
  MString varVal;
  MFnDependencyNode rGlobalNode( rGlobalObj );
  
  liquidGetPlugValue( rGlobalNode, "ribDirectory", varVal, gStatus );
  if ( gStatus == MS::kSuccess ) 
    if ( varVal != "" ) 
      liqglo_ribDir = removeEscapes( parseString( varVal, false ) );  
  
  liquidGetPlugValue( rGlobalNode, "textureDirectory", varVal, gStatus );
  if ( gStatus == MS::kSuccess ) 
    if ( varVal != "" ) 
      liqglo_textureDir = removeEscapes( parseString( varVal, false ) );  
    
  liquidGetPlugValue( rGlobalNode, "pictureDirectory", varVal, gStatus );
  if ( gStatus == MS::kSuccess ) 
    if ( varVal != "" ) 
       m_pixDir = removeEscapes( parseString( varVal, false ) );  
}

void liqRibTranslator::setSearchPaths()
{
  liqglo_shaderPath = "&:@:.:~:rmanshader";
  liqglo_texturePath = "&:@:.:~:rmantex";
  liqglo_archivePath = "&:@:.:~:rib";
  liqglo_proceduralPath = "&:@:.:~";

  MString tmphome( getenv( "LIQUIDHOME" ) );
  tmphome = liquidSanitizeSearchPath( tmphome );

  if ( tmphome != "" ) 
  {
    liqglo_shaderPath += ":" + tmphome + "/lib/shaders";
    liqglo_texturePath += ":" + tmphome + "/lib/textures";
    liqglo_archivePath += ":" + tmphome + "/lib/rib";
    liqglo_proceduralPath += ":" + tmphome + "/lib/plugins";
  }
  
  liqglo_shaderPath += ":" + liqglo_projectDir;
  liqglo_texturePath += ":" + liqglo_projectDir;
  liqglo_archivePath += ":" + liqglo_projectDir;
  liqglo_proceduralPath += ":" + liqglo_projectDir;

  MStatus gStatus;
  MString varVal;
  MFnDependencyNode rGlobalNode( rGlobalObj );

  liquidGetPlugValue( rGlobalNode, "shaderPath", varVal, gStatus );
  if ( gStatus == MS::kSuccess && varVal != "" ) 
  {
    MString str = removeEscapes( parseString( varVal, false ) );
    if ( varVal.index( ':' ) == 0 )
      liqglo_shaderPath += str;
    else if ( varVal.rindex( ':' ) == ( varVal.length() - 1 ) )
      liqglo_shaderPath = str + liqglo_shaderPath;
    else
      liqglo_shaderPath = str;
  }
  liquidGetPlugValue( rGlobalNode, "texturePath", varVal, gStatus );
  if ( gStatus == MS::kSuccess && varVal != "" ) 
  {
    MString str = removeEscapes( parseString( varVal, false ) );
    if ( varVal.index( ':' ) == 0 )
      liqglo_texturePath += str;
    else if ( varVal.rindex( ':' ) == ( varVal.length() - 1 ) )
      liqglo_texturePath = str + liqglo_texturePath;
    else
      liqglo_texturePath = str;
  }
  liquidGetPlugValue( rGlobalNode, "archivePath", varVal, gStatus );
  if ( gStatus == MS::kSuccess && varVal != "" ) 
  {
    MString str = removeEscapes( parseString( varVal, false ) );
    if ( varVal.index( ':' ) == 0 )
      liqglo_archivePath += str;
    else if ( varVal.rindex( ':' ) == ( varVal.length() - 1 ) )
      liqglo_archivePath = str + liqglo_archivePath;
    else
      liqglo_archivePath = str;
  }
  liquidGetPlugValue( rGlobalNode, "proceduralPath", varVal, gStatus );
  if ( gStatus == MS::kSuccess && varVal != "" ) 
  {
    MString str = removeEscapes( parseString( varVal, false ) );
    if ( varVal.index( ':' ) == 0 )
      liqglo_proceduralPath += str;
    else if ( varVal.rindex( ':' ) == ( varVal.length() - 1 ) )
      liqglo_proceduralPath = str + liqglo_proceduralPath;
    else
      liqglo_proceduralPath = str;
  }
}

bool liqRibTranslator::renderFrameSort( const structJob& a, const structJob& b )
{
  long v1 = ( a.isShadow )? a.renderFrame : 100000000;
  long v2 = ( b.isShadow )? b.renderFrame : 100000000;
  return v1 < v2;
}

MString liqRibTranslator::getHiderOptions( MString rendername, MString hidername )
{
  stringstream ss;
  // PRMAN
  if ( rendername == "PRMan" ) 
  {
    if ( hidername == "hidden" ) 
    {
      ss << "\"int jitter\" [" << m_hiddenJitter << "] ";

      // PRMAN 13 BEGIN
      if ( m_hiddenAperture[0] != 0.0 ||
          m_hiddenAperture[1] != 0.0 ||
          m_hiddenAperture[2] != 0.0 ||
          m_hiddenAperture[3] != 0.0 ) 
        ss << "\"float aperture[4]\" [" << m_hiddenAperture[0] << " " << m_hiddenAperture[1] << " " << m_hiddenAperture[2] << " " << m_hiddenAperture[3] << "] ";

      if ( m_hiddenShutterOpening[0] != 0.0 && m_hiddenShutterOpening[1] != 1.0 ) 
        ss << "\"float[2] shutteropening\" ["<< m_hiddenShutterOpening[0] << " " << m_hiddenShutterOpening[1] << "] ";
      // PRMAN 13 END
      
      if ( m_hiddenOcclusionBound != 0.0 ) ss << "\"occlusionbound\" [" << m_hiddenOcclusionBound << "] ";
      if ( m_hiddenMpCache != true )       ss << "\"int mpcache\" [0] ";
      if ( m_hiddenMpMemory != 6144 )      ss << "\"mpcache\" [" << m_hiddenMpMemory << "] ";
      if ( m_hiddenMpCacheDir != "" )      ss << "\"mpcachedir\" [\"" << m_hiddenMpCacheDir.asChar() << "\"] ";
      if ( m_hiddenSampleMotion != true )  ss << "\"int samplemotion\" [0] ";
      if ( m_hiddenSubPixel != 1 )         ss << "\"subpixel\" [" << m_hiddenSubPixel << "] ";
      if ( m_hiddenExtremeMotionDof != false ) ss << "\"extrememotiondof\" [1] ";
      if ( m_hiddenMaxVPDepth != -1 )      ss << "\"maxvpdepth\" [" << m_hiddenMaxVPDepth << "] ";

      // PRMAN 13 BEGIN
      if ( m_hiddenSigma != false )        ss << "\"int sigma\" [1] " << "\"sigmablur\" [" << m_hiddenSigmaBlur << "] ";
      // PRMAN 13 END
    } 
    else if ( hidername == "photon" ) 
    {
      if ( m_photonEmit != 0 ) ss << " \"int emit\" [" << m_photonEmit << "] ";
    } 
    else if ( hidername == "depthmask" ) 
    {
      ss << "\"zfile\" [\"" << m_depthMaskZFile.asChar() << "\"] ";
      ss << "\"reversesign\" [\"" << m_depthMaskReverseSign << "\"] ";
      ss << "\"depthbias\" [" << m_depthMaskDepthBias << "] ";
    }
  }
  // 3DELIGHT
  if ( rendername == "3Delight" ) 
  {
    if ( hidername == "hidden" ) 
    {
      ss << "\"jitter\" [" << m_hiddenJitter << "] ";
      if ( m_hiddenSampleMotion != true )      ss << "\"int samplemotion\" [0] ";
      if ( m_hiddenExtremeMotionDof != false ) ss << "\"int extrememotiondof\" [1] ";
    }
    else if ( hidername == "photon" ) 
    {
      if ( m_photonEmit != 0 ) ss << " \"int emit\" [" << m_photonEmit << "] ";
    } 
  }
  // PIXIE
  if ( rendername == "Pixie" ) 
  {
    if ( hidername == "hidden" ) ss << "\"float jitter\" [" << m_hiddenJitter << "] ";
    else if ( hidername == "raytrace" ) 
      if ( m_raytraceFalseColor != 0 ) ss << "\"int falsecolor\" [1] ";
    else if ( hidername == "photon" ) 
    {
      if ( m_photonEmit != 0 ) ss << " \"int emit\" [" << m_photonEmit << "] ";
      if ( m_photonSampleSpectrum ) ss << " \"int samplespectrum\" [1] ";
    }
  }
  // AQSIS
  if ( rendername == "Aqsis" ) 
  {
    // no known options
  }
  // AIR
  if ( rendername == "Air" ) 
  {
    // no known options
  }
  MString options( ss.str().c_str() );
  return options;
}
