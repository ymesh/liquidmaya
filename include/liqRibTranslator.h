/*
**
** The contents of this file are subject to the Mozilla Public License Version
** 1.1 (the "License"); you may not use this file except in compliance with
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
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#ifndef liqRibTranslator_H
#define liqRibTranslator_H

/* ______________________________________________________________________
**
** Liquid Rib Translator Header File
** ______________________________________________________________________
*/

#include <liquid.h>
#include <liqRenderer.h>
#include <liqRibHT.h>
#include <liqGenericShader.h>
#include <liqRenderScript.h>
#include <liqRibLightData.h>
#include <liqExpression.h>

#include <maya/MPxCommand.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnCamera.h>
#include <maya/MArgList.h>
#include <maya/MFloatArray.h>

#include <map>
#include <boost/shared_ptr.hpp>


using namespace boost;

class liqRibTranslator : public MPxCommand {
public:
  liqRibTranslator();
  ~liqRibTranslator();
  static void * creator();
  static MSyntax syntax();

  MStatus doIt(const MArgList& args );

  friend class liqJobList;

private: // Methods
  MObject rGlobalObj;

  MStatus setRenderLayer( const MArgList & );
  MStatus scanSceneNodes( MObject&, MDagPath &, float, int, int &, MStatus& ); 
  MStatus scanScene(float, int );

  void portFieldOfView( int width, int height, double& horizontal, double& vertical, MFnCamera& fnCamera );
  void computeViewingFrustum( double window_aspect, double& left, double& right, double& bottom, double& top, MFnCamera& cam );
  void getCameraInfo( MFnCamera &cam, structCamera &camStruct );
  MStatus getCameraTransform( MFnCamera& cam, structCamera &camStruct );
  void getCameraFilmOffset( MFnCamera& cam, structCamera &camStruct );
  void setSearchPaths();
  void setOutDirs();
  MString verifyResourceDir( const char *resourceName, MString resourceDir, bool &problem );
  bool verifyOutputDirectories();

	void exportJobCamera(const structJob &job, const structCamera camera[]);
 
 	// rib output functions 
	MStatus liquidDoArgs( MArgList args );
  bool liquidInitGlobals();
  void liquidReadGlobals();
  MString getHiderOptions( MString rendername, MString hidername );

  MStatus ribOutput( long scanTime, MString ribName, bool world_only, bool out_lightBlock, MString archiveName );

  MStatus buildJobs();
  MStatus ribPrologue();
  MStatus ribEpilogue();
  MStatus framePrologue( long );
  MStatus worldPrologue();
  MStatus lightBlock();
  MStatus coordSysBlock();
  MStatus objectBlock();
  MStatus worldEpilogue();
  MStatus frameEpilogue( long );
  void doAttributeBlocking( const MDagPath & newPath,  const MDagPath & previousPath );
  void printProgress ( unsigned stat, unsigned frames, unsigned where );

  MString generateRenderScriptName()  const;
  MString generateTempMayaSceneName() const;
  MString generateFileName( fileGenMode mode, const structJob& job );
  string  generateImageName( MString aovName, const structJob& job, MString format );
  // MString generateShadowArchiveName( bool renderAllFrames, long renderAtframe, MString geometrySet );
  static bool renderFrameSort( const structJob& a, const structJob& b );

public: // Data
  static MString magic;

  enum MRibStatus {
    kRibOK,
    kRibBegin,
    kRibFrame,
    kRibWorld,
    kRibError
  };
  MRibStatus ribStatus;

  
  // Data used to construct output file names
  MString     outFormat;
  MString     outExt;

  MString     extension;
  MString     imageName;

   
  MString     baseShadowName; // shadow rib archive name
  MString     baseSceneName;  // scene rib archive name

  // Directories
  bool        createOutputDirectories;
  // bool        m_noDirCheck; // UNUSED ???
  MString     m_systemTempDirectory;
  MString     m_tmpDir;
  MString     m_pixDir;

  // Camera
  MDagPath m_camDagPath;
  bool liqglo_rotateCamera;   // rotates the camera for sideways renderings
  bool m_isStereoCamera;

  // Animation
  bool          m_animation;
  bool          m_useFrameExt;
  vector< int > frameNumbers;
  
  // Rib
  bool        m_justRib;
  // MString     m_beautyRibFile;  // UNUSED ???
  bool m_exportReadArchive;

  // Shaders
  bool m_shaderDebug;
  bool m_illuminateByDefault;
  bool m_liquidSetLightLinking;

  bool m_ignoreLights;
  bool m_ignoreSurfaces;
  bool m_ignoreDisplacements;
  bool m_ignoreVolumes;

  bool m_skipVisibilityAttributes;
  bool m_skipShadingAttributes;
  bool m_skipRayTraceAttributes;
 

  // Shadows
  bool m_lazyCompute;
  bool m_outputShadersInShadows;
  bool m_outputShadersInDeepShadows;
  bool m_outputLightsInDeepShadows;

  // Ribgen
  bool m_deferredGen;
  liquidlong m_deferredBlockSize;
  MString m_defGenKey;
  MString m_defGenService;

  // Render
  bool launchRender;
  bool remoteRender;
  bool useNetRman;
  liquidlong    m_minCPU;
  liquidlong    m_maxCPU;
  bool m_showProgress;

  // Script Job
  vector<structJob>  jobList;
  vector<structJob>  shadowList;

  vector<structJob>  refList;  // reflections list
  vector<structJob>  envList;  // environments list
  vector<structJob>  txtList;  // make textures list

  

  bool useRenderScript;
  MString renderJobName;
  MString m_userRenderScriptFileName;
  MString m_renderScriptCommand;
  enum renderScriptFormat {
    NONE   = 0,
    ALFRED = 1,
    XML    = 2 }
  m_renderScriptFormat;

  MString m_dirmaps;

  bool cleanRib;              // clean the rib files up
  bool cleanRenderScript;
  bool cleanShadows;          // UN-USED GLOBAL
  bool cleanTextures;         // UN-USED GLOBAL
  
  // alfred stuff
  bool m_alfShadowRibGen; // not stored in Globals   

  MString m_alfredTags;
  MString m_alfredServices;
  bool m_alfredExpand;

  
  bool fullShadowRib;

  // Image
  liquidlong width, height, depth;
  float       aspectRatio;
  double        m_cropX1,
                m_cropX2,
                m_cropY1,
                m_cropY2;

  float         m_rgain,
                m_rgamma;

  // REYES
  bool doDof;                 // do camera depth of field

  // Motion blur
  double        m_blurTime;
  bool doCameraMotion;        // Motion blur for moving cameras
  
  enum shutterConfig {
    OPEN_ON_FRAME         = 0,
    CENTER_ON_FRAME       = 1,
    CENTER_BETWEEN_FRAMES = 2,
    CLOSE_ON_NEXT_FRAME   = 3
  } shutterConfig;
 
  liquidlong  pixelSamples;
  float      shadingRate;
  liquidlong  bucketSize[2];
  liquidlong  gridSize;
  liquidlong  textureMemory;
  liquidlong  eyeSplits;
  MVector     othreshold;
  MVector     zthreshold;

  bool       ignoreFilmGate;
  
  liquidlong  quantValue;

  bool m_outputComments;


  struct MStringCmp
  {
    bool operator() (const MString &a, const MString &b) const
    {
      return strcmp( a.asChar(), b.asChar() ) < 0;
    }
  };
  std::map<MString, MString, MStringCmp> m_shadowRibFile;

  // Hash table for scene
  boost::shared_ptr< liqRibHT > htable;

  // Depth in attribute blocking
  // NOTE : used in liqRibTranslator::doAttributeBlocking,
  // but this method isn't called anywhere.
  int attributeDepth;

  
  bool m_renderSelected;

  // UNUSED ???
  bool m_outputShadowPass;
  bool m_outputHeroPass;
  
  // long m_currentLiquidJobNumber; 

private :
  M3dView       m_activeView;

  int           m_errorMode;
  MComputation  m_escHandler;

#ifdef _WIN32
  int RiNColorSamples;
#endif
  /*
  // these are little storage variables to keep track of the current graphics state and will eventually be wrapped in
  // a specific class
  struct globals{
    struct feedback{
      bool showProgress;
      bool outputDetailedComments;
      bool shaderDebugging;
    } feedback;

    struct shadowMaps{
      bool noShadowRibs;
      bool fullShadowRibs;
      bool lazyCompute;
    } shadowMaps;

    struct depthShadows{
      bool opacityThreshold;
      bool outputAllShaders;
    } depthShadows;

    struct deepShadows{
      bool outputAllShaders;
      bool outputLights;
    } deepShadows;

    struct rib{
      bool readArchiveable;

      struct output{
        bool allCurves;
        bool meshUVs;
      } output;

      struct shaders{
        bool noSurfaces;
        bool noLights;
        bool noDisplacements;
        bool noVolumes;
        bool expandArrays;
      } shaders;

      struct paths{
        bool projectRelative;
        bool shaders;
      } paths;

      struct format{
        bool binary;
        bool gZip;
      } format;

      struct box{
        MString preWorld;
        MString postWorld;
        MString preObject;
      } box;

    } rib;

  } globals;
  */
  
  // bool m_currentMatteMode; // UNUSED ???

  
  
  bool m_renderAllCurves;
  
  MString m_preFrameMel;
  MString m_postFrameMel;

  MString m_renderCommand;
  MString m_ribgenCommand;
  MString m_preCommand;

  MString m_preJobCommand;
  MString m_postJobCommand;
  MString m_preFrameCommand;
  MString m_postFrameCommand;

  MString m_preTransformMel;
  MString m_postTransformMel;
  MString m_preShapeMel;
  MString m_postShapeMel;

  MString m_shaderPath;

  bool	  m_bakeNonRasterOrient;
  bool	  m_bakeNoCullBackface;
  bool	  m_bakeNoCullHidden;

  MString m_preFrameRIB;
  MString m_preWorldRIB;
  MString m_postWorldRIB;

  MString m_preGeomRIB;
  
  MString originalLayer;

  // Display Driver Variables
  typedef struct structDDParam {
    liquidlong    num;
    MStringArray  names;
    MStringArray  data;
    MIntArray     type;
  } structDDParam;

  bool          m_ignoreAOVDisplays;

  typedef struct structDisplay {
    MString         name;
    MString         type;
    MString         mode;
    bool            enabled;
    bool            doQuantize;
    int             bitDepth;
    float           dither;
    bool            doFilter;
    int             filter;
    float           filterX;
    float           filterY;
    structDDParam   xtraParams;
  } structDisplay;
  vector<structDisplay> m_displays;

  typedef struct structChannel {
    MString     name;
    int         type;
    int         arraySize;
    bool        quantize;
    int         bitDepth;
    float       dither;
    bool        filter;
    int         pixelFilter;
    float       pixelFilterX;
    float       pixelFilterY;
  } structChannel;
  vector<structChannel> m_channels;

  // MStringArray  m_pixelFilterNames;
  liquidlong    m_rFilter;
  float         m_rFilterX, m_rFilterY;

  bool          m_renderView;
  bool          m_renderViewCrop;
  bool          m_renderViewLocal;
  liquidlong    m_renderViewPort;
  liquidlong    m_renderViewTimeOut;

  int           m_statistics;
  MString       m_statisticsFile;

  int           m_hiddenJitter;
  // PRMAN 13 BEGIN
  float         m_hiddenAperture[4];
  float         m_hiddenShutterOpening[2];
  // PRMAN 13 END
  float         m_hiddenOcclusionBound;
  bool          m_hiddenMpCache;
  int           m_hiddenMpMemory;
  MString       m_hiddenMpCacheDir;
  bool          m_hiddenSampleMotion;
  int           m_hiddenSubPixel;
  bool          m_hiddenExtremeMotionDof;
  int           m_hiddenMaxVPDepth;
  // PRMAN 13 BEGIN
  bool          m_hiddenSigma;
  float         m_hiddenSigmaBlur;
  // PRMAN 13 END
  int           m_raytraceFalseColor;
  int           m_photonEmit;
  int           m_photonSampleSpectrum;

  MString       m_depthMaskZFile;
  bool          m_depthMaskReverseSign;
  float         m_depthMaskDepthBias;

  //vector<liqShader> m_shaders;

  //liqShader & liqGetShader( MObject shaderObj );
  // MStatus liqShaderParseVectorAttr ( liqShader & currentShader, MFnDependencyNode & shaderNode, const char * argName, ParameterType pType );
  //void freeShaders( void );
  
  MStringArray m_objectListToExport;
  bool m_exportSpecificList;
  bool m_exportOnlyObjectBlock;
  
  
};

#endif
