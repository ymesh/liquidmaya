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
** Liquid Light Shader Node Source
** ______________________________________________________________________
*/


#include <liquid.h>
#include <liqLightNode.h>
#include <liqNodeSwatch.h>
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
#include <maya/MFnMessageAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSwatchRenderBase.h>
#include <maya/MSwatchRenderRegister.h>
#include <maya/MImage.h>
#include <maya/MFnDependencyNode.h>



#include <liqIOStream.h>

// static data
MTypeId liqLightNode::id( liqLightNodeId );

// Attributes
MObject liqLightNode::aRmanShader;
MObject liqLightNode::aRmanShaderLong;
MObject liqLightNode::aRmanShaderLif;
MObject liqLightNode::aRmanParams;
MObject liqLightNode::aRmanDetails;
MObject liqLightNode::aRmanTypes;
MObject liqLightNode::aRmanDefaults;
MObject liqLightNode::aRmanArraySizes;
MObject liqLightNode::aRmanLifCmds;
MObject liqLightNode::aRmanMethods;
MObject liqLightNode::aRmanIsOutput;    
MObject liqLightNode::aRmanAccept;

MObject liqLightNode::aPreviewPrimitive;
MObject liqLightNode::aPreviewCustomPrimitive;
MObject liqLightNode::aOutputInShadow;
MObject liqLightNode::aResolution;
MObject liqLightNode::aRefreshPreview;
MObject liqLightNode::aPreviewObjectSize;
MObject liqLightNode::aPreviewShadingRate;
MObject liqLightNode::aPreviewBackplane;

MObject liqLightNode::aGenerateMainShadow;
MObject liqLightNode::aDeepShadows;
MObject liqLightNode::aPixelSamples;
MObject liqLightNode::aVolumeInterpretation; 
MObject liqLightNode::aDeepShadowsDisplayMode;
MObject liqLightNode::aEveryFrame;
MObject liqLightNode::aRenderAtFrame;
MObject liqLightNode::aAggregateShadowMaps;
MObject liqLightNode::aGeometrySet;
MObject liqLightNode::aShadingRateFactor;
MObject liqLightNode::aNearClipPlane;
MObject liqLightNode::aFarClipPlane;

MObject liqLightNode::aShadowMainCamera;

MObject liqLightNode::aShadowCameras;

MObject liqLightNode::aOutColor;
MObject liqLightNode::aOutTransparency;
MObject liqLightNode::aAssignedObjects;

#define MAKE_INPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(true)); 		\
    CHECK_MSTATUS(attr.setStorable(true));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
    CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_NONKEYABLE_INPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(false)); 		\
    CHECK_MSTATUS(attr.setStorable(true));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
    CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_OUTPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(false)); 		\
    CHECK_MSTATUS(attr.setStorable(false));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
    CHECK_MSTATUS(attr.setWritable(false));

void liqLightNode::postConstructor( )
{
  setMPSafe(true);

  // init swatch
  if ( swatchInit != true ) 
  {
    MObject obj = MPxNode::thisMObject();
    renderSwatch = new liqNodeSwatch( obj, obj, 128 );
    swatchInit = true;
  }

  MGlobal::executeCommandOnIdle( "liquidCheckGlobals()", false );
}

liqLightNode::liqLightNode()
{
  swatchInit = false;
  renderSwatch = NULL;
}

liqLightNode::~liqLightNode()
{
  if (renderSwatch != NULL) delete renderSwatch;
}

void* liqLightNode::creator()
{
    return new liqLightNode();
}

MStatus liqLightNode::initialize()
{
  MFnTypedAttribute   tAttr;
  MFnNumericAttribute nAttr;
  MFnEnumAttribute    eAttr;
  MFnMessageAttribute mAttr;
  MStatus status;

  // Create input attributes

  aRmanShader = tAttr.create( MString( "rmanShader"), MString( "rms"), MFnData::kString, aRmanShader, &status );
  MAKE_INPUT(tAttr);

  aRmanShaderLong = tAttr.create( MString( "rmanShaderLong"), MString( "rml"), MFnData::kString, aRmanShaderLong, &status );
  MAKE_INPUT(tAttr);

  aRmanShaderLif = tAttr.create(  MString( "rmanShaderLif"), MString( "lif"), MFnData::kString, aRmanShaderLif, &status );
  MAKE_INPUT(tAttr);

  aRmanParams = tAttr.create(  MString( "rmanParams"), MString( "rpr"), MFnData::kStringArray, aRmanParams, &status );
  MAKE_INPUT(tAttr);

  aRmanDetails = tAttr.create(  MString( "rmanDetails"), MString( "rdt"), MFnData::kStringArray, aRmanDetails, &status );
  MAKE_INPUT(tAttr);

  aRmanTypes = tAttr.create(  MString( "rmanTypes"), MString( "rty"), MFnData::kStringArray, aRmanTypes, &status );
  MAKE_INPUT(tAttr);

  aRmanDefaults = tAttr.create(  MString( "rmanDefaults"), MString( "rdf"), MFnData::kStringArray, aRmanDefaults, &status );
  MAKE_INPUT(tAttr);

  aRmanArraySizes = tAttr.create(  MString( "rmanArraySizes"), MString( "ras"), MFnData::kIntArray, aRmanArraySizes, &status );
  MAKE_INPUT(tAttr);

  aRmanLifCmds = tAttr.create(  MString( "rmanLifCmds"), MString( "rlc"), MFnData::kStringArray, aRmanLifCmds, &status );
  MAKE_INPUT(tAttr);

  aRmanMethods = tAttr.create(  MString("rmanMethods"),  MString("rmt"), MFnData::kStringArray, aRmanMethods, &status );
  MAKE_INPUT(tAttr);
  
  aRmanIsOutput = tAttr.create(  MString("rmanIsOutput"),  MString("rio"), MFnData::kIntArray, aRmanIsOutput, &status );
  MAKE_INPUT(tAttr);
  
  aRmanAccept = tAttr.create(  MString("rmanAccept"),  MString("rma"), MFnData::kStringArray, aRmanAccept, &status );
  MAKE_INPUT(tAttr);

  aPreviewPrimitive = eAttr.create( "previewPrimitive", "pvp", 7, &status );
  eAttr.addField( "Sphere",  0 );
  eAttr.addField( "Cube",    1 );
  eAttr.addField( "Cylinder", 2 );
  eAttr.addField( "Torus",   3 );
  eAttr.addField( "Plane",   4 );
  eAttr.addField( "Teapot",  5 );
  eAttr.addField( "Custom",  6 );
  eAttr.addField( "(globals)",7 );
  MAKE_NONKEYABLE_INPUT(eAttr);
  CHECK_MSTATUS(eAttr.setConnectable(false));

  aPreviewCustomPrimitive = tAttr.create( "previewCustomPrimitive", "pcp", MFnData::kString, aPreviewCustomPrimitive, &status );
  MAKE_INPUT(tAttr);

  aPreviewObjectSize = nAttr.create( "previewObjectSize", "pos", MFnNumericData::kDouble, 1.0, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false));

  aPreviewShadingRate = nAttr.create( "previewShadingRate", "psr", MFnNumericData::kDouble, 1.0, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false));

  aPreviewBackplane = nAttr.create( "previewBackplane", "pbp", MFnNumericData::kBoolean, true, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false));

  aOutputInShadow = nAttr.create( "outputInShadow", "ois", MFnNumericData::kBoolean, 0.0, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);

  // refreshPreview must be true to allow refresh
  aRefreshPreview = nAttr.create( "refreshPreview", "rfp", MFnNumericData::kBoolean, 0.0, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setHidden(true));

  aGenerateMainShadow = nAttr.create( "generateMainShadow", "gms", MFnNumericData::kBoolean, 1.0, &status );
  MAKE_INPUT(nAttr);
  aDeepShadows = nAttr.create( "deepShadows", "dsh", MFnNumericData::kBoolean, 0.0, &status );
  MAKE_INPUT(nAttr);
  aPixelSamples = nAttr.create( "pixelSamples", "dps", MFnNumericData::kInt, 1.0, &status );
  MAKE_INPUT(nAttr);
  aVolumeInterpretation = eAttr.create( "volumeInterpretation", "dvi", 0, &status );
  eAttr.addField( "Discreet",  0 );
  eAttr.addField( "Continuous", 1 );
  MAKE_INPUT(eAttr);

  aDeepShadowsDisplayMode = eAttr.create( "liqDeepShadowsDisplayMode", "dshdm", 0, &status );
  eAttr.addField( "Default",  0 );
  eAttr.addField( "deepprevdisttotal", 1 );
  MAKE_INPUT(eAttr);

  aEveryFrame = nAttr.create( "everyFrame", "ef", MFnNumericData::kBoolean, 1.0, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);
  aRenderAtFrame = nAttr.create( "renderAtFrame", "raf", MFnNumericData::kInt, 0, &status );
  MAKE_INPUT(nAttr);
  aAggregateShadowMaps = nAttr.create( "aggregateShadowMaps", "asm", MFnNumericData::kBoolean, 0.0, &status );
  MAKE_INPUT(nAttr);
  aGeometrySet = tAttr.create( "geometrySet", "sgs", MFnData::kString, aGeometrySet, &status );
  MAKE_INPUT(tAttr);

  aShadingRateFactor = nAttr.create( "shadingRateFactor", "srf", MFnNumericData::kDouble, 1.0, &status );
  MAKE_INPUT(nAttr);
  aNearClipPlane = nAttr.create( "nearClipPlane", "ncp", MFnNumericData::kDouble, 0.001, &status );
  MAKE_INPUT(nAttr);
  aFarClipPlane = nAttr.create( "farClipPlane", "fcp", MFnNumericData::kDouble, 250000.0, &status );
  MAKE_INPUT(nAttr);
  
  aShadowMainCamera = tAttr.create( "shadowMainCamera", "smc", MFnData::kString, aShadowMainCamera, &status );
  MAKE_INPUT(tAttr);

  aShadowCameras = mAttr.create( "shadowCameras", "shc" );
  MAKE_INPUT(mAttr);
  CHECK_MSTATUS( mAttr.setArray(true) );

  // resolution attribute for maya's hardware renderer
  aResolution = nAttr.create( "resolution", "res", MFnNumericData::kInt, 16, &status );
  nAttr.setStorable( true );
  nAttr.setReadable( true );
  nAttr.setWritable( true );
  nAttr.setHidden( true );

  // Create output attributes
  aOutColor = nAttr.createColor( "outColor", "oc");
  MAKE_OUTPUT(nAttr);
  aOutTransparency = nAttr.createColor( "outTransparency", "ot");
  MAKE_OUTPUT(nAttr);
  aAssignedObjects = mAttr.create( "assignedObjects", "ao");
  MAKE_OUTPUT(mAttr);


  CHECK_MSTATUS(addAttribute(aRmanShader));
  CHECK_MSTATUS(addAttribute(aRmanShaderLong));
  CHECK_MSTATUS(addAttribute(aRmanShaderLif));
  CHECK_MSTATUS(addAttribute(aRmanParams));
  CHECK_MSTATUS(addAttribute(aRmanDetails));
  CHECK_MSTATUS(addAttribute(aRmanTypes));
  CHECK_MSTATUS(addAttribute(aRmanDefaults));
  CHECK_MSTATUS(addAttribute(aRmanArraySizes));
  CHECK_MSTATUS(addAttribute(aRmanLifCmds));
  CHECK_MSTATUS( addAttribute(aRmanMethods) );
  CHECK_MSTATUS( addAttribute(aRmanIsOutput) );
  CHECK_MSTATUS(addAttribute(aRmanAccept));

  CHECK_MSTATUS(addAttribute(aPreviewPrimitive));
  CHECK_MSTATUS(addAttribute(aPreviewCustomPrimitive));
  CHECK_MSTATUS(addAttribute(aPreviewObjectSize));
  CHECK_MSTATUS(addAttribute(aPreviewShadingRate));
  CHECK_MSTATUS(addAttribute(aPreviewBackplane));

  CHECK_MSTATUS(addAttribute(aGenerateMainShadow));
  CHECK_MSTATUS(addAttribute(aOutputInShadow));
  CHECK_MSTATUS(addAttribute(aDeepShadows));
  CHECK_MSTATUS(addAttribute(aPixelSamples));
  CHECK_MSTATUS(addAttribute(aVolumeInterpretation));
   CHECK_MSTATUS(addAttribute(aDeepShadowsDisplayMode));
  CHECK_MSTATUS(addAttribute(aEveryFrame));
  CHECK_MSTATUS(addAttribute(aRenderAtFrame));
  CHECK_MSTATUS(addAttribute(aAggregateShadowMaps));
  CHECK_MSTATUS(addAttribute(aGeometrySet));
  CHECK_MSTATUS(addAttribute(aShadingRateFactor));
  CHECK_MSTATUS(addAttribute(aNearClipPlane));
  CHECK_MSTATUS(addAttribute(aFarClipPlane));
  
  CHECK_MSTATUS(addAttribute(aShadowMainCamera));

  CHECK_MSTATUS(addAttribute(aShadowCameras));

  CHECK_MSTATUS(addAttribute(aResolution));
  CHECK_MSTATUS(addAttribute(aRefreshPreview));
  CHECK_MSTATUS(addAttribute(aAssignedObjects));
  CHECK_MSTATUS(addAttribute(aOutColor));
  CHECK_MSTATUS(addAttribute(aOutTransparency));

  return MS::kSuccess;
}





