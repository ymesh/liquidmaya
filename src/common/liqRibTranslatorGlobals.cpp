// Maya headers
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MAnimControl.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnIntArrayData.h>

// Liquid headers
#include <liquid.h>
#include <liqRibTranslator.h>
#include <liqGlobalHelpers.h>
#include <liqRenderer.h>

extern bool liquidBin;
extern int  debugMode;

extern long         liqglo_lframe;
extern structJob    liqglo_currentJob;
extern bool         liqglo_doMotion;                         // Motion blur for transformations
extern bool         liqglo_doDef;                            // Motion blur for deforming objects
extern bool         liqglo_doCompression;                    // output compressed ribs
extern bool         liqglo_doBinary;                         // output binary ribs
extern bool         liqglo_relativeMotion;                   // Use relative motion blocks
extern RtFloat      liqglo_sampleTimes[LIQMAXMOTIONSAMPLES]; // current sample times
extern RtFloat      liqglo_sampleTimesOffsets[LIQMAXMOTIONSAMPLES]; // current sample times (as offsets from frame)
extern int          liqglo_motionSamples;                    // used to assign more than two motion blur samples!
extern float        liqglo_motionFactor;
extern float        liqglo_shutterTime;
extern float        liqglo_shutterEfficiency;
extern bool         liqglo_doShadows;                        // Kept global for liquidRigLightData
extern bool         liqglo_shapeOnlyInShadowNames;
extern MString      liqglo_sceneName;
extern bool         liqglo_beautyRibHasCameraName;           // if true, usual behaviour, otherwise, no camera name in beauty rib
extern bool         liqglo_isShadowPass;                     // true if we are rendering a shadow pass
extern bool         liqglo_expandShaderArrays;
extern bool         liqglo_shortShaderNames;                 // true if we don't want to output path names with shaders
extern bool         liqglo_relativeFileNames;                // true if we only want to output project relative names
// unused anymore as MStringArray.
extern MString      liqglo_DDimageName; 
extern double       liqglo_FPS;                              // Frame-rate (for particle streak length)
extern bool         liqglo_outputMeshUVs;                    // true if we are writing uvs for subdivs/polys (in addition to "st")
extern bool         liqglo_outputMeshAsRMSArrays;            // true => write uvs as arrays
extern bool         liqglo_noSingleFrameShadows;             // allows you to skip single-frame shadows when you chunk a render
extern bool         liqglo_singleFrameShadowsOnly;           // allows you to skip single-frame shadows when you chunk a render
extern MString      liqglo_renderCamera;                     // a global copy for liqRibPfxToonData
extern bool         liqglo_relativeTransforms;               // wether we do Transform or ConcatTransform (relative)
extern bool         liqglo_exportAllShadersParams;           // true => write all shaders parameters even with default values
extern bool				  liqglo_skipDefaultMatte;

// Kept global for liquidGlobalHelper
extern MString      liqglo_projectDir;
extern MString      liqglo_ribDir;
extern MString      liqglo_textureDir;
extern MString      liqglo_shaderPath;               // Shader searchpath
extern MString      liqglo_texturePath;             // Texture searchpath
extern MString      liqglo_archivePath;
extern MString      liqglo_proceduralPath;

// Kept global for liqRibNode.cpp
extern MStringArray liqglo_preReadArchive;
extern MStringArray liqglo_preRibBox;
extern MStringArray liqglo_preReadArchiveShadow;
extern MStringArray liqglo_preRibBoxShadow;
extern MString      liqglo_currentNodeName;
extern MString      liqglo_currentNodeShortName;

extern bool         liqglo_useMtorSubdiv;  // use mtor subdiv attributes
extern bool         liqglo_outputMayaPolyCreases;
extern bool         liqglo_renderAllCurves;
extern HiderType    liqglo_hider;

// Kept global for raytracing
extern bool         rt_useRayTracing;
extern RtFloat      rt_traceBreadthFactor;
extern RtFloat      rt_traceDepthFactor;
extern int          rt_traceMaxDepth;
extern RtFloat      rt_traceSpecularThreshold;
extern int          rt_traceRayContinuation;
extern int          rt_traceCacheMemory;
extern bool         rt_traceDisplacements;
extern bool         rt_traceSampleMotion;
extern RtFloat      rt_traceBias;
extern int          rt_traceMaxDiffuseDepth;
extern int          rt_traceMaxSpecularDepth;
extern RtFloat      rt_irradianceMaxError;
extern RtFloat      rt_irradianceMaxPixelDist;

extern MString      rt_irradianceGlobalHandle;
extern int          rt_irradianceGlobalFileMode;

extern MString      rt_photonGlobalHandle;
extern MString      rt_causticGlobalHandle;
extern int          rt_photonShadingModel;
extern int          rt_photonEstimator;

// Additionnal globals for organized people
extern MString      liqglo_shotName;
extern MString      liqglo_shotVersion;
extern MString      liqglo_layer;
extern bool         liqglo_doExtensionPadding;
extern int          liqglo_outPadding;

// renderer properties
extern liqRenderer liquidRenderer;

/**
 * Checks to see if the liquidGlobals are available.
 */
bool liqRibTranslator::liquidInitGlobals()
{
  MStatus status;
  MSelectionList rGlobalList;
  status = rGlobalList.add( "liquidGlobals" );
  if ( rGlobalList.length() > 0 ) 
  {
    status.clear();
    status = rGlobalList.getDependNode( 0, rGlobalObj );
    if ( status == MS::kSuccess ) return true;
    else return false;
  } 
  return false;
}

/**
 * Read the values from the render globals and set internal values.
 */
void liqRibTranslator::liquidReadGlobals()
{
  MStatus gStatus;
  MPlug gPlug;
  MFnDependencyNode rGlobalNode( rGlobalObj );
  MString varVal;
  int var;
  // find the activeView for previews;
  m_activeView = M3dView::active3dView();
  width        = m_activeView.portWidth();
  height       = m_activeView.portHeight();

  // Display Channels - Read and store 'em !
  // philippe : channels are stored as structures in a vector
  if ( liquidRenderer.supports_DISPLAY_CHANNELS ) 
  {
    m_channels.clear();
    unsigned int nChannels = liquidGetPlugNumElements( rGlobalNode, "channelName", &gStatus );
    
    for ( unsigned i( 0 ); i < nChannels; i++ ) 
    {
      structChannel theChannel;
      
      liquidGetPlugElementValue( rGlobalNode, i, "channelName", theChannel.name, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelType", theChannel.type, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelArraySize", theChannel.arraySize, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelQuantize", theChannel.quantize, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelBitDepth", theChannel.bitDepth, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelDither", theChannel.dither, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelFilter", theChannel.filter, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelPixelFilter", theChannel.pixelFilter, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelPixelFilterX", theChannel.pixelFilterX, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "channelPixelFilterY", theChannel.pixelFilterY, gStatus );
      
      m_channels.push_back( theChannel );
    }
  }
  // Display Driver Globals - Read 'em and store 'em !
  liquidGetPlugValue( rGlobalNode, "ignoreAOVDisplays", m_ignoreAOVDisplays, gStatus );
  {
    m_displays.clear();
    unsigned int nDisplays = liquidGetPlugNumElements( rGlobalNode, "ddImageName", &gStatus );
    //cout <<"  DD : we have "<<nDisplays<<" displays..."<<endl;
    for ( unsigned int i(0); i < nDisplays; i++ ) 
    {
      structDisplay theDisplay;
      
      liquidGetPlugElementValue( rGlobalNode, i, "ddImageName", theDisplay.name, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "ddImageType", theDisplay.type, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "ddImageMode", theDisplay.mode, gStatus );
      
      if ( i==0 )
      {
        theDisplay.enabled = true;
        theDisplay.doQuantize = true;
        theDisplay.doFilter = true;
      }
      else 
      {
        liquidGetPlugElementValue( rGlobalNode, i, "ddEnable", theDisplay.enabled, gStatus );
        liquidGetPlugElementValue( rGlobalNode, i, "ddQuantizeEnabled", theDisplay.doQuantize, gStatus );
        liquidGetPlugElementValue( rGlobalNode, i, "ddFilterEnabled", theDisplay.doFilter, gStatus );
      }
      if ( theDisplay.doQuantize )
      {
        liquidGetPlugElementValue( rGlobalNode, i, "ddBitDepth", theDisplay.bitDepth, gStatus );
        liquidGetPlugElementValue( rGlobalNode, i, "ddDither", theDisplay.dither, gStatus );
      }
      if ( theDisplay.doFilter )
      {
        liquidGetPlugElementValue( rGlobalNode, i, "ddPixelFilter", theDisplay.filter, gStatus );
        liquidGetPlugElementValue( rGlobalNode, i, "ddPixelFilterX", theDisplay.filterX, gStatus );
        liquidGetPlugElementValue( rGlobalNode, i, "ddPixelFilterY", theDisplay.filterY, gStatus );
      }
      // retrieve the extra parameters for this display
      MStringArray xtraParamsNames;
      MStringArray xtraParamsDatas;
      MIntArray    xtraParamsTypes;
      
      liquidGetPlugElementValue( rGlobalNode, i, "ddXtraParamNames", xtraParamsNames, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "ddXtraParamTypes", xtraParamsTypes, gStatus );
      liquidGetPlugElementValue( rGlobalNode, i, "ddXtraParamDatas", xtraParamsDatas, gStatus );

      if ( i == 0 ) 
      { // copy filter params from display 0
        m_rFilter = theDisplay.filter;
        m_rFilterX = theDisplay.filterX;
        m_rFilterY = theDisplay.filterY;
        quantValue = theDisplay.bitDepth;
        liqglo_DDimageName = theDisplay.name; // save primary display name
      }
      structDDParam xtraDDParams;
      xtraDDParams.num   = xtraParamsNames.length();
      xtraDDParams.names = xtraParamsNames;
      xtraDDParams.data  = xtraParamsDatas;
      xtraDDParams.type  = xtraParamsTypes;
      theDisplay.xtraParams = xtraDDParams;
      
      m_displays.push_back( theDisplay );
    }
  }
  liquidGetPlugValue( rGlobalNode, "shotName", liqglo_shotName, gStatus ); // no substitution here
  liquidGetPlugValue( rGlobalNode, "shotVersion", liqglo_shotVersion, gStatus ); // no substitution here
  liquidGetPlugValue( rGlobalNode, "rotateCamera", liqglo_rotateCamera, gStatus );
  liquidGetPlugValue( rGlobalNode, "relativeFileNames", liqglo_relativeFileNames, gStatus );
  liquidGetPlugValue( rGlobalNode, "renderScriptFileName", m_userRenderScriptFileName, gStatus );
  liquidGetPlugValue( rGlobalNode, "beautyRibHasCameraName", liqglo_beautyRibHasCameraName, gStatus );
  
  liquidGetPlugValue( rGlobalNode, "ribgenCommand", m_ribgenCommand, gStatus, true ); // get parsed result
  
  liquidGetPlugValue( rGlobalNode, "preJobCommand", m_preJobCommand, gStatus, true ); // get parsed result
  liquidGetPlugValue( rGlobalNode, "postJobCommand", m_postJobCommand, gStatus, true ); // get parsed result
  liquidGetPlugValue( rGlobalNode, "postFrameCommand", m_postFrameCommand, gStatus );
  liquidGetPlugValue( rGlobalNode, "preFrameCommand", m_preFrameCommand, gStatus );
  liquidGetPlugValue( rGlobalNode, "preCommand", m_preCommand, gStatus );
  liquidGetPlugValue( rGlobalNode, "launchRender", launchRender, gStatus );
  
  liquidGetPlugValue( rGlobalNode, "renderCamera", liqglo_renderCamera, gStatus, true ); // get parsed result
    
  liquidGetPlugValue( rGlobalNode, "ribName", liqglo_sceneName, gStatus, true ); // get parsed result
  liquidGetPlugValue( rGlobalNode, "alfredTags", m_alfredTags, gStatus, true ); // get parsed result
  liquidGetPlugValue( rGlobalNode, "alfredServices", m_alfredServices, gStatus, true ); // get parsed result
  liquidGetPlugValue( rGlobalNode, "dirmaps", m_dirmaps, gStatus, true ); // get parsed result
  liquidGetPlugValue( rGlobalNode, "key",  m_defGenKey, gStatus, true ); // get parsed result
  liquidGetPlugValue( rGlobalNode, "service", m_defGenService, gStatus, true ); // get parsed result

  liquidGetPlugValue( rGlobalNode, "preframeMel", m_preFrameMel, gStatus );
  liquidGetPlugValue( rGlobalNode, "postframeMel", m_postFrameMel, gStatus );
  
  // RENDER OPTIONS:BEGIN
  liquidGetPlugValue( rGlobalNode, "hider", var, gStatus );
  if ( gStatus == MS::kSuccess ) liqglo_hider = (enum HiderType) var;
  liquidGetPlugValue( rGlobalNode, "jitter", m_hiddenJitter, gStatus );
  // PRMAN 13 BEGIN
  liquidGetPlugValue( rGlobalNode, "hiddenApertureNSides", m_hiddenAperture[0], gStatus );
  liquidGetPlugValue( rGlobalNode, "hiddenApertureAngle", m_hiddenAperture[1], gStatus );
  liquidGetPlugValue( rGlobalNode, "hiddenApertureRoundness", m_hiddenAperture[2], gStatus );
  liquidGetPlugValue( rGlobalNode, "hiddenApertureDensity", m_hiddenAperture[3], gStatus );   
  liquidGetPlugValue( rGlobalNode, "hiddenShutterOpeningOpen", m_hiddenShutterOpening[0], gStatus );     
  liquidGetPlugValue( rGlobalNode, "hiddenShutterOpeningClose", m_hiddenShutterOpening[1], gStatus );   
  // PRMAN 13 END
  liquidGetPlugValue( rGlobalNode, "hiddenOcclusionBound", m_hiddenOcclusionBound, gStatus );  
  liquidGetPlugValue( rGlobalNode, "hiddenMpCache", m_hiddenMpCache, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "hiddenMpMemory", m_hiddenMpMemory, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "hiddenMpCacheDir", m_hiddenMpCacheDir, gStatus );   
  liquidGetPlugValue( rGlobalNode, "hiddenSampleMotion", m_hiddenSampleMotion, gStatus );     
  liquidGetPlugValue( rGlobalNode, "hiddenSubPixel", m_hiddenSubPixel, gStatus );  
  liquidGetPlugValue( rGlobalNode, "hiddenExtremeMotionDof", m_hiddenExtremeMotionDof, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "hiddenMaxVPDepth", m_hiddenMaxVPDepth, gStatus ); 
  // PRMAN 13 BEGIN  
  liquidGetPlugValue( rGlobalNode, "hiddenSigmaHiding", m_hiddenSigma, gStatus );   
  liquidGetPlugValue( rGlobalNode, "hiddenSigmaBlur", m_hiddenSigmaBlur, gStatus );   
  // PRMAN 13 END  
  liquidGetPlugValue( rGlobalNode, "raytraceFalseColor", m_raytraceFalseColor, gStatus );     
  liquidGetPlugValue( rGlobalNode, "photonEmit", m_photonEmit, gStatus );   
  liquidGetPlugValue( rGlobalNode, "photonSampleSpectrum", m_photonSampleSpectrum, gStatus );  
  liquidGetPlugValue( rGlobalNode, "depthMaskZFile", varVal, gStatus );
  if ( gStatus == MS::kSuccess ) 
    m_depthMaskZFile = parseString( varVal, false );  //  doEscaped = false
  liquidGetPlugValue( rGlobalNode, "depthMaskReverseSign", m_depthMaskReverseSign, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "depthMaskDepthBias", m_depthMaskDepthBias, gStatus ); 
  // RENDER OPTIONS:END

  liquidGetPlugValue( rGlobalNode, "cropX1", m_cropX1, gStatus );
  liquidGetPlugValue( rGlobalNode, "cropX2", m_cropX2, gStatus );
  liquidGetPlugValue( rGlobalNode, "cropY1", m_cropY1, gStatus );
  liquidGetPlugValue( rGlobalNode, "cropY2", m_cropY2, gStatus );

  // RAYTRACING OPTIONS:BEGIN
  liquidGetPlugValue( rGlobalNode, "useRayTracing", rt_useRayTracing, gStatus );
  liquidGetPlugValue( rGlobalNode, "traceMaxDepth", rt_traceMaxDepth, gStatus );
  liquidGetPlugValue( rGlobalNode, "traceSpecularThreshold", rt_traceSpecularThreshold, gStatus );
  liquidGetPlugValue( rGlobalNode, "traceBreadthFactor", rt_traceBreadthFactor, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceDepthFactor", rt_traceDepthFactor, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceRayContinuation", rt_traceRayContinuation, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceCacheMemory", rt_traceCacheMemory, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceDisplacements", rt_traceDisplacements, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceSampleMotion", rt_traceSampleMotion, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceBias", rt_traceBias, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceMaxDiffuseDepth", rt_traceMaxDiffuseDepth, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "traceMaxSpecularDepth", rt_traceMaxSpecularDepth, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "irradianceMaxError", rt_irradianceMaxError, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "irradianceMaxPixelDist", rt_irradianceMaxPixelDist, gStatus ); 
  
  liquidGetPlugValue( rGlobalNode, "irradianceHandle", rt_irradianceGlobalHandle, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "irradianceFileMode", rt_irradianceGlobalFileMode, gStatus );

  liquidGetPlugValue( rGlobalNode, "photonGlobalHandle", rt_photonGlobalHandle, gStatus );
  // if ( gStatus == MS::kSuccess ) 
  // rt_photonGlobalHandle = parseString( rt_photonGlobalHandle, false );  //  doEscaped = false
  liquidGetPlugValue( rGlobalNode, "causticGlobalHandle", rt_causticGlobalHandle, gStatus );
  // if ( gStatus == MS::kSuccess ) 
  // rt_causticGlobalHandle = parseString( rt_causticGlobalHandle, false );  //  doEscaped = false

  liquidGetPlugValue( rGlobalNode, "photonShadingModel", rt_photonShadingModel, gStatus );
  liquidGetPlugValue( rGlobalNode, "photonEstimator", rt_photonEstimator, gStatus );

  // RAYTRACING OPTIONS:END
  liquidGetPlugValue( rGlobalNode, "outputMayaPolyCreases", liqglo_outputMayaPolyCreases, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "useMtorSubdiv", liqglo_useMtorSubdiv, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "ribRelativeTransforms", liqglo_relativeTransforms, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "shortShaderNames", liqglo_shortShaderNames, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "expandAlfred", m_alfredExpand, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "createOutputDirectories", createOutputDirectories, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "minCPU", m_minCPU, gStatus ); 
  liquidGetPlugValue( rGlobalNode, "maxCPU", m_maxCPU, gStatus );
  liquidGetPlugValue( rGlobalNode, "showProgress", m_showProgress, gStatus );
  liquidGetPlugValue( rGlobalNode, "expandShaderArrays", liqglo_expandShaderArrays, gStatus );
  liquidGetPlugValue( rGlobalNode, "exportAllShadersParameters", liqglo_exportAllShadersParams, gStatus );
  liquidGetPlugValue( rGlobalNode, "outputComments", m_outputComments, gStatus );
  liquidGetPlugValue( rGlobalNode, "shaderDebug", m_shaderDebug, gStatus );
  liquidGetPlugValue( rGlobalNode, "deferredGen", m_deferredGen, gStatus );
  liquidGetPlugValue( rGlobalNode, "deferredBlock", m_deferredBlockSize, gStatus );
  liquidGetPlugValue( rGlobalNode, "useRenderScript", useRenderScript, gStatus );
  liquidGetPlugValue( rGlobalNode, "remoteRender", remoteRender, gStatus );
  liquidGetPlugValue( rGlobalNode, "renderAllCurves", m_renderAllCurves, gStatus );
  liquidGetPlugValue( rGlobalNode, "illuminateByDefault", m_illuminateByDefault, gStatus );
  liquidGetPlugValue( rGlobalNode, "liquidSetLightLinking", m_liquidSetLightLinking, gStatus );
  liquidGetPlugValue( rGlobalNode, "ignoreLights", m_ignoreLights, gStatus );
  liquidGetPlugValue( rGlobalNode, "ignoreSurfaces", m_ignoreSurfaces, gStatus );
  liquidGetPlugValue( rGlobalNode, "ignoreDisplacements", m_ignoreDisplacements, gStatus );
  liquidGetPlugValue( rGlobalNode, "ignoreVolumes", m_ignoreVolumes, gStatus );
  
  liquidGetPlugValue( rGlobalNode, "outputShadowPass", m_outputShadowPass, gStatus );
  liquidGetPlugValue( rGlobalNode, "outputHeroPass", m_outputHeroPass, gStatus );

  liquidGetPlugValue( rGlobalNode, "netRManRender", useNetRman, gStatus );
  liquidGetPlugValue( rGlobalNode, "ignoreShadows", liqglo_doShadows, gStatus );
  liqglo_doShadows = !liqglo_doShadows;
  
  liquidGetPlugValue( rGlobalNode, "shapeOnlyInShadowNames", liqglo_shapeOnlyInShadowNames, gStatus );
  liquidGetPlugValue( rGlobalNode, "fullShadowRibs", fullShadowRib, gStatus );
  liquidGetPlugValue( rGlobalNode, "binaryOutput", liqglo_doBinary, gStatus );
  liquidGetPlugValue( rGlobalNode, "lazyCompute", m_lazyCompute, gStatus );
  liquidGetPlugValue( rGlobalNode, "outputShadersInShadows", m_outputShadersInShadows, gStatus );
  // Moritz: added new options for light/shader output in deep shadows
  liquidGetPlugValue( rGlobalNode, "outputShadersInDeepShadows", m_outputShadersInDeepShadows, gStatus );
  liquidGetPlugValue( rGlobalNode, "outputLightsInDeepShadows", m_outputLightsInDeepShadows, gStatus );
 
  liquidGetPlugValue( rGlobalNode, "outputMeshUVs", liqglo_outputMeshUVs, gStatus );
  liquidGetPlugValue( rGlobalNode, "compressedOutput", liqglo_doCompression, gStatus );
  liquidGetPlugValue( rGlobalNode, "exportReadArchive", m_exportReadArchive, gStatus );
  liquidGetPlugValue( rGlobalNode, "renderJobName", renderJobName, gStatus );
  liquidGetPlugValue( rGlobalNode, "doAnimation", m_animation, gStatus );
  if ( m_animation ) 
  {
    MString frameSequence;
    liquidGetPlugValue( rGlobalNode, "frameSequence", frameSequence, gStatus );
    if( gStatus == MS::kSuccess ) frameNumbers = generateFrameNumbers( string( frameSequence.asChar() ) );
  }
  liquidGetPlugValue( rGlobalNode, "doPadding", liqglo_doExtensionPadding, gStatus );
  if ( liqglo_doExtensionPadding ) liquidGetPlugValue( rGlobalNode, "padding", liqglo_outPadding, gStatus );
  {  
    int gWidth, gHeight;
    liquidGetPlugValue( rGlobalNode, "xResolution", gWidth, gStatus );
    liquidGetPlugValue( rGlobalNode, "yResolution", gHeight, gStatus );
    if ( gWidth > 0 ) width = gWidth;
    if ( gHeight > 0 ) height = gHeight;
  }
  liquidGetPlugValue( rGlobalNode, "pixelAspectRatio", aspectRatio, gStatus );
  
  liquidGetPlugValue( rGlobalNode, "transformationBlur", liqglo_doMotion, gStatus );
  liquidGetPlugValue( rGlobalNode, "cameraBlur", doCameraMotion, gStatus );
  liquidGetPlugValue( rGlobalNode, "deformationBlur", liqglo_doDef, gStatus );
  liquidGetPlugValue( rGlobalNode, "shutterConfig", var, gStatus );
  if ( gStatus == MS::kSuccess ) shutterConfig = ( enum shutterConfig ) var;
  liquidGetPlugValue( rGlobalNode, "shutterEfficiency", liqglo_shutterEfficiency, gStatus );
  liquidGetPlugValue( rGlobalNode, "motionBlurSamples", liqglo_motionSamples, gStatus );
  if ( liqglo_motionSamples > LIQMAXMOTIONSAMPLES ) liqglo_motionSamples = LIQMAXMOTIONSAMPLES;
  liquidGetPlugValue( rGlobalNode, "motionFactor", liqglo_motionFactor, gStatus );
  liquidGetPlugValue( rGlobalNode, "relativeMotion", liqglo_relativeMotion, gStatus );
  liquidGetPlugValue( rGlobalNode, "depthOfField", doDof, gStatus );
  liquidGetPlugValue( rGlobalNode, "pixelSamples", pixelSamples, gStatus );
  liquidGetPlugValue( rGlobalNode, "shadingRate", shadingRate, gStatus );
  liquidGetPlugValue( rGlobalNode, "limitsBucketXSize", bucketSize[0], gStatus );
  liquidGetPlugValue( rGlobalNode, "limitsBucketYSize", bucketSize[1], gStatus );
  liquidGetPlugValue( rGlobalNode, "limitsGridSize", gridSize, gStatus );
  liquidGetPlugValue( rGlobalNode, "limitsTextureMemory", textureMemory, gStatus );
  liquidGetPlugValue( rGlobalNode, "limitsEyeSplits", eyeSplits, gStatus );
  
  liquidGetPlugValue( rGlobalNode, "limitsOThreshold", othreshold, gStatus );
  liquidGetPlugValue( rGlobalNode, "limitsZThreshold", zthreshold, gStatus );
  
  liquidGetPlugValue( rGlobalNode, "cleanRib", cleanRib, gStatus );
  liquidGetPlugValue( rGlobalNode, "cleanRenderScript", cleanRenderScript, gStatus );
  liquidGetPlugValue( rGlobalNode, "cleanTex", cleanTextures, gStatus );
  liquidGetPlugValue( rGlobalNode, "cleanShad", cleanShadows, gStatus );
  liquidGetPlugValue( rGlobalNode, "justRib", m_justRib, gStatus );
  liquidGetPlugValue( rGlobalNode, "gain", m_rgain, gStatus );
  liquidGetPlugValue( rGlobalNode, "gamma", m_rgamma, gStatus );
  liquidGetPlugValue( rGlobalNode, "renderViewLocal", m_renderViewLocal, gStatus );
  liquidGetPlugValue( rGlobalNode, "renderViewPort", m_renderViewPort, gStatus );
  liquidGetPlugValue( rGlobalNode, "renderViewTimeOut", m_renderViewTimeOut, gStatus );
  liquidGetPlugValue( rGlobalNode, "statistics", m_statistics, gStatus );
  liquidGetPlugValue( rGlobalNode, "statisticsFile", varVal, gStatus );
  if ( varVal != "" ) 
    m_statisticsFile = parseString( varVal, false );

  // Philippe : OBSOLETE ?
  liquidGetPlugValue( rGlobalNode, "imageDriver", varVal, gStatus );
  if ( gStatus == MS::kSuccess ) outFormat = parseString( varVal, false );
  liquidGetPlugValue( rGlobalNode, "bakeRasterOrient", m_bakeNonRasterOrient, gStatus );
  if ( gStatus == MS::kSuccess ) m_bakeNonRasterOrient = !m_bakeNonRasterOrient;
  liquidGetPlugValue( rGlobalNode, "bakeCullBackface", m_bakeNoCullBackface, gStatus );
  if ( gStatus == MS::kSuccess ) m_bakeNoCullBackface = !m_bakeNoCullBackface;
  liquidGetPlugValue( rGlobalNode, "bakeCullHidden", m_bakeNoCullHidden, gStatus );
  if ( gStatus == MS::kSuccess ) m_bakeNoCullHidden = !m_bakeNoCullHidden;
  
	// taking into account liquidRibRequest nodes and preposterous mel scripts - Alf
	{
		MStringArray requestArray;
		MString request;
    
    liquidGetPlugValue( rGlobalNode, "preFrameBegin", request, gStatus );
		if ( request != "" ) requestArray.append( request );
		// add rib request node values
		request = parseLiquidRibRequest( requestArray, "preFrame" );
		if ( request != "" ) m_preFrameRIB = parseString( request );
	
    requestArray.clear();
    request.clear();
    liquidGetPlugValue( rGlobalNode, "preWorld", request, gStatus );
		if ( request != "" ) requestArray.append( request );
		request = parseLiquidRibRequest( requestArray, "preWorld" );
		if ( request != "" ) m_preWorldRIB = parseString( request );

    requestArray.clear();
    request.clear();
    liquidGetPlugValue( rGlobalNode, "postWorld", request, gStatus );
		if ( request != "" ) requestArray.append( request );
		request = parseLiquidRibRequest( requestArray, "postWorld" );
		if ( request != "" ) m_postWorldRIB = parseString( request );
		
    requestArray.clear();
    request.clear();
    liquidGetPlugValue( rGlobalNode, "preGeom", request, gStatus );
		if ( request != "" ) requestArray.append( request );
		request = parseLiquidRibRequest( requestArray, "preGeom" );
		if ( request != "" ) m_preGeomRIB = parseString( request );
	}
	
  liquidGetPlugValue( rGlobalNode, "renderScriptFormat", var, gStatus );
  if ( gStatus == MS::kSuccess ) m_renderScriptFormat = ( enum renderScriptFormat ) var;
  
  liquidGetPlugValue( rGlobalNode, "renderScriptCommand", varVal, gStatus );
  m_renderScriptCommand = ( varVal != "" )? parseString( varVal, false ) : "alfred";
    
  setOutDirs();
  setSearchPaths();
}