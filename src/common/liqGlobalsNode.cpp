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

/* ______________________________________________________________________
**
** Liquid Globals Node Source
** ______________________________________________________________________
*/

#include <liquid.h>
#include <liqGlobalsNode.h>
#include <liqMayaNodeIds.h>

#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSwatchRenderBase.h>
#include <maya/MSwatchRenderRegister.h>
#include <maya/MImage.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnStringData.h>

#include <liqIOStream.h>

// static data
MTypeId liqGlobalsNode::id( liqGlobalsNodeId );

// Attributes
MObject liqGlobalsNode::aLaunchRender;

MObject liqGlobalsNode::aRenderCamera;
MObject liqGlobalsNode::aRotateCamera;

MObject liqGlobalsNode::aIgnoreAOVDisplays;
MObject liqGlobalsNode::aDdImageName;
MObject liqGlobalsNode::aDdImageType;
MObject liqGlobalsNode::aDdImageMode;
MObject liqGlobalsNode::aDdParamType;
MObject liqGlobalsNode::aDdEnable;
MObject liqGlobalsNode::aDdQuantizeEnabled;
MObject liqGlobalsNode::aDdBitDepth;
MObject liqGlobalsNode::aDdDither;
MObject liqGlobalsNode::aDdFilterEnabled;
MObject liqGlobalsNode::aDdPixelFilter;
MObject liqGlobalsNode::aDdPixelFilterX;
MObject liqGlobalsNode::aDdPixelFilterY;
MObject liqGlobalsNode::aDdXtraParamNames;
MObject liqGlobalsNode::aDdXtraParamTypes;
MObject liqGlobalsNode::aDdXtraParamDatas;
MObject liqGlobalsNode::aNumDD;
MObject liqGlobalsNode::aNumDDParam;

MObject liqGlobalsNode::aChannelName;
MObject liqGlobalsNode::aChannelType;
MObject liqGlobalsNode::aChannelArraySize;
MObject liqGlobalsNode::aChannelQuantize;
MObject liqGlobalsNode::aChannelBitDepth;
MObject liqGlobalsNode::aChannelDither;
MObject liqGlobalsNode::aChannelFilter;
MObject liqGlobalsNode::aChannelPixelFilter;
MObject liqGlobalsNode::aChannelPixelFilterX;
MObject liqGlobalsNode::aChannelPixelFilterY;

MObject liqGlobalsNode::aCreateOutputDirectories;
MObject liqGlobalsNode::aExpandShaderArrays;

MObject liqGlobalsNode::aBakeRasterOrient;
MObject liqGlobalsNode::aBakeCullBackface;
MObject liqGlobalsNode::aBakeCullHidden;

MObject liqGlobalsNode::aShaderPath;
MObject liqGlobalsNode::aTexturePath;
MObject liqGlobalsNode::aArchivePath;
MObject liqGlobalsNode::aProceduralPath;

MObject liqGlobalsNode::aRibName;
MObject liqGlobalsNode::aBeautyRibHasCameraName;

MObject liqGlobalsNode::aPictureDirectory;
MObject liqGlobalsNode::aTextureDirectory;
MObject liqGlobalsNode::aRibDirectory;
MObject liqGlobalsNode::aShaderDirectory;
MObject liqGlobalsNode::aTempDirectory;

MObject liqGlobalsNode::aDeferredGen;
MObject liqGlobalsNode::aDeferredBlock;
MObject liqGlobalsNode::aPreframeMel;
MObject liqGlobalsNode::aPostframeMel;
MObject liqGlobalsNode::aUseRenderScript;
MObject liqGlobalsNode::aRemoteRender;
MObject liqGlobalsNode::aNetRManRender;
MObject liqGlobalsNode::aMinCPU;
MObject liqGlobalsNode::aMaxCPU;
MObject liqGlobalsNode::aIgnoreShadows;
MObject liqGlobalsNode::aShapeOnlyInShadowNames;
MObject liqGlobalsNode::aFullShadowRibs;
MObject liqGlobalsNode::aBinaryOutput;
MObject liqGlobalsNode::aCompressedOutput;
MObject liqGlobalsNode::aOutputMayaPolyCreases;
MObject liqGlobalsNode::aRenderAllCurves;
MObject liqGlobalsNode::aOutputMeshUVs;
MObject liqGlobalsNode::aOutputMeshAsRMSArrays;
MObject liqGlobalsNode::aIlluminateByDefault;
MObject liqGlobalsNode::aLiquidSetLightLinking;
MObject liqGlobalsNode::aIgnoreSurfaces;
MObject liqGlobalsNode::aIgnoreDisplacements;
MObject liqGlobalsNode::aIgnoreLights;
MObject liqGlobalsNode::aIgnoreVolumes;
MObject liqGlobalsNode::aOutputShadersInShadows;
MObject liqGlobalsNode::aOutputShadersInDeepShadows;
MObject liqGlobalsNode::aOutputLightsInDeepShadows;
MObject liqGlobalsNode::aExportAllShadersParameters;

MObject liqGlobalsNode::aOutputShadowPass;
MObject liqGlobalsNode::aOutputHeroPass;
MObject liqGlobalsNode::aOutputComments;
MObject liqGlobalsNode::aShaderDebug;
MObject liqGlobalsNode::aShowProgress;
MObject liqGlobalsNode::aDoAnimation;
MObject liqGlobalsNode::aFrameSequence;
MObject liqGlobalsNode::aDoPadding;
MObject liqGlobalsNode::aPadding;
MObject liqGlobalsNode::aNumProcs;
MObject liqGlobalsNode::aGain;
MObject liqGlobalsNode::aGamma;
MObject liqGlobalsNode::aXResolution;
MObject liqGlobalsNode::aYResolution;
MObject liqGlobalsNode::aPixelAspectRatio;
MObject liqGlobalsNode::aImageDriver;

MObject liqGlobalsNode::aCameraBlur;
MObject liqGlobalsNode::aTransformationBlur;
MObject liqGlobalsNode::aDeformationBlur;
MObject liqGlobalsNode::aShutterConfig;
MObject liqGlobalsNode::aShutterEfficiency;
MObject liqGlobalsNode::aMotionBlurSamples;
MObject liqGlobalsNode::aRelativeMotion;
MObject liqGlobalsNode::aMotionFactor;
MObject liqGlobalsNode::aDepthOfField;

MObject liqGlobalsNode::aPixelSamples;
MObject liqGlobalsNode::aShadingRate;

MObject liqGlobalsNode::aLimitsOThreshold;
MObject liqGlobalsNode::aLimitsZThreshold;
MObject liqGlobalsNode::aLimitsBucketXSize;
MObject liqGlobalsNode::aLimitsBucketYSize;
MObject liqGlobalsNode::aLimitsGridSize;
MObject liqGlobalsNode::aLimitsTextureMemory;
MObject liqGlobalsNode::aLimitsEyeSplits;
MObject liqGlobalsNode::aLimitsGPrimSplits;

MObject liqGlobalsNode::aRibRelativeTransforms;

MObject liqGlobalsNode::aCleanRib;
MObject liqGlobalsNode::aCleanTex;
MObject liqGlobalsNode::aCleanShad;
MObject liqGlobalsNode::aCleanRenderScript;
MObject liqGlobalsNode::aJustRib;
MObject liqGlobalsNode::aAlfredTags;
MObject liqGlobalsNode::aAlfredServices;
MObject liqGlobalsNode::aDirMaps;
MObject liqGlobalsNode::aRenderCommand;
MObject liqGlobalsNode::aRibgenCommand;

MObject liqGlobalsNode::aPreviewer;
MObject liqGlobalsNode::aPreCommand;
MObject liqGlobalsNode::aPostFrameCommand;
MObject liqGlobalsNode::aPreFrameCommand;
MObject liqGlobalsNode::aPreJobCommand;
MObject liqGlobalsNode::aPostJobCommand;
MObject liqGlobalsNode::aKey;
MObject liqGlobalsNode::aService;
MObject liqGlobalsNode::aLastRenderScript;
MObject liqGlobalsNode::aLastRibFile;
MObject liqGlobalsNode::aSimpleGlobalsWindow;
MObject liqGlobalsNode::aLazyCompute;
MObject liqGlobalsNode::aCropX1;
MObject liqGlobalsNode::aCropX2;
MObject liqGlobalsNode::aCropY1;
MObject liqGlobalsNode::aCropY2;
MObject liqGlobalsNode::aExportReadArchive;
MObject liqGlobalsNode::aRenderJobName;
MObject liqGlobalsNode::aShortShaderNames;

MObject liqGlobalsNode::aRelativeFileNames;

MObject liqGlobalsNode::aExpandAlfred;

MObject liqGlobalsNode::aPreFrameBeginMel;
MObject liqGlobalsNode::aPreWorldMel;
MObject liqGlobalsNode::aPostWorldMel;
MObject liqGlobalsNode::aPreGeomMel;

MObject liqGlobalsNode::aPreFrameBegin;
MObject liqGlobalsNode::aPreWorld;
MObject liqGlobalsNode::aPostWorld;
MObject liqGlobalsNode::aPreGeom;

MObject liqGlobalsNode::aRenderScriptFormat;
MObject liqGlobalsNode::aRenderScriptCommand;
MObject liqGlobalsNode::aRenderScriptFileName;

MObject liqGlobalsNode::aFluidShaderBrowserDefaultPath;
MObject liqGlobalsNode::aPreviewType;
MObject liqGlobalsNode::aPreviewRenderer;
MObject liqGlobalsNode::aPreviewSize;
MObject liqGlobalsNode::aPreviewPrimitive;
MObject liqGlobalsNode::aPreviewDisplayDriver;
MObject liqGlobalsNode::aPreviewConnectionType;
MObject liqGlobalsNode::aRenderViewLocal;
MObject liqGlobalsNode::aRenderViewPort;
MObject liqGlobalsNode::aRenderViewTimeOut;

MObject liqGlobalsNode::aUseRayTracing;
MObject liqGlobalsNode::aTraceBreadthFactor;
MObject liqGlobalsNode::aTraceDepthFactor;
MObject liqGlobalsNode::aTraceMaxDepth;
MObject liqGlobalsNode::aTraceSpecularThreshold;
MObject liqGlobalsNode::aTraceRayContinuation;
MObject liqGlobalsNode::aTraceCacheMemory;
MObject liqGlobalsNode::aTraceDisplacements;
MObject liqGlobalsNode::aTraceBias;
MObject liqGlobalsNode::aTraceSampleMotion;
MObject liqGlobalsNode::aTraceMaxSpecularDepth;
MObject liqGlobalsNode::aTraceMaxDiffuseDepth;

MObject liqGlobalsNode::aIrradianceMaxError;
MObject liqGlobalsNode::aIrradianceMaxPixelDist;
MObject liqGlobalsNode::aIrradianceHandle;
MObject liqGlobalsNode::aIrradianceFileMode;

MObject liqGlobalsNode::aPhotonGlobalHandle;
MObject liqGlobalsNode::aCausticGlobalHandle;
MObject liqGlobalsNode::aPhotonShadingModel;
MObject liqGlobalsNode::aPhotonEstimator;

MObject liqGlobalsNode::aUseMtorSubdiv;
MObject liqGlobalsNode::aHider;
MObject liqGlobalsNode::aJitter;
// PRMAN 13 BEGIN
MObject liqGlobalsNode::aHiddenApertureNSides;
MObject liqGlobalsNode::aHiddenApertureAngle;
MObject liqGlobalsNode::aHiddenApertureRoundness;
MObject liqGlobalsNode::aHiddenApertureDensity;
MObject liqGlobalsNode::aHiddenShutterOpeningOpen;
MObject liqGlobalsNode::aHiddenShutterOpeningClose;
// PRMAN 13 END
MObject liqGlobalsNode::aHiddenOcclusionBound;
MObject liqGlobalsNode::aHiddenMpCache;
MObject liqGlobalsNode::aHiddenMpMemory;
MObject liqGlobalsNode::aHiddenMpCacheDir;
MObject liqGlobalsNode::aHiddenSampleMotion;
MObject liqGlobalsNode::aHiddenSubPixel;
MObject liqGlobalsNode::aHiddenExtremeMotionDof;
MObject liqGlobalsNode::aHiddenMaxVPDepth;
// PRMAN 13 BEGIN
MObject liqGlobalsNode::aHiddenSigma;
MObject liqGlobalsNode::aHiddenSigmaBlur;
// PRMAN 13 END

MObject liqGlobalsNode::aRaytraceFalseColor;

MObject liqGlobalsNode::aPhotonEmit;
MObject liqGlobalsNode::aPhotonSampleSpectrum;

MObject liqGlobalsNode::aDepthMaskZFile;
MObject liqGlobalsNode::aDepthMaskReverseSign;
MObject liqGlobalsNode::aDepthMaskDepthBias;

MObject liqGlobalsNode::aRenderCmdFlags;
MObject liqGlobalsNode::aShaderInfo;
MObject liqGlobalsNode::aShaderComp;
MObject liqGlobalsNode::aShaderExt;
MObject liqGlobalsNode::aMakeTexture;
MObject liqGlobalsNode::aViewTexture;
MObject liqGlobalsNode::aTextureExt;

MObject liqGlobalsNode::aBits_hiders;
MObject liqGlobalsNode::aBits_hiders_0;
MObject liqGlobalsNode::aBits_hiders_1;
MObject liqGlobalsNode::aBits_hiders_2;
MObject liqGlobalsNode::aBits_hiders_3;
MObject liqGlobalsNode::aBits_hiders_4;
MObject liqGlobalsNode::aBits_hiders_5;

MObject liqGlobalsNode::aBits_filters;
MObject liqGlobalsNode::aBits_filters_0;
MObject liqGlobalsNode::aBits_filters_1;
MObject liqGlobalsNode::aBits_filters_2;
MObject liqGlobalsNode::aBits_filters_3;
MObject liqGlobalsNode::aBits_filters_4;
MObject liqGlobalsNode::aBits_filters_5;
MObject liqGlobalsNode::aBits_filters_6;
MObject liqGlobalsNode::aBits_filters_7;
MObject liqGlobalsNode::aBits_filters_8;
MObject liqGlobalsNode::aBits_filters_9;
MObject liqGlobalsNode::aBits_filters_10;

MObject liqGlobalsNode::aBits_features;
MObject liqGlobalsNode::aBits_features_0;
MObject liqGlobalsNode::aBits_features_1;
MObject liqGlobalsNode::aBits_features_2;
MObject liqGlobalsNode::aBits_features_3;
MObject liqGlobalsNode::aBits_features_4;
MObject liqGlobalsNode::aBits_features_5;
MObject liqGlobalsNode::aBits_features_6;

MObject liqGlobalsNode::aBits_required;
MObject liqGlobalsNode::aBits_required_0;
MObject liqGlobalsNode::aBits_required_1;
MObject liqGlobalsNode::aBits_required_2;

MObject liqGlobalsNode::aDshDisplayName;
MObject liqGlobalsNode::aDshImageMode;

MObject liqGlobalsNode::aShotName;
MObject liqGlobalsNode::aShotVersion;

MObject liqGlobalsNode::aStatistics;
MObject liqGlobalsNode::aStatisticsFile;

MObject liqGlobalsNode::aShadersIgnoreOutputParams;
MObject liqGlobalsNode::aShadersOutputParamsFilter;
MObject liqGlobalsNode::aShadersMaxCachedAELayouts;

MObject liqGlobalsNode::aVerbosity;

#define LIQ_GLOBALS_HIDE_ATTRIBUTES false

#define CREATE_BOOL(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kBoolean, default, &status); \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_MULTI_BOOL(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kBoolean, default, &status); \
    CHECK_MSTATUS(attr.setArray(true));       \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_INT(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kInt, default, &status); \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_MULTI_INT(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kInt, default, &status); \
    CHECK_MSTATUS(attr.setArray(true));       \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_LONG(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kLong, default, &status); \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_MULTI_LONG(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kLong, default, &status); \
    CHECK_MSTATUS(attr.setArray(true));       \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_FLOAT(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kFloat, default, &status); \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_MULTI_FLOAT(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnNumericData::kFloat, default, &status); \
    CHECK_MSTATUS(attr.setArray(true));       \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_STRING(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnData::kString, obj, &status); \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));     \
    CHECK_MSTATUS(attr.setDefault( stringData.create( MString( default ), &sstat ) ) ); \
    CHECK_MSTATUS( sstat );         \
    CHECK_MSTATUS(addAttribute(obj));         

#define CREATE_MULTI_STRING(attr, obj, name, shortName, default)    \
    obj = attr.create( name, shortName, MFnData::kString, obj, &status); \
    CHECK_MSTATUS(attr.setArray(true));       \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_COMP(attr, obj, name, shortName)    \
    obj = attr.create( name, shortName, &status); \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_MULTI_STR_ARRAY(attr, obj, name, shortName)    \
    obj = attr.create( name, shortName, MFnData::kStringArray, obj, &status); \
    CHECK_MSTATUS(attr.setArray(true));       \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_MULTI_INT_ARRAY(attr, obj, name, shortName)    \
    obj = attr.create( name, shortName, MFnData::kIntArray, obj, &status); \
    CHECK_MSTATUS(attr.setArray(true));       \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_COLOR( attr, obj, name, shortName, default1, default2, default3 )    \
	obj = attr.createColor( name, shortName, &status ); \
	CHECK_MSTATUS(attr.setDefault( default1, default2, default3 )); \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(LIQ_GLOBALS_HIDE_ATTRIBUTES));      \
    CHECK_MSTATUS(addAttribute(obj));

liqGlobalsNode::liqGlobalsNode()
{
}

liqGlobalsNode::~liqGlobalsNode()
{
}

void* liqGlobalsNode::creator()
{
    return new liqGlobalsNode();
}

MStatus liqGlobalsNode::initialize()
{
	MFnTypedAttribute     tAttr;
	MFnNumericAttribute   nAttr;
	MFnCompoundAttribute  cAttr;
	MStatus status, sstat;
	
	MFnStringData stringData;


	// Create input attributes
	CREATE_BOOL( nAttr,  aLaunchRender,             "launchRender",                 "lr",     true  );
	CREATE_STRING( tAttr,  aRenderCamera,             "renderCamera",                 "rc",     ""    );
	CREATE_BOOL( nAttr,  aRotateCamera,             "rotateCamera",                 "roc",    false );
	CREATE_BOOL( nAttr,  aIgnoreAOVDisplays,        "ignoreAOVDisplays",            "iaov",   false );

	CREATE_MULTI_STRING( tAttr,  aDdImageName,              "ddImageName",                  "ddin",   ""    );
	CREATE_MULTI_STRING( tAttr,  aDdImageMode,              "ddImageMode",                  "ddim",   ""    );
	CREATE_MULTI_STRING( tAttr,  aDdImageType,              "ddImageType",                  "ddit",   ""    );
	CREATE_MULTI_STRING( tAttr,  aDdParamType,              "ddParamType",                  "ddpt",   ""    );
	CREATE_MULTI_BOOL( nAttr,  aDdEnable,                 "ddEnable",                     "dde",    true  );
	CREATE_MULTI_BOOL( nAttr,  aDdQuantizeEnabled,        "ddQuantizeEnabled",            "ddqe",   false );
	CREATE_MULTI_INT( nAttr,  aDdBitDepth,               "ddBitDepth",                   "ddbd",   0     );
	CREATE_MULTI_FLOAT( nAttr,  aDdDither,                 "ddDither",                     "ddd",    0     );
	CREATE_MULTI_BOOL( nAttr,  aDdFilterEnabled,          "ddFilterEnabled",              "ddfe",   false );
	CREATE_MULTI_INT( nAttr,  aDdPixelFilter,            "ddPixelFilter",                "ddpf",   0     );
	CREATE_MULTI_FLOAT( nAttr,  aDdPixelFilterX,           "ddPixelFilterX",               "ddpfx",  2.0   );
	CREATE_MULTI_FLOAT( nAttr,  aDdPixelFilterY,           "ddPixelFilterY",               "ddpfy",  2.0   );
	CREATE_MULTI_STR_ARRAY( tAttr,  aDdXtraParamNames,         "ddXtraParamNames",             "ddxpn"         );
	CREATE_MULTI_INT_ARRAY( tAttr,  aDdXtraParamTypes,         "ddXtraParamTypes",             "ddxpt"         );
	CREATE_MULTI_STR_ARRAY( tAttr,  aDdXtraParamDatas,         "ddXtraParamDatas",             "ddxpd"         );

	CREATE_MULTI_STRING( tAttr,  aChannelName,                "channelName",                  "dcn",    ""    );
	CREATE_MULTI_INT( nAttr,  aChannelType,                "channelType",                  "dct",    0     );
	CREATE_MULTI_INT( nAttr,  aChannelArraySize,           "channelArraySize",             "dcs",    0     );
	CREATE_MULTI_BOOL( nAttr,  aChannelQuantize,            "channelQuantize",              "dcq",    false );
	CREATE_MULTI_INT( nAttr,  aChannelBitDepth,            "channelBitDepth",              "dcbd",   8     );
	CREATE_MULTI_FLOAT( nAttr,  aChannelDither,              "channelDither",                "dcd",    0.5   );
	CREATE_MULTI_BOOL( nAttr,  aChannelFilter,              "channelFilter",                "dcf",    0     );
	CREATE_MULTI_INT( nAttr,  aChannelPixelFilter,         "channelPixelFilter",           "dcpf",   false );
	CREATE_MULTI_FLOAT( nAttr,  aChannelPixelFilterX,        "channelPixelFilterX",          "dcfx",   0.0   );
	CREATE_MULTI_FLOAT( nAttr,  aChannelPixelFilterY,        "channelPixelFilterY",          "dcfy",   0.0   );

	CREATE_BOOL( nAttr,  aCreateOutputDirectories,    "createOutputDirectories",      "cod",    true  );
	CREATE_BOOL( nAttr,  aExpandShaderArrays,         "expandShaderArrays",           "esa",    true  );

	CREATE_BOOL( nAttr,  aBakeRasterOrient,           "bakeRasterOrient",             "bro",    true  );
	CREATE_BOOL( nAttr,  aBakeCullBackface,           "bakeCullBackface",             "bcb",    true  );
	CREATE_BOOL( nAttr,  aBakeCullHidden,             "bakeCullHidden",               "bch",    true  );

	CREATE_STRING( tAttr,  aShaderPath,                 "shaderPath",                   "spth",   ""    );
	CREATE_STRING( tAttr,  aTexturePath,                "texturePath",                  "tpth",   ""    );
	CREATE_STRING( tAttr,  aArchivePath,                "archivePath",                  "apth",   ""    );
	CREATE_STRING( tAttr,  aProceduralPath,             "proceduralPath",               "ppth",   ""    );

	CREATE_STRING( tAttr,  aRibName,                    "ribName",                      "ribn",   ""    );
	CREATE_BOOL( nAttr,  aBeautyRibHasCameraName,     "beautyRibHasCameraName",       "bhcn",   true  );

	CREATE_STRING( tAttr,  aPictureDirectory,           "pictureDirectory",             "picd",   ""    );
	CREATE_STRING( tAttr,  aTextureDirectory,           "textureDirectory",             "texd",   ""    );
	CREATE_STRING( tAttr,  aRibDirectory,               "ribDirectory",                 "ribd",   ""    );
	CREATE_STRING( tAttr,  aShaderDirectory,            "shaderDirectory",              "shdd",   ""    );
	CREATE_STRING( tAttr,  aTempDirectory,              "tempDirectory",                "tmpd",   ""    );

	CREATE_BOOL( nAttr, aDeferredGen,                "deferredGen",                  "defg",   false );
	CREATE_INT( nAttr,  aDeferredBlock,              "deferredBlock",                "defb",   1     );
	CREATE_STRING( tAttr,aPreframeMel,                "preframeMel",                  "prfm",   ""    );
	CREATE_STRING( tAttr,aPostframeMel,               "postframeMel",                 "pofm",   ""    );
	CREATE_BOOL( nAttr,  aUseRenderScript,            "useRenderScript",              "urs",    false );
	CREATE_BOOL( nAttr,  aRemoteRender,               "remoteRender",                 "rr",     false );
	CREATE_BOOL( nAttr,  aNetRManRender,              "netRManRender",                "nrr",    false );
	CREATE_INT( nAttr,   aMinCPU,                     "minCPU",                       "min",    1     );
	CREATE_INT( nAttr,   aMaxCPU,                     "maxCPU",                       "max",    1     );
	CREATE_BOOL( nAttr,  aIgnoreShadows,              "ignoreShadows",                "ish",    false );
	CREATE_BOOL( nAttr,  aShapeOnlyInShadowNames,     "shapeOnlyInShadowNames",       "sosn",   false );
	CREATE_BOOL( nAttr,  aFullShadowRibs,             "fullShadowRibs",               "fsr",    false );
	CREATE_BOOL( nAttr,  aBinaryOutput,               "binaryOutput",                 "bin",    false );
	CREATE_BOOL( nAttr,  aCompressedOutput,           "compressedOutput",             "comp",   false );

	CREATE_BOOL( nAttr,  aOutputMayaPolyCreases,      "outputMayaPolyCreases",        "ompc",    true );
	CREATE_BOOL( nAttr,  aRenderAllCurves,            "renderAllCurves",              "rac",    true );
	CREATE_BOOL( nAttr,  aOutputMeshUVs,              "outputMeshUVs",                "muv",    false );
	CREATE_BOOL( nAttr,  aOutputMeshAsRMSArrays,      "outputMeshAsRMSArrays",        "rmsuv",  false );
	CREATE_BOOL( nAttr,  aIlluminateByDefault,        "illuminateByDefault",          "ilbd",   true );
	CREATE_BOOL( nAttr,  aLiquidSetLightLinking,      "liquidSetLightLinking",        "setll",   false );
	CREATE_BOOL( nAttr,  aIgnoreSurfaces,             "ignoreSurfaces",               "isrf",   false );
	CREATE_BOOL( nAttr,  aIgnoreDisplacements,        "ignoreDisplacements",          "idsp",   false );
	CREATE_BOOL( nAttr,  aIgnoreLights,               "ignoreLights",                 "ilgt",   false );
	CREATE_BOOL( nAttr,  aIgnoreVolumes,              "ignoreVolumes",                "ivol",   false );
	CREATE_BOOL( nAttr,  aOutputShadersInShadows,     "outputShadersInShadows",       "osis",   false );
	CREATE_BOOL( nAttr,  aOutputShadersInDeepShadows, "outputShadersInDeepShadows",   "osids",  false );
	CREATE_BOOL( nAttr,  aOutputLightsInDeepShadows,  "outputLightsInDeepShadows",    "olids",  false );
	CREATE_BOOL( nAttr,  aExportAllShadersParameters, "exportAllShadersParameters",   "easp",  false );

	CREATE_BOOL( nAttr,  aOutputShadowPass,           "outputShadowPass",             "osp",    false );
	CREATE_BOOL( nAttr,  aOutputHeroPass,             "outputHeroPass",               "ohp",    true  );
	CREATE_BOOL( nAttr,  aOutputComments,             "outputComments",               "oc",     false );
	CREATE_BOOL( nAttr,  aShaderDebug,                "shaderDebug",                  "sdbg",   false );
	CREATE_BOOL( nAttr,  aShowProgress,               "showProgress",                 "prog",   false );
	CREATE_BOOL( nAttr,  aDoAnimation,                "doAnimation",                  "anim",   false );
	CREATE_STRING( tAttr,aFrameSequence,              "frameSequence",                "fseq",   "1-100@1" );
	CREATE_BOOL( nAttr,  aDoPadding,                  "doPadding",                    "dpad",   true  );
	CREATE_INT( nAttr,  aPadding,                     "padding",                      "pad",    4     );
	CREATE_INT( nAttr,  aNumProcs,                    "numProcs",                     "np",     0     );
	CREATE_FLOAT( nAttr,  aGain,                      "gain",                         "gn",     1.0   );
	CREATE_FLOAT( nAttr,  aGamma,                     "gamma",                        "gm",     1.0   );
	CREATE_INT( nAttr,  aXResolution,                "xResolution",                  "xres",   1024  );
	CREATE_INT( nAttr,  aYResolution,                "yResolution",                  "yres",   768   );
	CREATE_FLOAT( nAttr,  aPixelAspectRatio,          "pixelAspectRatio",             "par",    1.0   );

	CREATE_BOOL(  nAttr,  aCameraBlur,                 "cameraBlur",                   "cb",     false  );
	CREATE_BOOL(  nAttr,  aTransformationBlur,         "transformationBlur",           "tb",     false  );
	CREATE_BOOL(  nAttr,  aDeformationBlur,            "deformationBlur",              "db",     false  );
	CREATE_INT(   nAttr,  aShutterConfig,              "shutterConfig",                "shc",    0     );
	CREATE_FLOAT( nAttr,  aShutterEfficiency,          "shutterEfficiency",            "shef",   1.0   );
	CREATE_INT(   nAttr,  aMotionBlurSamples,          "motionBlurSamples",            "mbs",    2     );
	CREATE_BOOL(  nAttr,  aRelativeMotion,             "relativeMotion",            	"rmot",   false );
	CREATE_FLOAT( nAttr,  aMotionFactor,               "motionFactor",                 "mf",     1.0   );
	CREATE_BOOL(  nAttr,  aDepthOfField,               "depthOfField",                 "dof",    false );

	CREATE_INT(   nAttr,  aPixelSamples,               "pixelSamples",                 "ps",     4     );
	CREATE_FLOAT( nAttr,  aShadingRate,                "shadingRate",                  "sr",     1.0   );

	CREATE_COLOR( nAttr,  aLimitsOThreshold,           "limitsOThreshold",             "lot",    0.996, 0.996, 0.996  );
	CREATE_COLOR( nAttr,  aLimitsZThreshold,           "limitsZThreshold",             "lzt",    0.996, 0.996, 0.996  );
	CREATE_INT(   nAttr,  aLimitsBucketXSize,          "limitsBucketXSize",            "lbsx",   16    );
	CREATE_INT(   nAttr,  aLimitsBucketYSize,          "limitsBucketYSize",            "lbsy",   16    );
	CREATE_INT(   nAttr,  aLimitsGridSize,             "limitsGridSize",               "lgs",    256   );
	CREATE_LONG(  nAttr,  aLimitsTextureMemory,        "limitsTextureMemory",          "ltm",    65536 );
	CREATE_INT(   nAttr,  aLimitsEyeSplits,            "limitsEyeSplits",              "les",    10    );
	CREATE_INT(   nAttr,  aLimitsGPrimSplits,          "limitsGPrimSplits",            "lges",   4     );

	CREATE_BOOL(   nAttr,  aRibRelativeTransforms,     "ribRelativeTransforms",       	"rxf",    false );

	CREATE_BOOL(   nAttr,  aCleanRib,                  "cleanRib",                     "clr",    false );
	CREATE_BOOL(   nAttr,  aCleanTex,                  "cleanTex",                     "clt",    false );
	CREATE_BOOL(   nAttr,  aCleanShad,                 "cleanShad",                    "cls",    false );
	CREATE_BOOL(   nAttr,  aCleanRenderScript,         "cleanRenderScript",            "clrs",   false );
	CREATE_BOOL(   nAttr,  aJustRib,                   "justRib",                      "jr",     false );

	CREATE_STRING( tAttr,  aAlfredTags,                "alfredTags",                   "alft",   "prman"    );
	CREATE_STRING( tAttr,  aAlfredServices,            "alfredServices",               "alfs",   "pixarRender"    );
  CREATE_STRING( tAttr,  aDirMaps,                   "dirmaps",                      "dmps",   ""    );
	CREATE_STRING( tAttr,  aRenderCommand,             "renderCommand",                "rdc",    ""    );
	CREATE_STRING( tAttr,  aRibgenCommand,             "ribgenCommand",                "rgc",    "liquid"    );

	CREATE_STRING( tAttr,  aPreviewer,                 "previewer",                    "prv",    ""    );
	CREATE_STRING( tAttr,  aPreCommand,                "preCommand",                   "prc",    ""    );
	CREATE_STRING( tAttr,  aPostFrameCommand,          "postFrameCommand",             "pofc",   ""    );
	CREATE_STRING( tAttr,  aPreFrameCommand,           "preFrameCommand",              "prfc",   ""    );
	CREATE_STRING( tAttr,  aPreJobCommand,             "preJobCommand",                "prjc",   ""    );
	CREATE_STRING( tAttr,  aPostJobCommand,            "postJobCommand",               "pojc",   ""    );
	CREATE_STRING( tAttr,  aKey,                       "key",                          "k",      "liquid"    );
	CREATE_STRING( tAttr,  aService,                   "service",                      "srv",    "pixarMTOR"    );
	CREATE_STRING( tAttr,  aLastRenderScript,          "lastRenderScript",             "lrs",    ""    );
	CREATE_STRING( tAttr,  aLastRibFile,               "lastRibFile",                  "lrf",    ""    );

	CREATE_BOOL(   nAttr,  aSimpleGlobalsWindow,        "simpleGlobalsWindow",          "sgw",    false );
	CREATE_BOOL(   nAttr,  aLazyCompute,                "lazyCompute",                  "lc",     false );
	CREATE_FLOAT(  nAttr,  aCropX1,                     "cropX1",                       "cx1",    0.0   );
	CREATE_FLOAT(  nAttr,  aCropX2,                     "cropX2",                       "cx2",    1.0   );
	CREATE_FLOAT(  nAttr,  aCropY1,                     "cropY1",                       "cy1",    0.0   );
	CREATE_FLOAT(  nAttr,  aCropY2,                     "cropY2",                       "cy2",    1.0   );
	CREATE_BOOL(   nAttr,  aExportReadArchive,          "exportReadArchive",            "era",    false );
	CREATE_STRING( tAttr,  aRenderJobName,              "renderJobName",                "rjn",    ""    );
	CREATE_BOOL(   nAttr,  aShortShaderNames,           "shortShaderNames",             "ssn",    false );

	CREATE_BOOL( nAttr,    aRelativeFileNames,          "relativeFileNames",            "rfn",    false );
	CREATE_BOOL( nAttr,    aExpandAlfred,               "expandAlfred",                 "ea",     false );

	CREATE_STRING( tAttr,  aPreFrameBeginMel,           "preFrameBeginMel",                "prfbm",   ""    );
	CREATE_STRING( tAttr,  aPreWorldMel,                "preWorldMel",                     "prwm",    ""    );
	CREATE_STRING( tAttr,  aPostWorldMel,               "postWorldMel",                    "powm",    ""    );
	CREATE_STRING( tAttr,  aPreGeomMel,                 "preGeomMel",                      "prgm",    ""    );

	CREATE_STRING( tAttr,  aPreFrameBegin,              "preFrameBegin",                "prfb",   ""    );
	CREATE_STRING( tAttr,  aPreWorld,                   "preWorld",                     "prw",    ""    );
	CREATE_STRING( tAttr,  aPostWorld,                  "postWorld",                    "pow",    ""    );
	CREATE_STRING( tAttr,  aPreGeom,                    "preGeom",                      "prg",    ""    );

	CREATE_INT( nAttr,     aRenderScriptFormat,         "renderScriptFormat",           "rsf",    2     );
	CREATE_STRING( tAttr,  aRenderScriptCommand,        "renderScriptCommand",          "rsc",    ""    );
	CREATE_STRING( tAttr,  aRenderScriptFileName,       "renderScriptFileName",          "rsn",    ""    );

	CREATE_STRING( tAttr,  aFluidShaderBrowserDefaultPath, "fluidShaderBrowserDefaultPath",  "fsbdp",  "" );

	CREATE_INT( nAttr,     aPreviewType,                "previewType",                  "prt",    0     );
	CREATE_STRING( tAttr,  aPreviewRenderer,            "previewRenderer",              "prr",    ""    );
	CREATE_INT( nAttr,     aPreviewSize,                "previewSize",                  "prs",    128   );
	CREATE_INT( nAttr,     aPreviewPrimitive,           "previewPrimitive",             "prp",    0     );
	CREATE_STRING( tAttr,  aPreviewDisplayDriver,       "previewDisplayDriver",         "prdd",   ""    );
	CREATE_INT( nAttr,     aPreviewConnectionType,      "previewConnectionType",        "prct",   0     );
	CREATE_BOOL( nAttr,    aRenderViewLocal,            "renderViewLocal",              "rvl",    1     );
	CREATE_LONG( nAttr,    aRenderViewPort,             "renderViewPort",               "rvp",    6667  );
	CREATE_INT( nAttr,     aRenderViewTimeOut,          "renderViewTimeOut",            "rvto",   20    );

	CREATE_BOOL( nAttr,    aUseRayTracing,              "useRayTracing",                "ray",    false );
	CREATE_FLOAT( nAttr,   aTraceBreadthFactor,         "traceBreadthFactor",           "trbf",   1.0   );
	CREATE_FLOAT( nAttr,   aTraceDepthFactor,           "traceDepthFactor",             "trdf",   1.0   );
	CREATE_INT( nAttr,     aTraceMaxDepth,              "traceMaxDepth",                "trmd",   10    );
	CREATE_FLOAT( nAttr,   aTraceSpecularThreshold,     "traceSpecularThreshold",       "trst",   10.0  );
	CREATE_BOOL( nAttr,    aTraceRayContinuation,       "traceRayContinuation",         "trrc",   true     );
	CREATE_LONG( nAttr,    aTraceCacheMemory,           "traceCacheMemory",             "trcm",   30720 );
	CREATE_BOOL( nAttr,    aTraceDisplacements,         "traceDisplacements",           "trd",    false );
	CREATE_FLOAT( nAttr,   aTraceBias,                  "traceBias",                    "trb",    0.05  );
	CREATE_BOOL( nAttr,    aTraceSampleMotion,          "traceSampleMotion",            "tsm",    false );
	CREATE_INT( nAttr,     aTraceMaxSpecularDepth,      "traceMaxSpecularDepth",        "trmsd",  2     );
	CREATE_INT( nAttr,     aTraceMaxDiffuseDepth,       "traceMaxDiffuseDepth",         "trmdd",  2     );

	CREATE_FLOAT( nAttr,   aIrradianceMaxError,         "irradianceMaxError",           "ime",    -1.0  );
	CREATE_FLOAT( nAttr,   aIrradianceMaxPixelDist,     "irradianceMaxPixelDist",       "impd",   -1.0  );
	CREATE_STRING( tAttr,  aIrradianceHandle,           "irradianceHandle",             "ih",     ""    );
	CREATE_INT( nAttr,     aIrradianceFileMode,         "irradianceFileMode",           "ifm",    0     );

  CREATE_STRING( tAttr,  aPhotonGlobalHandle,         "photonGlobalHandle",            "pgh",     ""    );
  CREATE_STRING( tAttr,  aCausticGlobalHandle,        "causticGlobalHandle",           "cgh",     ""    );
  CREATE_INT( nAttr,     aPhotonShadingModel,         "photonShadingModel",            "pshm",  0    );
  CREATE_INT( nAttr,     aPhotonEstimator,            "photonEstimator",               "pest",  0    );

	CREATE_BOOL( nAttr,    aUseMtorSubdiv,              "useMtorSubdiv",                "ums",    false );

	CREATE_INT( nAttr,     aHider,                      "hider",                        "h",      0     );
	// "hidden" hider advanced options - PRMAN ONLY
	CREATE_INT( nAttr,     aJitter,                     "jitter",                       "j",      0     );
	// PRMAN 13 BEGIN
	CREATE_FLOAT( nAttr,   aHiddenApertureNSides,       "hiddenApertureNSides",         "hans",   0.0   );
	CREATE_FLOAT( nAttr,   aHiddenApertureAngle,        "hiddenApertureAngle",          "haa",    0.0   );
	CREATE_FLOAT( nAttr,   aHiddenApertureRoundness,    "hiddenApertureRoundness",      "har",    0.0   );
	CREATE_FLOAT( nAttr,   aHiddenApertureDensity,      "hiddenApertureDensity",        "had",    0.0   );
	CREATE_FLOAT( nAttr,   aHiddenShutterOpeningOpen,   "hiddenShutterOpeningOpen",     "hsoo",   0.0   );
	CREATE_FLOAT( nAttr,   aHiddenShutterOpeningClose,  "hiddenShutterOpeningClose",    "hsoc",   1.0   );
	// PRMAN 13 END
	CREATE_FLOAT( nAttr,   aHiddenOcclusionBound,       "hiddenOcclusionBound",         "hob",    0.0   );
	CREATE_BOOL( nAttr,    aHiddenMpCache,              "hiddenMpCache",                "hmpc",   true  );
	CREATE_INT( nAttr,     aHiddenMpMemory,             "hiddenMpMemory",               "hmpm",   6144  );
	CREATE_STRING( tAttr,  aHiddenMpCacheDir,           "hiddenMpCacheDir",             "hmcd",   ""   );
	CREATE_BOOL( nAttr,    aHiddenSampleMotion,         "hiddenSampleMotion",           "hsm",    true  );
	CREATE_INT( nAttr,     aHiddenSubPixel,             "hiddenSubPixel",               "hsp",    1     );
	CREATE_BOOL( nAttr,    aHiddenExtremeMotionDof,     "hiddenExtremeMotionDof",       "hemd",   false );
	CREATE_INT( nAttr,     aHiddenMaxVPDepth,           "hiddenMaxVPDepth",             "hmvd",  -1     );
	// PRMAN 13 BEGIN
	CREATE_BOOL( nAttr,    aHiddenSigma,                "hiddenSigmaHiding",            "hsh",    false );
	CREATE_FLOAT( nAttr,   aHiddenSigmaBlur,            "hiddenSigmaBlur",              "hshb",   1.0   );
	// PRMAN 13 END

	CREATE_INT( nAttr,     aRaytraceFalseColor,         "raytraceFalseColor",            "rfc",   0     );

	CREATE_INT( nAttr,     aPhotonEmit,                 "photonEmit",                    "phe",   0     );
	CREATE_BOOL( nAttr,    aPhotonSampleSpectrum,       "photonSampleSpectrum",          "phss",  false );

	CREATE_STRING( tAttr,  aDepthMaskZFile,             "depthMaskZFile",               "dmzf",   ""    );
	CREATE_BOOL( nAttr,    aDepthMaskReverseSign,       "depthMaskReverseSign",         "dmrs",   false );
	CREATE_FLOAT( nAttr,   aDepthMaskDepthBias,         "depthMaskDepthBias",           "dmdb",   0.01  );

	CREATE_STRING( tAttr,  aRenderCmdFlags,             "renderCmdFlags",               "rcf",    ""    );
	CREATE_STRING( tAttr,  aShaderInfo,                 "shaderInfo",                   "shi",    ""    );
	CREATE_STRING( tAttr,  aShaderComp,                 "shaderComp",                   "shcp",   ""    );
	CREATE_STRING( tAttr,  aShaderExt,                  "shaderExt",                    "she",    ""    );
	CREATE_STRING( tAttr,  aMakeTexture,                "makeTexture",                  "mtx",    ""    );
	CREATE_STRING( tAttr,  aViewTexture,                "viewTexture",                  "vtx",    ""    );
	CREATE_STRING( tAttr,  aTextureExt,                 "textureExt",                   "txe",    ""    );

	CREATE_STRING( tAttr,  aDshDisplayName,             "dshDisplayName",               "dsdn",   ""    );
	CREATE_STRING( tAttr,  aDshImageMode,               "dshImageMode",                 "dsim",   ""    );

	CREATE_INT(    nAttr,  aStatistics,                 "statistics",                   "st",     0     );
	CREATE_STRING( tAttr,  aStatisticsFile,             "statisticsFile",               "stf",    ""    );

	CREATE_BOOL(   nAttr,  aShadersIgnoreOutputParams,  "shadersIgnoreOutputParams",    "iop",    true  );
	CREATE_STRING( tAttr,  aShadersOutputParamsFilter,  "shadersOutputParamsFilter",    "opf",    "^_*" );
	CREATE_INT(    nAttr,  aShadersMaxCachedAELayouts,  "shadersMaxCachedAELayouts",    "mcl",    10    );

	CREATE_INT( nAttr,     aVerbosity,                  "verbosity",                    "vty",    1     );

	CREATE_COMP( cAttr,    aBits_hiders,                "bits_hiders",                  "bhid" );
  CREATE_BOOL( nAttr,    aBits_hiders_0,              "Hidden",                       "Hidden", 1 );
  CHECK_MSTATUS( cAttr.addChild( aBits_hiders_0 ) );
  CREATE_BOOL( nAttr, aBits_hiders_1, "Photon", "Photon", 0 );
  CHECK_MSTATUS( cAttr.addChild( aBits_hiders_1 ) );
  CREATE_BOOL( nAttr, aBits_hiders_2, "ZBuffer", "ZBuffer", 1 );
  CHECK_MSTATUS( cAttr.addChild( aBits_hiders_2 ) );
  CREATE_BOOL( nAttr, aBits_hiders_3, "Raytrace", "Raytrace", 0 );
  CHECK_MSTATUS( cAttr.addChild( aBits_hiders_3 ) );
  CREATE_BOOL( nAttr, aBits_hiders_4, "OpenGL", "OpenGL", 0 );
  CHECK_MSTATUS( cAttr.addChild( aBits_hiders_4 ) );
  CREATE_BOOL( nAttr, aBits_hiders_5, "DepthMask", "DepthMask", 0 );
  CHECK_MSTATUS( cAttr.addChild( aBits_hiders_5 ) );

	CREATE_COMP( cAttr, aBits_filters, "bits_filters", "bfil" );
	CREATE_BOOL( nAttr, aBits_filters_0, "Box", "Box", 1 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_0 ) );
	CREATE_BOOL( nAttr, aBits_filters_1, "Triangle", "Triangle", 1 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_1 ) );
	CREATE_BOOL( nAttr, aBits_filters_2, "Catmull_Rom", "Catmull_Rom", 1 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_2 ) );
	CREATE_BOOL( nAttr, aBits_filters_3, "Gaussian", "Gaussian", 1 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_3 ) );
	CREATE_BOOL( nAttr, aBits_filters_4, "Sinc", "Sinc", 1 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_4 ) );
	CREATE_BOOL( nAttr, aBits_filters_5, "Blackman_Harris", "Blackman_Harris", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_5 ) );
	CREATE_BOOL( nAttr, aBits_filters_6, "Mitchell", "Mitchell", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_6 ) );
	CREATE_BOOL( nAttr, aBits_filters_7, "SeparableCatmull_Rom", "SeparableCatmull_Rom", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_7 ) );
	CREATE_BOOL( nAttr, aBits_filters_8, "Lanczos", "Lanczos", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_8 ) );
	CREATE_BOOL( nAttr, aBits_filters_9, "Bessel", "Bessel", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_9 ) );
	CREATE_BOOL( nAttr, aBits_filters_10, "Disk", "Disk", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_filters_10 ) );

	CREATE_COMP( cAttr, aBits_features, "bits_features", "bfea" );
	CREATE_BOOL( nAttr, aBits_features_0, "Blobbies", "Blobbies", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_features_0 ) );
	CREATE_BOOL( nAttr, aBits_features_1, "Points", "Points", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_features_1 ) );
	CREATE_BOOL( nAttr, aBits_features_2, "Eyesplits", "Eyesplits", 1 );
	CHECK_MSTATUS( cAttr.addChild( aBits_features_2 ) );
	CREATE_BOOL( nAttr, aBits_features_3, "Raytracing", "Raytracing", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_features_3 ) );
	CREATE_BOOL( nAttr, aBits_features_4, "DepthOfField", "DepthOfField", 1 );
	CHECK_MSTATUS( cAttr.addChild( aBits_features_4 ) );
	CREATE_BOOL( nAttr, aBits_features_5, "AdvancedVisibility", "AdvancedVisibility", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_features_5 ) );
	CREATE_BOOL( nAttr, aBits_features_6, "DisplayChannels", "DisplayChannels", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_features_6 ) );

	CREATE_COMP( cAttr, aBits_required, "bits_required", "breq" );
	CREATE_BOOL( nAttr, aBits_required_0, "Swap_UV", "Swap_UV", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_required_0 ) );
	CREATE_BOOL( nAttr, aBits_required_1, "__Pref", "__Pref", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_required_1 ) );
	CREATE_BOOL( nAttr, aBits_required_2, "MakeShadow", "MakeShadow", 0 );
	CHECK_MSTATUS( cAttr.addChild( aBits_required_2 ) );

	CREATE_STRING( tAttr,  aShotName,                   "shotName",                     "sn",     ""    );
	CREATE_STRING( tAttr,  aShotVersion,                "shotVersion",                  "sv",     ""    );


	return MS::kSuccess;
}





