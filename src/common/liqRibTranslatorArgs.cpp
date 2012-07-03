// Maya headers
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MAnimControl.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnIntArrayData.h>

// Liquid headers
#include <liquid.h>
#include <liqRibTranslator.h>
#include <liqGlobalHelpers.h>

extern bool liquidBin;
extern int  debugMode;

extern MString  liqglo_renderCamera;
extern MString  liqglo_projectDir;
extern MString  liqglo_sceneName;
extern MString  liqglo_ribDir;
extern MString  liqglo_textureDir;

extern MString  liqglo_shotName;
extern MString  liqglo_shotVersion;

extern bool     liqglo_doBinary; 
extern bool     liqglo_doShadows; 
extern bool     liqglo_doCompression; 
extern bool     liqglo_doMotion; 
extern bool     liqglo_doDef;
extern bool     liqglo_relativeMotion;
extern bool     liqglo_relativeFileNames;
extern bool     liqglo_noSingleFrameShadows;
extern bool     liqglo_singleFrameShadowsOnly;

extern bool     liqglo_exportAllShadersParams;
extern bool     liqglo_beautyRibHasCameraName;
extern bool     liqglo_skipDefaultMatte;

extern int      liqglo_motionSamples;
extern int      liqglo_outPadding;


/**
 * Syntax defintion
 */
MSyntax liqRibTranslator::syntax()
{
  MSyntax syntax;

  syntax.addFlag("lr",    "launchRender");
  syntax.addFlag("nolr",  "noLaunchRender");
  syntax.addFlag("GL",    "useGlobals");
  syntax.addFlag("sel",   "selected");
  syntax.addFlag("ra",    "readArchive");
  syntax.addFlag("acv",   "allCurves");
  syntax.addFlag("tif",   "tiff");
  syntax.addFlag("dof",   "dofOn");
  syntax.addFlag("bin",   "doBinary");
  syntax.addFlag("sh",    "shadows");
  syntax.addFlag("nsh",   "noShadows");
  syntax.addFlag("zip",   "doCompression");
  syntax.addFlag("cln",   "cleanRib");
  syntax.addFlag("pro",   "progress");
  syntax.addFlag("mb",    "motionBlur");
  syntax.addFlag("rmot",  "relativeMotion");
  syntax.addFlag("db",    "deformationBlur");
  syntax.addFlag("d",     "debug");
  syntax.addFlag("net",   "netRender");
  syntax.addFlag("fsr",   "fullShadowRib");
  syntax.addFlag("rem",   "remote");
  syntax.addFlag("rs",    "renderScript");
  syntax.addFlag("nrs",   "noRenderScript");
  syntax.addFlag("err",   "errHandler");
  syntax.addFlag("sdb",   "shaderDebug");
  syntax.addFlag("n",     "sequence",         MSyntax::kLong, MSyntax::kLong, MSyntax::kLong);
  syntax.addFlag("fl",    "frameList",        MSyntax::kString);
  syntax.addFlag("m",     "mbSamples",        MSyntax::kLong);
  syntax.addFlag("dbs",   "defBlock");
  syntax.addFlag("cam",   "camera",           MSyntax::kString);
  syntax.addFlag("rcam",  "rotateCamera");
  syntax.addFlag("s",     "samples",          MSyntax::kLong);
  syntax.addFlag("rnm",   "ribName",          MSyntax::kString);
  syntax.addFlag("pd",    "projectDir",       MSyntax::kString);
  syntax.addFlag("rel",   "relativeDirs");
  syntax.addFlag("prm",   "preFrameMel",      MSyntax::kString);
  syntax.addFlag("pom",   "postFrameMel",     MSyntax::kString);
  syntax.addFlag("rid",   "ribdir",           MSyntax::kString);
  syntax.addFlag("txd",   "texdir",           MSyntax::kString);
  syntax.addFlag("tmd",   "tmpdir",           MSyntax::kString);
  syntax.addFlag("pid",   "picdir",           MSyntax::kString);
  syntax.addFlag("pec",   "preCommand",       MSyntax::kString);
  syntax.addFlag("poc",   "postJobCommand",   MSyntax::kString);
  syntax.addFlag("pof",   "postFrameCommand", MSyntax::kString);
  syntax.addFlag("prf",   "preFrameCommand",  MSyntax::kString);
  syntax.addFlag("rec",   "renderCommand",    MSyntax::kString);
  syntax.addFlag("rgc",   "ribgenCommand",    MSyntax::kString);
  syntax.addFlag("blt",   "blurTime",         MSyntax::kDouble);
  syntax.addFlag("sr",    "shadingRate",      MSyntax::kDouble);
  syntax.addFlag("bs",    "bucketSize",       MSyntax::kLong, MSyntax::kLong);
  syntax.addFlag("pf",    "pixelFilter",      MSyntax::kLong, MSyntax::kLong, MSyntax::kLong);
  syntax.addFlag("gs",    "gridSize",         MSyntax::kLong);
  syntax.addFlag("txm",   "texmem",           MSyntax::kLong);
  syntax.addFlag("es",    "eyeSplits",        MSyntax::kLong);
  syntax.addFlag("ar",    "aspect",           MSyntax::kDouble);
  syntax.addFlag("x",     "width",            MSyntax::kLong);
  syntax.addFlag("y",     "height",           MSyntax::kLong);
  syntax.addFlag("cw",    "cropWindow",       MSyntax::kLong, MSyntax::kLong, MSyntax::kLong, MSyntax::kLong);
  syntax.addFlag("def",   "deferred");
  syntax.addFlag("ndf",   "noDef");
  syntax.addFlag("pad",   "padding",          MSyntax::kLong);
  syntax.addFlag("rgo",   "ribGenOnly");
  syntax.addFlag("sfso",  "singleFrameShadowsOnly");
  syntax.addFlag("nsfs",  "noSingleFrameShadows");
  syntax.addFlag("rv",    "renderView");
  syntax.addFlag("rvl",   "renderViewlocal");
  syntax.addFlag("rvp",   "renderViewPort",  MSyntax::kLong);
  syntax.addFlag("shn",   "shotName",        MSyntax::kString);
  syntax.addFlag("shv",   "shotVersion",     MSyntax::kString);
  syntax.addFlag("lyr",   "layer",           MSyntax::kString);

  syntax.addFlag("obl",   "objectList", MSyntax::kString, MSyntax::kString, MSyntax::kString, MSyntax::kString, MSyntax::kString, MSyntax::kString);
  syntax.addFlag("oob",   "onlyObjectBlock");
  syntax.addFlag("igs",   "ignoreSurfaces");
  syntax.addFlag("no_igs","noIgnoreSurfaces");
  syntax.addFlag("igd",   "ignoreDisplacements");
  syntax.addFlag("no_igd","noIgnoreDisplacements");
  syntax.addFlag("igv",   "ignoreVolumes");
  syntax.addFlag("no_igv","noIgnoreVolumes");
  syntax.addFlag("no_ufe","noUseFrameExtension");
  syntax.addFlag("skv",   "skipVisibilityAttributes");
  syntax.addFlag("sks",   "skipShadingAttributes");
  syntax.addFlag("skr",   "skipRayTraceAttributes");
  syntax.addFlag("easp",   "exportAllShadersParams");
  syntax.addFlag("rhcn",   "ribHasCameraName");
  return syntax;
}

/**
 * Read the values from the command line and set the internal values.
 */
MStatus liqRibTranslator::liquidDoArgs( MArgList args )
{
  MStatus status;
  MString argValue;

  LIQDEBUGPRINTF( "-> processing arguments\n" );

  // Parse the arguments and set the options.
  if ( args.length() == 0 ) 
  {
    liquidMessage( "Doing nothing, no parameters given", messageError );
    return MS::kFailure;
  }
  // find the activeView for previews;
  m_activeView = M3dView::active3dView();
  width        = m_activeView.portWidth();
  height       = m_activeView.portHeight();

  // get the current project directory
  MString MELCommand = "workspace -q -rd";
  MString MELReturn;
  MGlobal::executeCommand( MELCommand, MELReturn );
  liqglo_projectDir = MELReturn;

  // get the current scene name
  liqglo_sceneName = liquidTransGetSceneName();

  // setup default animation parameters
  frameNumbers.push_back( ( int )MAnimControl::currentTime().as( MTime::uiUnit() ) );

  // check to see if the correct project directory was found
  /*if( !fileFullyAccessible( liqglo_projectDir ) ) {
    liqglo_projectDir = m_systemTempDirectory;
    liquidMessage( "Trying to set project directory to system temp directory '" + string( liqglo_projectDir.asChar() ) + "'.", messageWarning );
  }*/
  LIQ_ADD_SLASH_IF_NEEDED( liqglo_projectDir );
  if ( !fileFullyAccessible( liqglo_projectDir ) ) 
  {
    liquidMessage( "Cannot find project directory, '" + string( liqglo_projectDir.asChar() ) + "'. Defaulting to system temp directory!", messageWarning );
    liqglo_projectDir = m_systemTempDirectory;
  }

  bool GL_read( false );

  for ( unsigned i( 0 ); i < args.length() ; i++ ) 
  {
    MString arg = args.asString( i, &status );
    MString err, err_fmt = "error in ^1s parameter";
    err.format( err_fmt, arg);
    LIQCHECKSTATUS(status, err );

    if ( (arg == "-GL") || (arg == "-useGlobals") ) 
    {
      //load up all the render global parameters!
      if ( liquidInitGlobals() && !GL_read )  liquidReadGlobals();
      GL_read = true;
    } 
    else if ((arg == "-lr") || (arg == "-launchRender"))     launchRender = true;
    else if ((arg == "-nolr") || (arg == "-noLaunchRender")) launchRender = false;
    else if ((arg == "-sel") || (arg == "-selected"))        m_renderSelected = true;
    else if ((arg == "-ra") || (arg == "-readArchive"))      m_exportReadArchive = true;
    else if ((arg == "-acv") || (arg == "-allCurves"))       m_renderAllCurves = true;
    else if ((arg == "-tif") || (arg == "-tiff"))            outFormat = "tiff";
    else if ((arg == "-dof") || (arg == "-dofOn"))           doDof = true;
    else if ((arg == "-bin") || (arg == "-doBinary"))        liqglo_doBinary = true;
    else if ((arg == "-sh") || (arg == "-shadows"))          liqglo_doShadows = true;
    else if ((arg == "-nsh") || (arg == "-noShadows"))       liqglo_doShadows = false;
    else if ((arg == "-zip") || (arg == "-doCompression"))   liqglo_doCompression = true;
    else if ((arg == "-cln") || (arg == "-cleanRib"))        cleanRib = true;
    else if ((arg == "-cmd") || (arg == "-createMissingDirs")) createOutputDirectories = true;
    else if ((arg == "-pro") || (arg == "-progress"))        m_showProgress = true;
    else if ((arg == "-mb") || (arg == "-motionBlur"))       liqglo_doMotion = true;
    else if ((arg == "-db") || (arg == "-deformationBlur"))  liqglo_doDef = true;
    else if ((arg == "-d") || (arg == "-debug"))             debugMode = 1;
    else if ((arg == "-net") || (arg == "-netRender"))       useNetRman = true;
    else if ((arg == "-fsr") || (arg == "-fullShadowRib"))   fullShadowRib = true;
    else if ((arg == "-rem") || (arg == "-remote"))          remoteRender = true;
    else if ((arg == "-rs") || (arg == "-renderScript"))     useRenderScript = true;
    else if ((arg == "-nrs") || (arg == "-noRenderScript"))  useRenderScript = false;
    else if ((arg == "-err") || (arg == "-errHandler"))      m_errorMode = 1;
    else if ((arg == "-sdb") || (arg == "-shaderDebug"))     m_shaderDebug = true;
    else if ((arg == "-rmot") || (arg == "-relativeMotion")) liqglo_relativeMotion = true;
    else if ((arg == "-rcam") || (arg == "-rotateCamera"))   liqglo_rotateCamera = true;
    else if ((arg == "-rel") || (arg == "-relativeDirs"))    liqglo_relativeFileNames = true;
    else if ((arg == "-def") || (arg == "-deferred"))        m_deferredGen = true;
    else if ((arg == "-ndf") || (arg == "-noDef"))           m_deferredGen = false;
    else if ((arg == "-rgo") || (arg == "-ribGenOnly"))      m_justRib = true;
    else if ((arg == "-rv") || (arg == "-renderView"))       m_renderView = true;
    else if ((arg == "-rvl") || (arg == "-renderViewLocal")) m_renderViewLocal = true;
    else if ((arg == "-nsfs") || (arg == "-noSingleFrameShadows"))   liqglo_noSingleFrameShadows = true;
    else if ((arg == "-sfso") || (arg == "-singleFrameShadowsOnly")) liqglo_singleFrameShadowsOnly = true;
    else if ((arg == "-n") || (arg == "-sequence")) 
    {
      argValue = args.asString( ++i, &status );
      int first( argValue.asInt() );
      LIQCHECKSTATUS(status, err);
      argValue = args.asString( ++i, &status );
      int last( argValue.asInt() );
      LIQCHECKSTATUS(status, err);
      argValue = args.asString( ++i, &status );
      int step( argValue.asInt() );
      LIQCHECKSTATUS(status, err);
      m_animation = true;
      if ( first > last ) step = -abs( step );
      // Fill our vector with frames
      frameNumbers.clear();
      for ( int frame( first ); frame <= last; frame += step ) frameNumbers.push_back( frame );
    } 
    else if ((arg == "-fl") || (arg == "-frameList") ) 
    {
      argValue = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
      // fill our vector with frames
      frameNumbers = generateFrameNumbers( string( argValue.asChar() ) );
      if ( frameNumbers.size() )  
        m_animation = true;
    } 
    else if ((arg == "-m") || (arg == "-mbSamples")) 
    {
      argValue = args.asString( ++i, &status );
      liqglo_motionSamples = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-dbs") || (arg == "-defBlock")) 
    {
      argValue = args.asString( ++i, &status );
      m_deferredBlockSize = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-cam") || (arg == "-camera")) 
    {
      MString parsingString = args.asString( ++i, &status );
      liqglo_renderCamera = parseString( parsingString );
      LIQCHECKSTATUS(status,err);
    } 
    else if ((arg == "-s") || (arg == "-samples")) 
    {
      argValue = args.asString( ++i, &status );
      pixelSamples = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-rnm") || (arg == "-ribName")) 
    {
      MString parsingString = args.asString( ++i, &status );
      liqglo_sceneName = parseString( parsingString );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-pd") || (arg == "-projectDir")) 
    {
      MString parsingString = args.asString( ++i, &status );
      liqglo_projectDir = parseString( parsingString, false );
      LIQ_ADD_SLASH_IF_NEEDED( liqglo_projectDir );
#ifdef _WIN32
      int dirMode = 6; // dummy arg
      int mkdirMode = 0;
#else
      mode_t dirMode, mkdirMode;
      dirMode = R_OK|W_OK|X_OK|F_OK;
      mkdirMode = S_IRWXU|S_IRWXG|S_IRWXO;
#endif
      if ( createOutputDirectories ) 
        makeFullPath( liqglo_projectDir.asChar(), mkdirMode );
      
      if ( !fileFullyAccessible( liqglo_projectDir ) ) 
      {
        liquidMessage( "Cannot find or access Maya project directory; defaulting to system temp directory!", messageWarning );
        liqglo_projectDir = m_systemTempDirectory;
      }
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-prm") || (arg == "-preFrameMel")) 
    {
      m_preFrameMel =  args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-pom") || (arg == "-postFrameMel")) 
    {
      m_postFrameMel = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-rid") || (arg == "-ribdir")) 
    {
      MString parsingString = args.asString( ++i, &status );
      liqglo_ribDir = parseString( parsingString, false );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-txd") || (arg == "-texdir")) 
    {
      MString parsingString = args.asString( ++i, &status );
      liqglo_textureDir = parseString( parsingString, false );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-tmd") || (arg == "-tmpdir")) 
    {
      MString parsingString = args.asString( ++i, &status );
      m_tmpDir = parseString( parsingString, false );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-pid") || (arg == "-picdir")) 
    {
      MString parsingString = args.asString( ++i, &status );
      m_pixDir = parseString( parsingString, false );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-pec") || (arg == "-preCommand")) 
    {
      m_preCommand = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-poc") || (arg == "-postJobCommand")) 
    {
      MString varVal = args.asString( ++i, &status );
      m_postJobCommand = parseString( varVal );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-pof") || (arg == "-postFrameCommand")) 
    {
      m_postFrameCommand = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-prf") || (arg == "-preFrameCommand")) 
    {
      m_preFrameCommand = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-rec") || (arg == "-renderCommand")) 
    {
      m_renderCommand = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-rgc") || (arg == "-ribgenCommand")) 
    {
      m_ribgenCommand = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-blt") || (arg == "-blurTime")) 
    {
      argValue = args.asString( ++i, &status );
      m_blurTime = argValue.asDouble();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-sr") || (arg == "-shadingRate")) 
    {
      argValue = args.asString( ++i, &status );
      shadingRate = argValue.asDouble();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-bs") || (arg == "-bucketSize")) 
    {
      argValue = args.asString( ++i, &status );
      bucketSize[0] = argValue.asInt();
      LIQCHECKSTATUS(status, err);
      argValue = args.asString( ++i, &status );
      bucketSize[1] = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-pf") || (arg == "-pixelFilter")) 
    {
      argValue = args.asString( ++i, &status );
      m_rFilter = argValue.asInt();
      LIQCHECKSTATUS(status, err);  
      argValue = args.asString( ++i, &status );
      m_rFilterX = argValue.asInt();
      LIQCHECKSTATUS(status, err);  
      argValue = args.asString( ++i, &status );
      m_rFilterY = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-gs") || (arg == "-gridSize")) 
    {
      argValue = args.asString( ++i, &status );
      gridSize = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-txm") || (arg == "-texmem")) 
    {
      argValue = args.asString( ++i, &status );
      textureMemory = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-es") || (arg == "-eyeSplits")) 
    {
      argValue = args.asString( ++i, &status );
      eyeSplits = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-ar") || (arg == "-aspect")) 
    {
      argValue = args.asString( ++i, &status );
      aspectRatio = argValue.asDouble();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-x") || (arg == "-width")) 
    {
      argValue = args.asString( ++i, &status );
      width = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-y") || (arg == "-height")) 
    {
      argValue = args.asString( ++i, &status );
      height = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-pad") || (arg == "-padding")) 
    {
      argValue = args.asString( ++i, &status );
      liqglo_outPadding = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-rvp") || (arg == "-renderViewPort")) 
    {
      argValue = args.asString( ++i, &status );
      m_renderViewPort = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-cw") || (arg == "-cropWindow")) 
    {
      argValue = args.asString( ++i, &status );
      m_cropX1 = argValue.asDouble();
      LIQCHECKSTATUS(status, err);
      argValue = args.asString( ++i, &status );
      m_cropX2 = argValue.asDouble();
      LIQCHECKSTATUS(status, err);
      argValue = args.asString( ++i, &status );
      m_cropY1 = argValue.asDouble();
      LIQCHECKSTATUS(status, err);
      argValue = args.asString( ++i, &status );
      m_cropY2 = argValue.asDouble();
      LIQCHECKSTATUS(status, err);
      if( m_renderView ) 
        m_renderViewCrop = true;
    } 
    else if ((arg == "-shn") || (arg == "-shotName")) 
    {
      liqglo_shotName = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
    else if ((arg == "-shv") || (arg == "-shotVersion")) 
    {
      liqglo_shotVersion = args.asString( ++i, &status );
      LIQCHECKSTATUS(status, err);
    } 
		else if ((arg == "-obl") || (arg == "-objectList")) 
		{
      m_objectListToExport = args.asStringArray( ++i, &status );
      m_exportSpecificList = true;
      LIQCHECKSTATUS(status, err);
    }
    else if ((arg == "-oob") || (arg == "-onlyObjectBlock")) m_exportOnlyObjectBlock = true;
    else if ((arg == "-igs") || (arg == "-ignoreSurfaces"))  m_ignoreSurfaces = true;
    else if ((arg == "-no_igs") || (arg == "-noIgnoreSurfaces")) m_ignoreSurfaces = false;
    else if ((arg == "-igd") || (arg == "-ignoreDisplacements")) m_ignoreDisplacements = true;
    else if ((arg == "-no_igd") || (arg == "-noIgnoreDisplacements")) m_ignoreDisplacements = false;
    else if ((arg == "-igv") || (arg == "-ignoreVolumes")) m_ignoreVolumes = true;
    else if ((arg == "-no_igv") || (arg == "-noIgnoreVolumes")) m_ignoreVolumes = false;
    else if ((arg == "-no_ufe") || (arg == "-noUseFrameExtension")) m_useFrameExt = false;
    else if ((arg == "-skv") || (arg == "-skipVisibilityAttributes")) m_skipVisibilityAttributes = true;
    else if ((arg == "-sks") || (arg == "-skipShadingAttributes")) m_skipShadingAttributes = true;
    else if ((arg == "-skr") || (arg == "-skipRayTraceAttributes")) m_skipRayTraceAttributes = true;
    else if ((arg == "-easp") || (arg == "-exportAllShadersParams")) 
    {
      argValue = args.asString( ++i, &status );
      liqglo_exportAllShadersParams = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    }
    else if ((arg == "-rhcn") || (arg == "-ribHasCameraName")) 
    {
      argValue = args.asString( ++i, &status );
      liqglo_beautyRibHasCameraName = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    }
    else if ((arg == "-sdm") || (arg == "-skipDefaultMatte")) 
    {
      argValue = args.asString( ++i, &status );
      liqglo_skipDefaultMatte = argValue.asInt();
      LIQCHECKSTATUS(status, err);
    }
    else
    { 
		  stringstream ss;
		  ss << "[liqRibTranslator] undefined argument " << i << " : " << args.asString( i ).asChar() << ends;
		  liquidMessage( ss.str(), messageError );
    }
  }
	
	if ( !m_useFrameExt )
	{
		if ( m_animation )
			liquidMessage( "[liqRibTranslator] useFrameExtension is false and animation was true, set animation=false", messageWarning );
		m_animation = false;
	}

  liquidMessage( "Using project base path '" + string( liqglo_projectDir.asChar() ) + "'", messageInfo );
  setSearchPaths();
  return MS::kSuccess;
}
