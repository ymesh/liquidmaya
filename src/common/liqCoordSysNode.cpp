//-
// ==========================================================================
// Copyright (C) 1995 - 2005 Alias Systems Corp. and/or its licensors.  All
// rights reserved.
//
// The coded instructions, statements, computer programs, and/or related
// material (collectively the "Data") in these files are provided by Alias
// Systems Corp. ("Alias") and/or its licensors for the exclusive use of the
// Customer (as defined in the Alias Software License Agreement that
// accompanies this Alias software). Such Customer has the right to use,
// modify, and incorporate the Data into other products and to distribute such
// products for use by end-users.
//
// THE DATA IS PROVIDED "AS IS".  ALIAS HEREBY DISCLAIMS ALL WARRANTIES
// RELATING TO THE DATA, INCLUDING, WITHOUT LIMITATION, ANY AND ALL EXPRESS OR
// IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE. IN NO EVENT SHALL ALIAS BE LIABLE FOR ANY DAMAGES
// WHATSOEVER, WHETHER DIRECT, INDIRECT, SPECIAL, OR PUNITIVE, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, OR IN EQUITY,
// ARISING OUT OF ACCESS TO, USE OF, OR RELIANCE UPON THE DATA.
// ==========================================================================
//+

#include <maya/MPxLocatorNode.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MVector.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnNumericAttribute.h>

#include <liqCoordSysNode.h>
#include <liqMayaNodeIds.h>
/*
#if defined(OSMac_MachO_)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
*/
#if defined(OSMac_)
#include <gl.h>
#include <glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <liqIOStream.h>

MTypeId liqCoordSysNode::id( liqCoordSysNodeId );
MObject liqCoordSysNode::aCoordType;
MObject liqCoordSysNode::aCoordColor;
MObject liqCoordSysNode::aCoordOpacity;

#define MAKE_INPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(true)); 		\
    CHECK_MSTATUS(attr.setStorable(true));		\
    CHECK_MSTATUS(attr.setReadable(true)); 		\
    CHECK_MSTATUS(attr.setWritable(true));

#define MAKE_NONKEYABLE_INPUT(attr)		\
    CHECK_MSTATUS(attr.setKeyable(false));  \
    CHECK_MSTATUS(attr.setStorable(true));  \
    CHECK_MSTATUS(attr.setReadable(true));  \
    CHECK_MSTATUS(attr.setWritable(true));



void* liqCoordSysNode::creator()
{
  return new liqCoordSysNode;
}


MStatus liqCoordSysNode::initialize()
{
  MFnEnumAttribute      eAttr;
  MFnNumericAttribute   numAttr;
  MStatus               status;

  aCoordType = eAttr.create( "type", "t", 0, &status );
  eAttr.addField( "Card",           0 );
  eAttr.addField( "Sphere",         1 );
  eAttr.addField( "Cylinder",       2 );
  eAttr.addField( "Cube",           3 );
  eAttr.addField( "Deep Card",      4 );
  eAttr.addField( "Clipping Plane", 5 );
  MAKE_INPUT(eAttr);
  CHECK_MSTATUS(eAttr.setConnectable(false));
  CHECK_MSTATUS( addAttribute( aCoordType ) );

  aCoordColor = numAttr.createColor( "coordColor", "cc", &status );
  MAKE_INPUT(numAttr);
  CHECK_MSTATUS( numAttr.setMin( 0.0, 0.0, 0.0 ) );
  CHECK_MSTATUS( numAttr.setMax( 1.0, 1.0, 1.0 ) );
  CHECK_MSTATUS( numAttr.setDefault( 0.0, 0.0, 0.5) );
  CHECK_MSTATUS( addAttribute( aCoordColor ) );

  aCoordOpacity = numAttr.create( "coordOpacity", "co", MFnNumericData::kFloat, 0.0, &status );
  MAKE_INPUT(numAttr);
  CHECK_MSTATUS( numAttr.setMin( 0.0 ) );
  CHECK_MSTATUS( numAttr.setMax( 1.0 ) );
  CHECK_MSTATUS( addAttribute( aCoordOpacity ) );

  return MS::kSuccess;
}



MStatus liqCoordSysNode::compute( const MPlug& /* plug */, MDataBlock& /* data */ )
{
  return MS::kUnknownParameter;
}

void liqCoordSysNode::draw(  M3dView & view, const MDagPath & /*path*/,
                             M3dView::DisplayStyle displaystyle,
                             M3dView::DisplayStatus displaystatus )
{
  // Get the type
  //
  MObject thisNode = thisMObject();
  MPlug typePlug( thisNode, aCoordType );
  CHECK_MSTATUS(typePlug.getValue( m_coordType ));
  MPlug colorPlug( thisNode, aCoordColor );
  CHECK_MSTATUS(colorPlug.child(0).getValue( m_coordColor.r ));
  CHECK_MSTATUS(colorPlug.child(1).getValue( m_coordColor.g ));
  CHECK_MSTATUS(colorPlug.child(2).getValue( m_coordColor.b ));
  MPlug opacityPlug( thisNode, aCoordOpacity );
  CHECK_MSTATUS(opacityPlug.getValue( m_coordColor.a ));

  view.beginGL();

  // Draw the arrows
  //

  glPushAttrib( GL_ALL_ATTRIB_BITS );

  glBegin( GL_LINES );

    glColor4f( 1.0f, 0.0f, 0.0f, 1.0f );
    glVertex3f(  0.00f,  0.00f,  0.00f );
    glVertex3f(  0.50f,  0.00f,  0.00f );
    glVertex3f(  0.50f,  0.00f,  0.00f );
    glVertex3f(  0.40f, -0.05f,  0.00f );
    glVertex3f(  0.40f, -0.05f,  0.00f );
    glVertex3f(  0.40f,  0.00f,  0.00f );

    glColor4f( 0.0f, 1.0f, 0.0f, 1.0f );
    glVertex3f(  0.00f,  0.00f,  0.00f  );
    glVertex3f(  0.00f,  0.50f,  0.00f  );
    glVertex3f(  0.00f,  0.50f,  0.00f  );
    glVertex3f(  0.05f,  0.40f,  0.00f  );
    glVertex3f(  0.05f,  0.40f,  0.00f  );
    glVertex3f(  0.00f,  0.40f,  0.00f  );

    glColor4f( 0.0f, 0.0f, 1.0f, 1.0f );
    glVertex3f(  0.00f,  0.00f,  0.00f   );
    glVertex3f(  0.00f,  0.00f,  0.50f   );
    glVertex3f(  0.00f,  0.00f,  0.50f   );
    glVertex3f(  0.00f, -0.05f,  0.40f   );
    glVertex3f(  0.00f, -0.05f,  0.40f   );
    glVertex3f(  0.00f,  0.00f,  0.40f   );

  glEnd();

  glPopAttrib();


  // draw the warious primitives
  //

  switch ( m_coordType ) 
  {
    case 0:
      // PLANE
      glPushAttrib( GL_ALL_ATTRIB_BITS );
      if ( displaystatus == M3dView::kDormant ) glColor4f( m_coordColor.r, m_coordColor.g, m_coordColor.b, 1.0f );
      glBegin( GL_LINES );
        glVertex3f( -0.50f,  0.50f,  0.00f );
        glVertex3f(  0.50f,  0.50f,  0.00f );
        glVertex3f(  0.50f,  0.50f,  0.00f );
        glVertex3f(  0.50f, -0.50f,  0.00f );
        glVertex3f(  0.50f, -0.50f,  0.00f );
        glVertex3f( -0.50f, -0.50f,  0.00f );
        glVertex3f( -0.50f, -0.50f,  0.00f );
        glVertex3f( -0.50f,  0.50f,  0.00f );
      glEnd();
      glPopAttrib();
      break;

    case 1:
      // SPHERE
      {
        glPushAttrib( GL_ALL_ATTRIB_BITS );
        if ( displaystatus == M3dView::kDormant ) glColor4f( m_coordColor.r, m_coordColor.g, m_coordColor.b, 1.0f );
        glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
        GLUquadricObj* pQuadric = gluNewQuadric();
        gluQuadricDrawStyle( pQuadric, GLU_LINE);
        gluSphere( pQuadric, 0.5f , 10, 6);
        gluDeleteQuadric(pQuadric);
        glPopAttrib();
      }
      break;

    case 2:
      // CYLINDER
      {
        glPushAttrib( GL_ALL_ATTRIB_BITS );
        if ( displaystatus == M3dView::kDormant ) glColor4f( m_coordColor.r, m_coordColor.g, m_coordColor.b, 1.0f );
        glTranslatef( 0.0f, -0.5f, 0.0f );
        glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
        GLUquadricObj* pQuadric = gluNewQuadric();
        gluQuadricDrawStyle( pQuadric, GLU_LINE);
        gluCylinder( pQuadric, 0.5f , 0.5f, 1.0f, 10, 2);
        gluDeleteQuadric(pQuadric);
        glPopAttrib();
      }
      break;

    case 3:
      // CUBE
      glPushAttrib( GL_ALL_ATTRIB_BITS );
      if ( displaystatus == M3dView::kDormant ) glColor4f( m_coordColor.r, m_coordColor.g, m_coordColor.b, 1.0f );
      glBegin( GL_LINES );

        glVertex3f( -0.50f,  0.50f, -0.50f );
        glVertex3f(  0.50f,  0.50f, -0.50f );
        glVertex3f(  0.50f,  0.50f, -0.50f );
        glVertex3f(  0.50f,  0.50f,  0.50f );
        glVertex3f(  0.50f,  0.50f,  0.50f );
        glVertex3f( -0.50f,  0.50f,  0.50f );
        glVertex3f( -0.50f,  0.50f,  0.50f );
        glVertex3f( -0.50f,  0.50f, -0.50f );

        glVertex3f( -0.50f, -0.50f, -0.50f );
        glVertex3f(  0.50f, -0.50f, -0.50f );
        glVertex3f(  0.50f, -0.50f, -0.50f );
        glVertex3f(  0.50f, -0.50f,  0.50f );
        glVertex3f(  0.50f, -0.50f,  0.50f );
        glVertex3f( -0.50f, -0.50f,  0.50f );
        glVertex3f( -0.50f, -0.50f,  0.50f );
        glVertex3f( -0.50f, -0.50f, -0.50f );

        glVertex3f( -0.50f,  0.50f, -0.50f );
        glVertex3f( -0.50f, -0.50f, -0.50f );
        glVertex3f(  0.50f,  0.50f, -0.50f );
        glVertex3f(  0.50f, -0.50f, -0.50f );
        glVertex3f(  0.50f,  0.50f,  0.50f );
        glVertex3f(  0.50f, -0.50f,  0.50f );
        glVertex3f( -0.50f,  0.50f,  0.50f );
        glVertex3f( -0.50f, -0.50f,  0.50f );

      glEnd();
      glPopAttrib();
      break;

    case 4:
      // DEEP PLANE
      glPushAttrib( GL_ALL_ATTRIB_BITS );
      if ( displaystatus == M3dView::kDormant ) glColor4f( m_coordColor.r, m_coordColor.g, m_coordColor.b, 1.0f );
      glBegin( GL_LINES );
        glVertex3f( -0.50f,  0.50f,  0.00f );
        glVertex3f(  0.50f,  0.50f,  0.00f );
        glVertex3f(  0.50f,  0.50f,  0.00f );
        glVertex3f(  0.50f, -0.50f,  0.00f );
        glVertex3f(  0.50f, -0.50f,  0.00f );
        glVertex3f( -0.50f, -0.50f,  0.00f );
        glVertex3f( -0.50f, -0.50f,  0.00f );
        glVertex3f( -0.50f,  0.50f,  0.00f );
      glEnd();
      glEnable( GL_LINE_STIPPLE);
      glLineStipple(1,0xff);
      glBegin( GL_LINES );
        glVertex3f( -0.50f,  0.50f,  1.00f );
        glVertex3f(  0.50f,  0.50f,  1.00f );
        glVertex3f(  0.50f,  0.50f,  1.00f );
        glVertex3f(  0.50f, -0.50f,  1.00f );
        glVertex3f(  0.50f, -0.50f,  1.00f );
        glVertex3f( -0.50f, -0.50f,  1.00f );
        glVertex3f( -0.50f, -0.50f,  1.00f );
        glVertex3f( -0.50f,  0.50f,  1.00f );
      glEnd();
      glPopAttrib();
      break;

    case 5:
      // CLIPPING PLANE
      glPushAttrib( GL_ALL_ATTRIB_BITS );

      glEnable( GL_LINE_STIPPLE);
      glLineStipple(1,0xff);
      if ( displaystatus == M3dView::kDormant ) glColor4f( m_coordColor.r, m_coordColor.g, m_coordColor.b, 1.0f );

      glBegin( GL_LINES );

        glVertex3f( -1.0f,  1.0f,  0.0f );
        glVertex3f(  1.0f,  1.0f,  0.0f );
        glVertex3f(  1.0f,  1.0f,  0.0f );
        glVertex3f(  1.0f, -1.0f,  0.0f );
        glVertex3f(  1.0f, -1.0f,  0.0f );
        glVertex3f( -1.0f, -1.0f,  0.0f );
        glVertex3f( -1.0f, -1.0f,  0.0f );
        glVertex3f( -1.0f,  1.0f,  0.0f );

        glVertex3f( -1.0f,  1.0f,  0.0f );
        glVertex3f(  1.0f, -1.0f,  0.0f );
        glVertex3f(  1.0f,  1.0f,  0.0f );
        glVertex3f( -1.0f, -1.0f,  0.0f );

      glEnd();

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_ALWAYS ,0.1);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);
      glDepthMask( GL_FALSE );
      glColor4f( m_coordColor.r, m_coordColor.g, m_coordColor.b, m_coordColor.a );
      glBegin( GL_QUADS );
        glVertex3f( -1.0f , 1.0f, 0.0f );
        glVertex3f(  1.0f , 1.0f, 0.0f );
        glVertex3f(  1.0f ,-1.0f, 0.0f );
        glVertex3f( -1.0f ,-1.0f, 0.0f );
      glEnd();

      glPopAttrib();
      break;
  }
  view.endGL();
}


bool liqCoordSysNode::isBounded() const
{
  return true;
}


MBoundingBox liqCoordSysNode::boundingBox() const
{
  MPoint corner1;
  MPoint corner2;

  switch ( m_coordType ) 
  {
    case 0:
      corner1 = MPoint( -0.5, -0.5,  0.0 );
      corner2 = MPoint(  0.5,  0.5,  0.5 );
      break;

    case 4:
      corner1 = MPoint( -0.5, -0.5,  0.0 );
      corner2 = MPoint(  0.5,  0.5,  1.0 );
      break;

    case 5:
      corner1 = MPoint( -1.0, -1.0,  0.0 );
      corner2 = MPoint(  1.0,  1.0,  0.5 );
      break;

    default:
      corner1 = MPoint( -0.5, -0.5, -0.5 );
      corner2 = MPoint(  0.5,  0.5,  0.5 );
      break;
  }

  return MBoundingBox( corner1, corner2 );
}



