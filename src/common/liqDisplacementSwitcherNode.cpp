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

#include <liqDisplacementSwitcherNode.h>
#include <liqMayaNodeIds.h>

#include <maya/MFnNumericAttribute.h>

MTypeId liqDisplacementSwitcherNode::id( liqDisplacementSwitcherNodeId );

MObject liqDisplacementSwitcherNode::aDisplacement;
MObject liqDisplacementSwitcherNode::aOutColor;

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

liqDisplacementSwitcherNode::liqDisplacementSwitcherNode()
{
}


liqDisplacementSwitcherNode::~liqDisplacementSwitcherNode()
{
}


void* liqDisplacementSwitcherNode::creator()
{
	return new liqDisplacementSwitcherNode();
}


MStatus liqDisplacementSwitcherNode::initialize()
{
	MStatus status;
	status = liqSwitcherNode::initialize();
	if(status != MS::kSuccess)
	{
		return status;
	}
	MFnNumericAttribute nAttr;
	mAcceptedConnectionNodeType = nAttr.create("acceptedType", "act", MFnNumericData::kInt);
	nAttr.setKeyable(false);
	nAttr.setStorable(false);
	nAttr.setReadable(true);
	nAttr.setWritable(false);
	nAttr.setDefault(liqDisplacementNodeId);
	CHECK_MSTATUS( addAttribute( mAcceptedConnectionNodeType ) );

	// Create output attributes
	aDisplacement = nAttr.create("displacement", "d", MFnNumericData::kFloat, 0.0, &status);
	MAKE_OUTPUT(nAttr);
	aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

	CHECK_MSTATUS(addAttribute(aDisplacement));
	CHECK_MSTATUS(addAttribute(aOutColor));

	return MS::kSuccess;
}


MStatus liqDisplacementSwitcherNode::compute( const MPlug& plug, MDataBlock& block )
{
	if( (plug == aDisplacement) || (plug.parent() == aDisplacement) )
	{
		MDataHandle outDispHandle = block.outputValue( aDisplacement );
		outDispHandle.set( 0.0f );
		outDispHandle.setClean();
	}
	else
	{
		return MS::kUnknownParameter;
	}
	return MS::kSuccess;
}

