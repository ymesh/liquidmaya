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
** Liquid Globals Node Header
** ______________________________________________________________________
*/

#ifndef liqGlobalsNode_H
#define liqGlobalsNode_H

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MImage.h>
#include <maya/MSwatchRenderBase.h>
#include <maya/MFnDependencyNode.h>

#include "liquid.h"
#include "liqNodeSwatch.h"

class liqGlobalsNode : public MPxNode
{
  public:
                      liqGlobalsNode();
    virtual          ~liqGlobalsNode();

    static  void *    creator();
    static  MStatus   initialize();

    //  Id tag for use with binary file format
    static  MTypeId   id;

  private:

    // attributes
    static MObject aLaunchRender;

    static MObject aRenderCamera;
    static MObject aRotateCamera;
    static MObject aIgnoreAOVDisplays;
    static MObject aDdImageName;
    static MObject aDdImageType;
    static MObject aDdImageMode;
    static MObject aDdParamType;
    static MObject aDdEnable;
    static MObject aDdQuantizeEnabled;
    static MObject aDdBitDepth;
    static MObject aDdDither;
    static MObject aDdFilterEnabled;
    static MObject aDdPixelFilter;
    static MObject aDdPixelFilterX;
    static MObject aDdPixelFilterY;
    static MObject aDdXtraParamNames;
    static MObject aDdXtraParamTypes;
    static MObject aDdXtraParamDatas;
    static MObject aNumDD;
    static MObject aNumDDParam;

    static MObject aChannelName;
    static MObject aChannelType;
    static MObject aChannelArraySize;
    static MObject aChannelQuantize;
    static MObject aChannelBitDepth;
    static MObject aChannelDither;
    static MObject aChannelFilter;
    static MObject aChannelPixelFilter;
    static MObject aChannelPixelFilterX;
    static MObject aChannelPixelFilterY;

    static MObject aCreateOutputDirectories;
    static MObject aExpandShaderArrays;

    static MObject aBakeRasterOrient;
    static MObject aBakeCullBackface;
    static MObject aBakeCullHidden;

    static MObject aShaderPath;
    static MObject aTexturePath;
    static MObject aArchivePath;
    static MObject aProceduralPath;

    static MObject aRibName;
    static MObject aBeautyRibHasCameraName;

    static MObject aPictureDirectory;
    static MObject aTextureDirectory;
    static MObject aRibDirectory;
    static MObject aShaderDirectory;
    static MObject aTempDirectory;

    static MObject aDeferredGen;
    static MObject aDeferredBlock;
    static MObject aPreframeMel;
    static MObject aPostframeMel;
    static MObject aUseRenderScript;
    static MObject aRemoteRender;
    static MObject aNetRManRender;
    static MObject aMinCPU;
    static MObject aMaxCPU;
    static MObject aIgnoreShadows;
    static MObject aShapeOnlyInShadowNames;
    static MObject aFullShadowRibs;
    static MObject aBinaryOutput;
    static MObject aCompressedOutput;
    static MObject aOutputMayaPolyCreases;
    static MObject aRenderAllCurves;
    static MObject aOutputMeshUVs;
    static MObject aOutputMeshAsRMSArrays;
    static MObject aIlluminateByDefault;
    static MObject aLiquidSetLightLinking;
    static MObject aIgnoreSurfaces;
    static MObject aIgnoreDisplacements;
    static MObject aIgnoreLights;
    static MObject aIgnoreVolumes;
    static MObject aOutputShadersInShadows;
    static MObject aOutputShadersInDeepShadows;
    static MObject aOutputLightsInDeepShadows;
	static MObject aExportAllShadersParameters;

    static MObject aOutputShadowPass;
    static MObject aOutputHeroPass;
    static MObject aOutputComments;
    static MObject aShaderDebug;
    static MObject aShowProgress;
    static MObject aDoAnimation;
    static MObject aFrameSequence;
    static MObject aDoPadding;
    static MObject aPadding;
    static MObject aNumProcs;
    static MObject aGain;
    static MObject aGamma;
    static MObject aXResolution;
    static MObject aYResolution;
    static MObject aPixelAspectRatio;
    static MObject aImageDriver;

    static MObject aCameraBlur;
    static MObject aTransformationBlur;
    static MObject aDeformationBlur;
    static MObject aShutterConfig;
    static MObject aShutterEfficiency;
    static MObject aMotionBlurSamples;
    static MObject aRelativeMotion;
    static MObject aMotionFactor;
    static MObject aDepthOfField;

    static MObject aPixelSamples;
    static MObject aShadingRate;

    static MObject aLimitsOThreshold;
    static MObject aLimitsZThreshold;
    static MObject aLimitsBucketXSize;
    static MObject aLimitsBucketYSize;
    static MObject aLimitsGridSize;
    static MObject aLimitsTextureMemory;
    static MObject aLimitsEyeSplits;
    static MObject aLimitsGPrimSplits;

    static MObject aRibRelativeTransforms;

    static MObject aCleanRib;
    static MObject aCleanTex;
    static MObject aCleanShad;
    static MObject aCleanRenderScript;
    static MObject aJustRib;
    static MObject aAlfredTags;
    static MObject aAlfredServices;
    static MObject aDirMaps;
    static MObject aRenderCommand;
    static MObject aRibgenCommand; 
    
    static MObject aPreviewer;
    static MObject aPreCommand;
    static MObject aPostFrameCommand;
    static MObject aPreFrameCommand;
    static MObject aPreJobCommand;
    static MObject aPostJobCommand;
    static MObject aKey;
    static MObject aService;
    static MObject aLastRenderScript;
    static MObject aLastRibFile;

    static MObject aSimpleGlobalsWindow;
    static MObject aLazyCompute;
    static MObject aCropX1;
    static MObject aCropX2;
    static MObject aCropY1;
    static MObject aCropY2;
    static MObject aExportReadArchive;
    static MObject aRenderJobName;
    static MObject aShortShaderNames;

    static MObject aRelativeFileNames;

    static MObject aExpandAlfred;

    static MObject aPreFrameBeginMel;
    static MObject aPreWorldMel;
    static MObject aPostWorldMel;
    static MObject aPreGeomMel;

    static MObject aPreFrameBegin;
    static MObject aPreWorld;
    static MObject aPostWorld;
    static MObject aPreGeom;

    static MObject aRenderScriptFormat;
    static MObject aRenderScriptCommand;
    static MObject aRenderScriptFileName;

    static MObject aFluidShaderBrowserDefaultPath;

    static MObject aPreviewType;
    static MObject aPreviewRenderer;
    static MObject aPreviewSize;
    static MObject aPreviewPrimitive;
    static MObject aPreviewDisplayDriver;
    static MObject aPreviewConnectionType;
    static MObject aRenderViewLocal;
    static MObject aRenderViewPort;
    static MObject aRenderViewTimeOut;

    static MObject aUseRayTracing;
    static MObject aTraceBreadthFactor;
    static MObject aTraceDepthFactor;
    static MObject aTraceMaxDepth;
    static MObject aTraceSpecularThreshold;
    static MObject aTraceRayContinuation;
    static MObject aTraceCacheMemory;
    static MObject aTraceDisplacements;
    static MObject aTraceBias;
    static MObject aTraceSampleMotion;
    static MObject aTraceMaxSpecularDepth;
    static MObject aTraceMaxDiffuseDepth;

    static MObject aIrradianceMaxError;
    static MObject aIrradianceMaxPixelDist;
    static MObject aIrradianceHandle;
    static MObject aIrradianceFileMode;

    static MObject aPhotonGlobalHandle;
    static MObject aCausticGlobalHandle;
    static MObject aPhotonShadingModel;
    static MObject aPhotonEstimator;

    static MObject aUseMtorSubdiv;
    static MObject aHider;
    static MObject aJitter;
    static MObject aRenderCmdFlags;

    static MObject aShaderInfo;
    static MObject aShaderComp;
    static MObject aShaderExt;
    static MObject aMakeTexture;
    static MObject aViewTexture;
    static MObject aViewTextureCmd;
    static MObject aViewTextureFilt;
    static MObject aTextureExt;

    static MObject aBits_hiders;
    static MObject aBits_hiders_0;
    static MObject aBits_hiders_1;
    static MObject aBits_hiders_2;
    static MObject aBits_hiders_3;
    static MObject aBits_hiders_4;
    static MObject aBits_hiders_5;

    static MObject aBits_filters;
    static MObject aBits_filters_0;
    static MObject aBits_filters_1;
    static MObject aBits_filters_2;
    static MObject aBits_filters_3;
    static MObject aBits_filters_4;
    static MObject aBits_filters_5;
    static MObject aBits_filters_6;
    static MObject aBits_filters_7;
    static MObject aBits_filters_8;
    static MObject aBits_filters_9;
    static MObject aBits_filters_10;

    static MObject aBits_features;
    static MObject aBits_features_0;
    static MObject aBits_features_1;
    static MObject aBits_features_2;
    static MObject aBits_features_3;
    static MObject aBits_features_4;
    static MObject aBits_features_5;
    static MObject aBits_features_6;

    static MObject aBits_required;
    static MObject aBits_required_0;
    static MObject aBits_required_1;
    static MObject aBits_required_2;

    static MObject aDshDisplayName;
    static MObject aDshImageMode;

    static MObject aShotName;
    static MObject aShotVersion;

    static MObject aStatistics;
    static MObject aStatisticsFile;

    static MObject aShadersIgnoreOutputParams;
    static MObject aShadersOutputParamsFilter;
    static MObject aShadersMaxCachedAELayouts;

    // PRMAN 13 BEGIN
    static MObject aHiddenApertureNSides;
    static MObject aHiddenApertureAngle;
    static MObject aHiddenApertureRoundness;
    static MObject aHiddenApertureDensity;
    static MObject aHiddenShutterOpeningOpen;
    static MObject aHiddenShutterOpeningClose;
    // PRMAN 13 END
    static MObject aHiddenOcclusionBound;
    static MObject aHiddenMpCache;
    static MObject aHiddenMpMemory;
    static MObject aHiddenMpCacheDir;
    static MObject aHiddenSampleMotion;
    static MObject aHiddenSubPixel;
    static MObject aHiddenExtremeMotionDof;
    static MObject aHiddenMaxVPDepth;
    // PRMAN 13 BEGIN
    static MObject aHiddenSigma;
    static MObject aHiddenSigmaBlur;
    // PRMAN 13 END

    static MObject aRaytraceFalseColor;

    static MObject aPhotonEmit;
    static MObject aPhotonSampleSpectrum;

    static MObject aDepthMaskZFile;
    static MObject aDepthMaskReverseSign;
    static MObject aDepthMaskDepthBias;

    static MObject aVerbosity;
};




#endif
