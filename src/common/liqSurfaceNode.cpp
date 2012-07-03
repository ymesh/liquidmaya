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
** Liquid Surface Shader Node Source
** ______________________________________________________________________
*/

#include <liquid.h>
#include <liqSurfaceNode.h>
#include <liqNodeSwatch.h>
#include <liqMayaNodeIds.h>

#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MFnStringData.h>
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
#include <maya/MFnLightDataAttribute.h>


#include <liqIOStream.h>

// static data
MTypeId liqSurfaceNode::id( liqSurfaceNodeId );

// Attributes
MObject liqSurfaceNode::aRmanShader;
MObject liqSurfaceNode::aRmanShaderLong;
MObject liqSurfaceNode::aRmanShaderLif;
MObject liqSurfaceNode::aRmanParams;
MObject liqSurfaceNode::aRmanDetails;
MObject liqSurfaceNode::aRmanTypes;
MObject liqSurfaceNode::aRmanDefaults;
MObject liqSurfaceNode::aRmanArraySizes;
MObject liqSurfaceNode::aRmanLifCmds;
MObject liqSurfaceNode::aRmanMethods;
MObject liqSurfaceNode::aRmanIsOutput;  
MObject liqSurfaceNode::aRmanAccept;

MObject liqSurfaceNode::aPreviewPrimitive;
MObject liqSurfaceNode::aPreviewCustomPrimitive;
MObject liqSurfaceNode::aPreviewCustomBackplane;
MObject liqSurfaceNode::aPreviewCustomLightRig;
MObject liqSurfaceNode::aColor;
MObject liqSurfaceNode::aTransparency;
MObject liqSurfaceNode::aOpacity;
MObject liqSurfaceNode::aShaderSpace;
MObject liqSurfaceNode::aDisplacementBound;
MObject liqSurfaceNode::aDisplacementBoundSpace;
MObject liqSurfaceNode::aOutputInShadow; 

MObject liqSurfaceNode::aVisiblePoints;

MObject liqSurfaceNode::aResolution;
MObject liqSurfaceNode::aRefreshPreview;
MObject liqSurfaceNode::aPreviewObjectSize;
MObject liqSurfaceNode::aPreviewShadingRate;
MObject liqSurfaceNode::aPreviewBackplane;
MObject liqSurfaceNode::aPreviewIntensity;
MObject liqSurfaceNode::aGLPreviewTexture;
//MObject liqSurfaceNode::aCi;
//MObject liqSurfaceNode::aOi;

MObject liqSurfaceNode::aMayaIgnoreLights;
MObject liqSurfaceNode::aMayaKa;
MObject liqSurfaceNode::aMayaKd;
MObject liqSurfaceNode::aNormalCameraX;
MObject liqSurfaceNode::aNormalCameraY;
MObject liqSurfaceNode::aNormalCameraZ;
MObject liqSurfaceNode::aNormalCamera;
MObject liqSurfaceNode::aLightDirectionX;
MObject liqSurfaceNode::aLightDirectionY;
MObject liqSurfaceNode::aLightDirectionZ;
MObject liqSurfaceNode::aLightDirection;
MObject liqSurfaceNode::aLightIntensityR;
MObject liqSurfaceNode::aLightIntensityG;
MObject liqSurfaceNode::aLightIntensityB;
MObject liqSurfaceNode::aLightIntensity;
MObject liqSurfaceNode::aLightAmbient;
MObject liqSurfaceNode::aLightDiffuse;
MObject liqSurfaceNode::aLightSpecular;
MObject liqSurfaceNode::aLightShadowFraction;
MObject liqSurfaceNode::aPreShadowIntensity;
MObject liqSurfaceNode::aLightBlindData;
MObject liqSurfaceNode::aLightData;

MObject liqSurfaceNode::aEvaluateAtEveryFrame;
MObject liqSurfaceNode::aPreviewGamma;
  
MObject liqSurfaceNode::aOutColor;
MObject liqSurfaceNode::aOutTransparency;

#define MAKE_INPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(true ) ); 		\
    CHECK_MSTATUS(attr.setStorable(true ) );		\
    CHECK_MSTATUS(attr.setReadable(true ) ); 		\
    CHECK_MSTATUS(attr.setWritable(true ) );

#define MAKE_NONKEYABLE_INPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(false ) ); 		\
    CHECK_MSTATUS(attr.setStorable(true ) );		\
    CHECK_MSTATUS(attr.setReadable(true ) ); 		\
    CHECK_MSTATUS(attr.setWritable(true ) );

#define MAKE_OUTPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(false ) ); 		\
    CHECK_MSTATUS(attr.setStorable(false ) );		\
    CHECK_MSTATUS(attr.setReadable(true ) ); 		\
    CHECK_MSTATUS(attr.setWritable(false ) );

void liqSurfaceNode::postConstructor( )
{
  setMPSafe(true);

  // init swatch
  if ( swatchInit != true ) {
    MObject obj = MPxNode::thisMObject();
    renderSwatch = new liqNodeSwatch( obj, obj, 128 );
    swatchInit = true;
  }

  MGlobal::executeCommandOnIdle( "liquidCheckGlobals()", false );
}

liqSurfaceNode::liqSurfaceNode()
{
  swatchInit = false;
  renderSwatch = NULL;
}

liqSurfaceNode::~liqSurfaceNode()
{
  if (renderSwatch != NULL) delete renderSwatch;
}

void* liqSurfaceNode::creator()
{
    return new liqSurfaceNode();
}

MStatus liqSurfaceNode::initialize()
{
  MFnTypedAttribute   tAttr;
  MFnStringData       tDefault;
  MFnNumericAttribute nAttr;
  MFnEnumAttribute    eAttr;
  MFnLightDataAttribute lAttr;
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
  CHECK_MSTATUS(eAttr.setConnectable(false ) );

  aPreviewCustomPrimitive = tAttr.create(  MString("previewCustomPrimitive"),  MString("pcp"), MFnData::kString, aPreviewCustomPrimitive, &status );
  MAKE_INPUT(tAttr);

  aPreviewCustomBackplane = tAttr.create(  MString("previewCustomBackplane"),  MString("pcb"), MFnData::kString, aPreviewCustomBackplane, &status );
  MAKE_INPUT(tAttr);

  aPreviewCustomLightRig = tAttr.create(  MString("previewCustomLights"),  MString("pcl"), MFnData::kString, aPreviewCustomLightRig, &status );
  MAKE_INPUT(tAttr);

  aPreviewObjectSize = nAttr.create("previewObjectSize", "pos", MFnNumericData::kDouble, 1.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false ) );

  aPreviewShadingRate = nAttr.create("previewShadingRate", "psr", MFnNumericData::kDouble, 1.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false ) );

  aPreviewBackplane = nAttr.create("previewBackplane", "pbp", MFnNumericData::kBoolean, true, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false ) );

  aPreviewIntensity = nAttr.create("previewIntensity", "pi", MFnNumericData::kDouble, 1.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setConnectable(false ) );
  
  aGLPreviewTexture = nAttr.createColor("GLPreviewTexture", "gpt");
  nAttr.setDefault( -1.0, -1.0, -1.0 );
  nAttr.setDisconnectBehavior( MFnAttribute::kReset );
  MAKE_INPUT(nAttr);

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

  aShaderSpace = tAttr.create( MString("shaderSpace"), MString("ssp"), MFnData::kString, aShaderSpace, &status );
  MAKE_INPUT(tAttr);

  aDisplacementBound = nAttr.create("displacementBound", "db", MFnNumericData::kDouble, 0.0, &status);
  MAKE_INPUT(nAttr);

  MObject defaultSpaceObj = tDefault.create( MString("shader"), &status);
  aDisplacementBoundSpace = tAttr.create( MString("displacementBoundSpace"), MString("dbs"), MFnData::kString, defaultSpaceObj, &status );
  MAKE_INPUT(tAttr);

  aOutputInShadow = nAttr.create("outputInShadow", "ois",  MFnNumericData::kBoolean, 0.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);

  aVisiblePoints = nAttr.create( "useVisiblePoints", "uvp", MFnNumericData::kBoolean, false, &status );
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS( nAttr.setConnectable(false ) );

	// resolution attribute for maya's hardware renderer
  aResolution = nAttr.create("resolution", "res",  MFnNumericData::kInt, 32, &status);
  CHECK_MSTATUS(nAttr.setStorable( true ) );
  CHECK_MSTATUS(nAttr.setReadable( true ) );
  CHECK_MSTATUS(nAttr.setWritable( true ) );
  CHECK_MSTATUS(nAttr.setHidden( true ) );
  
  

  // refreshPreview must be true to allow refresh
  aRefreshPreview = nAttr.create("refreshPreview", "rfp",  MFnNumericData::kBoolean, 0.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  CHECK_MSTATUS(nAttr.setHidden(true ) );

  // dynamic shader attr
  //aCi = nAttr.createColor("Ci", "ci");
  //nAttr.setDefault( 1.0, 1.0, 1.0 );
  //MAKE_INPUT(nAttr);

  //aOi = nAttr.createColor("Oi", "oi");
  //nAttr.setDefault( 1.0, 1.0, 1.0 );
  //MAKE_INPUT(nAttr);


  // create attributes for maya renderer

  // lambertian control
  aMayaIgnoreLights = nAttr.create("mayaIgnoreLights", "mil",  MFnNumericData::kBoolean, 0.0, &status);
  MAKE_INPUT(nAttr);
  aMayaKa = nAttr.create("mayaKa", "mka", MFnNumericData::kFloat, 0.2, &status);
  MAKE_INPUT(nAttr);
  aMayaKd = nAttr.create("mayaKd", "mkd", MFnNumericData::kFloat, 0.8, &status);
  MAKE_INPUT(nAttr);

  // Camera Normals
  aNormalCameraX = nAttr.create( "normalCameraX", "nx", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aNormalCameraY = nAttr.create( "normalCameraY", "ny", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aNormalCameraZ = nAttr.create( "normalCameraZ", "nz", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aNormalCamera = nAttr.create( "normalCamera","n", aNormalCameraX, aNormalCameraY, aNormalCameraZ, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f, 1.0f, 1.0f ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );

  // Light Direction
  aLightDirectionX = nAttr.create( "lightDirectionX", "ldx", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aLightDirectionY = nAttr.create( "lightDirectionY", "ldy", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aLightDirectionZ = nAttr.create( "lightDirectionZ", "ldz", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aLightDirection = nAttr.create( "lightDirection", "ld", aLightDirectionX, aLightDirectionY, aLightDirectionZ, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f, 1.0f, 1.0f ) );

  // Light Intensity
  aLightIntensityR = nAttr.create( "lightIntensityR", "lir", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aLightIntensityG = nAttr.create( "lightIntensityG", "lig", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aLightIntensityB = nAttr.create( "lightIntensityB", "lib", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aLightIntensity = nAttr.create( "lightIntensity", "li", aLightIntensityR, aLightIntensityG, aLightIntensityB, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f, 1.0f, 1.0f ) );

  // Light
  aLightAmbient = nAttr.create( "lightAmbient", "la", MFnNumericData::kBoolean, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( true ) );
  aLightDiffuse = nAttr.create( "lightDiffuse", "ldf", MFnNumericData::kBoolean, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( true ) );
  aLightSpecular = nAttr.create( "lightSpecular", "ls", MFnNumericData::kBoolean, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( false ) );
  aLightShadowFraction = nAttr.create( "lightShadowFraction", "lsf", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setReadable( true ) );
  CHECK_MSTATUS( nAttr.setWritable( true ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  aPreShadowIntensity = nAttr.create( "preShadowIntensity", "psi", MFnNumericData::kFloat, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );

  #if MAYA_API_VERSION >= 800
  aLightBlindData = nAttr.createAddr( "lightBlindData", "lbld", 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( (void*) 0 ) );
  #else
  aLightBlindData = nAttr.create( "lightBlindData", "lbld", MFnNumericData::kLong, 0, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( false ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( false ) );
  CHECK_MSTATUS( nAttr.setDefault( 0 ) );
  #endif
  aLightData = lAttr.create( "lightDataArray", "ltd", aLightDirection,
                              aLightIntensity, aLightAmbient, aLightDiffuse, aLightSpecular,
                              aLightShadowFraction, aPreShadowIntensity, aLightBlindData,
                              &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( lAttr.setArray( true ) );
  CHECK_MSTATUS( lAttr.setStorable( false ) );
  CHECK_MSTATUS( lAttr.setHidden( true ) );
  CHECK_MSTATUS( lAttr.setDefault( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, 1.0f, 1.0f, 0 ) );

  aEvaluateAtEveryFrame = nAttr.create("evaluateAtEveryFrame", "def",  MFnNumericData::kBoolean, 0.0, &status);
  MAKE_NONKEYABLE_INPUT(nAttr);
  
  aPreviewGamma = nAttr.create( "previewGamma", "pg", MFnNumericData::kFloat, 1, &status );
  CHECK_MSTATUS( status );
  CHECK_MSTATUS( nAttr.setStorable( true ) );
  CHECK_MSTATUS( nAttr.setHidden( true ) );
  CHECK_MSTATUS( nAttr.setReadable( true ) );
  CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
  // Create output attributes
  aOutColor = nAttr.createColor("outColor", "oc");
  MAKE_OUTPUT(nAttr);
  aOutTransparency = nAttr.createColor("outTransparency", "ot");
  MAKE_OUTPUT(nAttr);

  CHECK_MSTATUS( addAttribute( aRmanShader ) );
  CHECK_MSTATUS( addAttribute( aRmanShaderLong ) );
  CHECK_MSTATUS( addAttribute( aRmanShaderLif ) );
  CHECK_MSTATUS( addAttribute( aRmanParams ) );
  CHECK_MSTATUS( addAttribute( aRmanDetails ) );
  CHECK_MSTATUS( addAttribute( aRmanTypes ) );
  CHECK_MSTATUS( addAttribute( aRmanDefaults ) );
  CHECK_MSTATUS( addAttribute( aRmanArraySizes ) );
  CHECK_MSTATUS( addAttribute( aRmanLifCmds ) );
  CHECK_MSTATUS( addAttribute(aRmanMethods) );
  CHECK_MSTATUS( addAttribute(aRmanIsOutput) );
  CHECK_MSTATUS( addAttribute( aRmanAccept) );

  CHECK_MSTATUS( addAttribute( aPreviewPrimitive ) );
  CHECK_MSTATUS( addAttribute( aPreviewCustomPrimitive ) );
  CHECK_MSTATUS( addAttribute( aPreviewCustomBackplane ) );
  CHECK_MSTATUS( addAttribute( aPreviewCustomLightRig ) );
  CHECK_MSTATUS( addAttribute( aPreviewObjectSize ) );
  CHECK_MSTATUS( addAttribute( aPreviewShadingRate ) );
  CHECK_MSTATUS( addAttribute( aPreviewBackplane ) );
  CHECK_MSTATUS( addAttribute( aPreviewIntensity ) );
  CHECK_MSTATUS(addAttribute(aGLPreviewTexture));
  //CHECK_MSTATUS( addAttribute( aCi ) );
  //CHECK_MSTATUS( addAttribute( aOi ) );

  CHECK_MSTATUS( addAttribute( aColor ) );
  CHECK_MSTATUS( addAttribute( aTransparency ) );
  CHECK_MSTATUS( addAttribute( aOpacity ) );
  CHECK_MSTATUS( addAttribute( aShaderSpace ) );
  CHECK_MSTATUS( addAttribute( aDisplacementBound ) );
  CHECK_MSTATUS( addAttribute( aDisplacementBoundSpace ) );
  CHECK_MSTATUS( addAttribute( aOutputInShadow ) );
  CHECK_MSTATUS( addAttribute( aVisiblePoints ) );
  
  CHECK_MSTATUS( addAttribute( aResolution ) );
  CHECK_MSTATUS( addAttribute( aRefreshPreview ) );
  CHECK_MSTATUS( addAttribute( aMayaIgnoreLights ) );
  CHECK_MSTATUS( addAttribute( aMayaKa ) );
  CHECK_MSTATUS( addAttribute( aMayaKd ) );
  CHECK_MSTATUS( addAttribute( aNormalCamera ) );
  CHECK_MSTATUS( addAttribute( aLightData ) );
  
  CHECK_MSTATUS( addAttribute( aEvaluateAtEveryFrame ) );
  CHECK_MSTATUS( addAttribute( aPreviewGamma ) );
  
  CHECK_MSTATUS( addAttribute( aOutColor ) );
  CHECK_MSTATUS( addAttribute( aOutTransparency ) );
  CHECK_MSTATUS(attributeAffects( aGLPreviewTexture,    aOutColor ));
  CHECK_MSTATUS(attributeAffects( aColor,               aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aOpacity,             aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aMayaIgnoreLights,    aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aMayaKa,              aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aMayaKd,              aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightIntensityR,     aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightIntensityB,     aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightIntensityG,     aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightIntensity,      aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aNormalCameraX,       aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aNormalCameraY,       aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aNormalCameraZ,       aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aNormalCamera,        aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightDirectionX,     aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightDirectionY,     aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightDirectionZ,     aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightDirection,      aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightAmbient,        aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightSpecular,       aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightDiffuse,        aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightShadowFraction, aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aPreShadowIntensity,  aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightBlindData,      aOutColor ) );
  CHECK_MSTATUS(attributeAffects( aLightData,           aOutColor ) );

  CHECK_MSTATUS(attributeAffects( aOpacity,           aOutTransparency ) );
  return MS::kSuccess;
}

MStatus liqSurfaceNode::compute( const MPlug& plug, MDataBlock& block )
{
  // outColor or individual R, G, B channel
  if( 	(plug == aOutColor) || (plug.parent() == aOutColor) ||
  		( plug == aOutTransparency ) || (plug.parent() == aOutTransparency) ) 
  { 

    //cout <<"compute... "<<endl;

    // init shader
    MStatus status;
    MFloatVector& cColor  = block.inputValue(aColor).asFloatVector();
    MFloatVector& cTrans  = block.inputValue(aOpacity).asFloatVector();
    
    MFloatVector& ctex    = block.inputValue(aGLPreviewTexture).asFloatVector();

    // exploit maya's free openGL preview
    if ( ctex != MFloatVector( -1.0, -1.0, -1.0 ) ) cColor = ctex;
    //else theColor = cColor;
    
    MFloatVector resultColor( 0.0, 0.0, 0.0 );
    MFloatVector resultTrans( cTrans );



    // lambert calc -------------------
    bool&  ignoreLights = block.inputValue( aMayaIgnoreLights, &status ).asBool();
    float& Ka = block.inputValue( aMayaKa, &status ).asFloat();
    float& Kd = block.inputValue( aMayaKd, &status ).asFloat();

    // get surface normal
    MFloatVector& surfaceNormal = block.inputValue( aNormalCamera, &status ).asFloatVector();
    CHECK_MSTATUS( status );

    if ( ignoreLights ) 
		{
      MFloatVector cam( 0.0, 0.0, 1.0 );
      float cosln = cam * surfaceNormal;
      if ( cosln > 0.0f ) 
			{
        float diff = cosln * cosln * Kd + Ka;
        resultColor = diff * cColor;
      }
    } 
		else 
		{
      // Get light list
      MArrayDataHandle lightData = block.inputArrayValue( aLightData, &status );
      CHECK_MSTATUS( status );
      int numLights = lightData.elementCount( &status );
      CHECK_MSTATUS( status );

      // Iterate through light list and get ambient/diffuse values
      for ( int count=1; count <= numLights; count++ )
      {
        // Get the current light out of the array
        MDataHandle currentLight = lightData.inputValue( &status );
        CHECK_MSTATUS( status );

        // Get the intensity of that light
        MFloatVector& lightIntensity = currentLight.child( aLightIntensity ).asFloatVector();

        // Find ambient component
        if ( currentLight.child( aLightAmbient ).asBool() ) resultColor += lightIntensity;

        // Find diffuse component
        if ( currentLight.child( aLightDiffuse ).asBool() ) 
				{
          MFloatVector& lightDirection = currentLight.child( aLightDirection ).asFloatVector();
          float cosln = lightDirection * surfaceNormal;
          if ( cosln > 0.0f )  resultColor += lightIntensity * cosln * Kd ;
        }
        // Advance to the next light.
        if ( count < numLights ) 
				{
          status = lightData.next();
          CHECK_MSTATUS( status );
        }
      }
      resultColor[0] *= cColor[0];
      resultColor[1] *= cColor[1];
      resultColor[2] *= cColor[2];
    }

    resultTrans[0] = ( 1.0 - resultTrans[0] );
    resultTrans[1] = ( 1.0 - resultTrans[1] );
    resultTrans[2] = ( 1.0 - resultTrans[2] );

    // set ouput color attribute
    MDataHandle outColorHandle = block.outputValue( aOutColor );
    MFloatVector& outColor = outColorHandle.asFloatVector();
    outColor = resultColor;
    outColorHandle.setClean();

    MDataHandle outTransHandle = block.outputValue( aOutTransparency );
    MFloatVector& outTrans = outTransHandle.asFloatVector();
    outTrans = resultTrans;
    outTransHandle.setClean();

  } else return MS::kUnknownParameter;


  return MS::kSuccess;
}




