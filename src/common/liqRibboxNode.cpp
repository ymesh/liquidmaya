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
** Liquid RIB Box Node Source
** ______________________________________________________________________
*/

#include <liquid.h>
#include <liqRibboxNode.h>
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
MTypeId liqRibboxNode::id( liqRibboxNodeId );

// Attributes
MObject liqRibboxNode::aRmanShader;
MObject liqRibboxNode::aRmanShaderLong;
MObject liqRibboxNode::aRibbox;

MObject liqRibboxNode::aOutColor;

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

void liqRibboxNode::postConstructor( )
{
	setMPSafe(true);

  // init swatch
  if ( swatchInit != true ) {

    MObject obj = MPxNode::thisMObject();
    renderSwatch = new liqNodeSwatch( obj, obj, 64 );

    swatchInit = true;
  }

  MGlobal::executeCommandOnIdle( "liquidCheckGlobals()", false );
}

liqRibboxNode::liqRibboxNode()
{
  swatchInit = false;
  renderSwatch = NULL;
}

liqRibboxNode::~liqRibboxNode()
{
  if (renderSwatch != NULL) delete renderSwatch;
}

void* liqRibboxNode::creator()
{
    return new liqRibboxNode();
}

MStatus liqRibboxNode::initialize()
{
  MFnTypedAttribute   tAttr;
  MFnNumericAttribute nAttr;
  MStatus status;

  // Create input attributes
	aRmanShader = tAttr.create( MString("rmanShader"), MString("rms"), MFnData::kString, aRmanShader, &status );
	MAKE_INPUT(tAttr);

  aRmanShaderLong = tAttr.create( MString("rmanShaderLong"), MString("rml"), MFnData::kString, aRmanShaderLong, &status );
	MAKE_INPUT(tAttr);

  aRibbox = tAttr.create( MString("ribbox"), MString("rib"), MFnData::kString, aRibbox, &status );
	MAKE_INPUT(tAttr);

	// Create output attributes
  aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

  CHECK_MSTATUS(addAttribute(aRmanShader));
  CHECK_MSTATUS(addAttribute(aRmanShaderLong));
  CHECK_MSTATUS(addAttribute(aRibbox));

  CHECK_MSTATUS(addAttribute(aOutColor));

  return MS::kSuccess;
}

//MStatus liqRibboxNode::compute( const MPlug& plug, MDataBlock& block )
//{
//  return MS::kSuccess;
//}



