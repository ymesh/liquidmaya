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
** Contributor(s): Alf Kraus.
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
** Liquid RIB Request Node Source
** ______________________________________________________________________
*/

#include <liqRibRequestNode.h>
#include <liqMayaNodeIds.h>

#include <maya/MFnTypedAttribute.h>

MTypeId liqRibRequestNode::id( liqRibRequestId );

MObject liqRibRequestNode::aPreFrame;
MObject liqRibRequestNode::aPreWorld;
MObject liqRibRequestNode::aPostWorld;
MObject liqRibRequestNode::aPreGeom;

#define MAKE_INPUT( attr )		\
	CHECK_MSTATUS( attr.setKeyable( true ) ); 		\
	CHECK_MSTATUS( attr.setStorable( true ) );		\
	CHECK_MSTATUS( attr.setReadable( true ) ); 		\
	CHECK_MSTATUS( attr.setWritable( true ) );

void *liqRibRequestNode::creator()
{
	return new liqRibRequestNode();
}

MStatus liqRibRequestNode::initialize()
{
	MFnTypedAttribute tAttr;
	MStatus status;

	aPreFrame = tAttr.create( MString( "preFrame" ), MString( "preF" ), MFnData::kString, aPreFrame, &status );
	MAKE_INPUT(tAttr);

	aPreWorld = tAttr.create( MString( "preWorld" ), MString( "preW" ), MFnData::kString, aPreWorld, &status );
	MAKE_INPUT(tAttr);

	aPostWorld = tAttr.create( MString( "postWorld" ), MString( "postW" ), MFnData::kString, aPostWorld, &status );
	MAKE_INPUT(tAttr);

	aPreGeom = tAttr.create( MString( "preGeom" ), MString( "preG" ), MFnData::kString, aPreGeom, &status );
	MAKE_INPUT(tAttr);

	CHECK_MSTATUS( addAttribute( aPreFrame ) );
	CHECK_MSTATUS( addAttribute( aPreWorld ) );
	CHECK_MSTATUS( addAttribute( aPostWorld ) );
	CHECK_MSTATUS( addAttribute( aPreGeom ) );

	return MS::kSuccess;
}
