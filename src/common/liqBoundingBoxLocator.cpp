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
** Contributor(s): Alf Kraus
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/
#include <maya/MDagPath.h>
#include <maya/MColor.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MGlobal.h>
#include <maya/MMatrix.h>
#include <maya/M3dView.h>
#include <liqMayaNodeIds.h>
#include <liqBoundingBoxLocator.h>

const MTypeId liqBoundingBoxLocator::id( liqBoundingBoxLocatorId );

MObject liqBoundingBoxLocator::aDrawBox;

void liqBoundingBoxLocator::draw( M3dView & view, 
                                  const MDagPath & path,
                                  M3dView::DisplayStyle style, 
                                  M3dView::DisplayStatus status )
{ 
	MFnDagNode dagFn( thisMObject() );
	MFnDagNode transFn( dagFn.parent( 0 ) );

	MStatus stat;

	bool drawBox( 0 );
	MPlug plug = dagFn.findPlug( "drawBox", stat );
	if ( stat == MS::kSuccess ) plug.getValue( drawBox );
	if ( !drawBox ) return;

	MDoubleArray bb( 6 );
	MGlobal::executeCommand( MString( "exactWorldBoundingBox " ) + transFn.fullPathName(), bb );

	pts = shared_array< MPoint >( new MPoint[ 8 ] );

	pts[0].x = bb[0];
	pts[0].y = bb[4];
	pts[0].z = bb[2];

	pts[1].x = bb[0];
	pts[1].y = bb[4];
	pts[1].z = bb[5];

	pts[2].x = bb[3];
	pts[2].y = bb[4];
	pts[2].z = bb[5];

	pts[3].x = bb[3];
	pts[3].y = bb[4];
	pts[3].z = bb[2];

	pts[4].x = bb[0];
	pts[4].y = bb[1];
	pts[4].z = bb[2];

	pts[5].x = bb[0];
	pts[5].y = bb[1];
	pts[5].z = bb[5];

	pts[6].x = bb[3];
	pts[6].y = bb[1];
	pts[6].z = bb[5];

	pts[7].x = bb[3];
	pts[7].y = bb[1];
	pts[7].z = bb[2];

	MMatrix m( path.inclusiveMatrix() );

	for( unsigned i( 0 ); i < 8; i++ )
		pts[i] *= m.inverse();

	// draw box
	view.beginGL();
	view.setDrawColor( MColor( .57f, 0, .57f ) );
	glPushAttrib( GL_CURRENT_BIT );

	glBegin(GL_LINE_STRIP);
	glVertex3f( (float)pts[0].x, (float)pts[0].y, (float)pts[0].z );
	glVertex3f( (float)pts[1].x, (float)pts[1].y, (float)pts[1].z );
	glVertex3f( (float)pts[2].x, (float)pts[2].y, (float)pts[2].z );
	glVertex3f( (float)pts[3].x, (float)pts[3].y, (float)pts[3].z );
	glVertex3f( (float)pts[0].x, (float)pts[0].y, (float)pts[0].z );
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f( (float)pts[4].x, (float)pts[4].y, (float)pts[4].z );
	glVertex3f( (float)pts[5].x, (float)pts[5].y, (float)pts[5].z );
	glVertex3f( (float)pts[6].x, (float)pts[6].y, (float)pts[6].z );
	glVertex3f( (float)pts[7].x, (float)pts[7].y, (float)pts[7].z );
	glVertex3f( (float)pts[4].x, (float)pts[4].y, (float)pts[4].z );
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f( (float)pts[0].x, (float)pts[0].y, (float)pts[0].z );
	glVertex3f( (float)pts[4].x, (float)pts[4].y, (float)pts[4].z );
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f( (float)pts[1].x, (float)pts[1].y, (float)pts[1].z );
	glVertex3f( (float)pts[5].x, (float)pts[5].y, (float)pts[5].z );
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f( (float)pts[2].x, (float)pts[2].y, (float)pts[2].z );
	glVertex3f( (float)pts[6].x, (float)pts[6].y, (float)pts[6].z );
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f( (float)pts[3].x, (float)pts[3].y, (float)pts[3].z );
	glVertex3f( (float)pts[7].x, (float)pts[7].y, (float)pts[7].z );
	glEnd();
	view.endGL();
}

void *liqBoundingBoxLocator::creator()
{
	return new liqBoundingBoxLocator();
}

MStatus liqBoundingBoxLocator::initialize()
{ 
	MFnNumericAttribute numAttr;
	aDrawBox = numAttr.create( "drawBox", "d", MFnNumericData::kBoolean, true );
	addAttribute( aDrawBox );
	return MS::kSuccess;
}
