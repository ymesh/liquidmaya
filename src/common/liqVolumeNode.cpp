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
** Liquid Volume Shader Node Source
** ______________________________________________________________________
*/

#include <liquid.h>
#include <liqVolumeNode.h>
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
#include <maya/MFloatVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSwatchRenderBase.h>
#include <maya/MSwatchRenderRegister.h>
#include <maya/MImage.h>
#include <maya/MFnDependencyNode.h>


#include <liqIOStream.h>

// static data
MTypeId liqVolumeNode::id( liqVolumeNodeId );

// Attributes
MObject liqVolumeNode::aRmanShader;
MObject liqVolumeNode::aRmanShaderLong;
MObject liqVolumeNode::aRmanShaderLif;
MObject liqVolumeNode::aRmanParams;
MObject liqVolumeNode::aRmanDetails;
MObject liqVolumeNode::aRmanTypes;
MObject liqVolumeNode::aRmanDefaults;
MObject liqVolumeNode::aRmanArraySizes;
MObject liqVolumeNode::aRmanLifCmds;
MObject liqVolumeNode::aRmanMethods;
MObject liqVolumeNode::aRmanIsOutput;
MObject liqVolumeNode::aRmanAccept;

MObject liqVolumeNode::aPreviewGamma;
MObject liqVolumeNode::aPreviewPrimitive;
MObject liqVolumeNode::aPreviewCustomPrimitive;
MObject liqVolumeNode::aPreviewObjectSize;
MObject liqVolumeNode::aPreviewShadingRate;
MObject liqVolumeNode::aPreviewBackplane;

MObject liqVolumeNode::aVolumeType;
MObject liqVolumeNode::aVisiblePoints;

MObject liqVolumeNode::aColor;
MObject liqVolumeNode::aTransparency;
MObject liqVolumeNode::aOpacity;

MObject liqVolumeNode::aShaderSpace;
MObject liqVolumeNode::aOutputInShadow;
MObject liqVolumeNode::aRefreshPreview;

MObject liqVolumeNode::aOutColor;

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

void liqVolumeNode::postConstructor( )
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

liqVolumeNode::liqVolumeNode()
{
  swatchInit = false;
  renderSwatch = NULL;
}

liqVolumeNode::~liqVolumeNode()
{
  if (renderSwatch != NULL) delete renderSwatch;
}

void* liqVolumeNode::creator()
{
    return new liqVolumeNode();
}

MStatus liqVolumeNode::initialize()
{
  MFnTypedAttribute   tAttr;
  MFnNumericAttribute nAttr;
  MFnEnumAttribute    eAttr;
  MStatus status;

  // Create input attributes
  aRmanShader = tAttr.create( MString("rmanShader"), MString("rms"), MFnData::kString, aRmanShader, &status );
  MAKE_INPUT(tAttr);

  aRmanShaderLong = tAttr.create( MString("rmanShaderLong"), MString("rml"), MFnData::kString, aRmanShaderLong, &status );
  MAKE_INPUT(tAttr);

  aRmanShaderLif = tAttr.create(  MString("rmanShaderLif"),  MString("lif"), MFnData::kString, aRmanShaderLif, &status );
  MAKE_INPUT(tAttr);

  aRmanParams = tAttr.create(  MString("rmanParams"),  MString("rpr"), MFnData::kStringArray, aRmanParams, &status );
  MAKE_INPUT(tAttr);

  aRmanDetails = tAttr.create(  MString("rmanDetails"),  MString("rdt"), MFnData::kStringArray, aRmanDetails, &status );
  MAKE_INPUT(tAttr);

  aRmanTypes = tAttr.create(  MString("rmanTypes"),  MString("rty"), MFnData::kStringArray, aRmanTypes, &status );
  MAKE_INPUT(tAttr);

  aRmanDefaults = tAttr.create(  MString("rmanDefaults"),  MString("rdf"), MFnData::kStringArray, aRmanDefaults, &status );
  MAKE_INPUT(tAttr);

  aRmanArraySizes = tAttr.create(  MString("rmanArraySizes"),  MString("ras"), MFnData::kIntArray, aRmanArraySizes, &status );
  MAKE_INPUT(tAttr);

  aRmanLifCmds = tAttr.create(  MString("rmanLifCmds"),  MString("rlc"), MFnData::kStringArray, aRmanLifCmds, &status );
  MAKE_INPUT(tAttr);

  aRmanMethods = tAttr.create(  MString("rmanMethods"),  MString("rmt"), MFnData::kStringArray, aRmanMethods, &status );
  MAKE_INPUT(tAttr);

  aRmanIsOutput = tAttr.create(  MString("rmanIsOutput"),  MString("rio"), MFnData::kIntArray, aRmanIsOutput, &status );
  MAKE_INPUT(tAttr);
  
  aRmanAccept = tAttr.create(  MString("rmanAccept"),  MString("rma"), MFnData::kStringArray, aRmanAccept, &status );
  MAKE_INPUT(tAttr);

  aPreviewPrimitive = eAttr.create( "previewPrimitive", "pvp", 7, &status );
  eAttr.addField( "Sphere",   0 );
  eAttr.addField( "Cube",     1 );
  eAttr.addField( "Cylinder", 2 );
  eAttr.addField( "Torus",    3 );
  eAttr.addField( "Plane",    4 );
  eAttr.addField( "Teapot",   5 );
  eAttr.addField( "Custom",   6 );
  eAttr.addField( "(globals)",7 );
  MAKE_NONKEYABLE_INPUT(eAttr);
  CHECK_MSTATUS(eAttr.setConnectable(false));

  aVolumeType = eAttr.create( "volumeType", "vot", 3, &status );
  eAttr.addField( "Atmosphere", 0 );
  eAttr.addField( "Interior",   1 );
  eAttr.addField( "Exterior",   2 );
  MAKE_NONKEYABLE_INPUT( eAttr );
  CHECK_MSTATUS( eAttr.setConnectable(false) );

  aColor = nAttr.createColor("color", "cs");
  nAttr.setDefault( 1.0, 1.0, 1.0 );
  nAttr.setDisconnectBehavior( MFnAttribute::kReset );
  MAKE_INPUT(nAttr);

  aOpacity = nAttr.createColor("opacity", "os");
  nAttr.setDefault( 1.0, 1.0, 1.0 );
  MAKE_INPUT(nAttr);

  aTransparency = nAttr.createColor("transparency", "ts"); // Needed by Maya for Open Gl preview in "5" mode, invert opacity in compute
  nAttr.setDefault( 0.0, 0.0, 0.0 );
  MAKE_INPUT(nAttr);

  aPreviewCustomPrimitive = tAttr.create(  MString("previewCustomPrimitive"),  MString("pcp"), MFnData::kString, aPreviewCustomPrimitive, &status );
  MAKE_INPUT(tAttr);

  aPreviewObjectSize = nAttr.create("previewObjectSize", "pos", MFnNumericData::kDouble, 1.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false));

  aPreviewShadingRate = nAttr.create("previewShadingRate", "psr", MFnNumericData::kDouble, 1.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false));

  aPreviewBackplane = nAttr.create("previewBackplane", "pbp", MFnNumericData::kBoolean, true, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false));

  aShaderSpace = tAttr.create( MString("shaderSpace"), MString("ssp"), MFnData::kString, aShaderSpace, &status );
  MAKE_INPUT(tAttr);

  aOutputInShadow = nAttr.create("outputInShadow", "ois",  MFnNumericData::kBoolean, 0.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);

  aVisiblePoints = nAttr.create( "useVisiblePoints", "uvp", MFnNumericData::kBoolean, false, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS( nAttr.setConnectable(false ) );

  // refreshPreview must be true to allow refresh
  aRefreshPreview = nAttr.create("refreshPreview", "rfp",  MFnNumericData::kBoolean, 0.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setHidden(true));

  aPreviewGamma = nAttr.create( "previewGamma", "pg", MFnNumericData::kFloat, 1, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( true ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( true ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  // Create output attributes
  aOutColor = nAttr.createColor("outColor", "oc");
  MAKE_OUTPUT(nAttr);

  CHECK_MSTATUS(addAttribute(aRmanShader));
  CHECK_MSTATUS(addAttribute(aRmanShaderLong));
  CHECK_MSTATUS(addAttribute(aRmanShaderLif));
  CHECK_MSTATUS(addAttribute(aRmanParams));
  CHECK_MSTATUS(addAttribute(aRmanDetails));
  CHECK_MSTATUS(addAttribute(aRmanTypes));
  CHECK_MSTATUS(addAttribute(aRmanDefaults));
  CHECK_MSTATUS(addAttribute(aRmanArraySizes));
  CHECK_MSTATUS(addAttribute(aRmanLifCmds));
  CHECK_MSTATUS(addAttribute(aRmanMethods));
  CHECK_MSTATUS(addAttribute(aRmanIsOutput));
  CHECK_MSTATUS(addAttribute(aRmanAccept));
  CHECK_MSTATUS(addAttribute(aPreviewGamma));

  CHECK_MSTATUS(addAttribute(aPreviewPrimitive));
  CHECK_MSTATUS(addAttribute(aPreviewCustomPrimitive));
  CHECK_MSTATUS(addAttribute(aPreviewObjectSize));
  CHECK_MSTATUS(addAttribute(aPreviewShadingRate));
  CHECK_MSTATUS(addAttribute(aPreviewBackplane));
  CHECK_MSTATUS(addAttribute(aShaderSpace));
  CHECK_MSTATUS(addAttribute(aOutputInShadow));
  CHECK_MSTATUS(addAttribute(aRefreshPreview));

  CHECK_MSTATUS( addAttribute( aVolumeType ) );
  CHECK_MSTATUS( addAttribute( aVisiblePoints ) );

  CHECK_MSTATUS( addAttribute( aColor ) );
  CHECK_MSTATUS( addAttribute( aOpacity ) );
  CHECK_MSTATUS( addAttribute( aTransparency ) );

  CHECK_MSTATUS(addAttribute(aOutColor));

  return MS::kSuccess;
}

//MStatus liqVolumeNode::compute( const MPlug& plug, MDataBlock& block )
//{
//  return MS::kSuccess;
//}



