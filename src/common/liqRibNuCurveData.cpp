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
** Contributor(s): Berj Bannayan.
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
** Liquid Rib Nurbs Curve Data Source
** ______________________________________________________________________
*/

// Renderman Headers
extern "C" {
#include <ri.h>
}

// Maya headers
#include <maya/MDoubleArray.h>
#include <maya/MItCurveCV.h>
#include <maya/MPoint.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPlug.h>

// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibData.h>
#include <liqRibNuCurveData.h>

// Boost headers
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

using namespace boost;

extern int debugMode;


/** Create a RIB compatible representation of a Maya nurbs curve.
 */
liqRibNuCurveData::liqRibNuCurveData( MObject curve )
: nverts(),
  CVs(),
  NuCurveWidth()
{
  LIQDEBUGPRINTF( "-> creating nurbs curve\n" );
  MStatus status( MS::kSuccess );
  MFnNurbsCurve nurbs( curve, &status );
  assert( status == MS::kSuccess );

  // Extract the order and number of CVs in the surface keeping
  // in mind that UV order is switched between Renderman and Maya
  ncurves = 1;  //RiNuCurves can be passed many curves but right now it only passes one from maya at a time
  nverts = shared_array< RtInt >( new RtInt[ ncurves ] );
  nverts[0] = nurbs.numCVs() + 4;

  CVs   = shared_array< RtFloat >( new RtFloat[ nverts[ 0 ] * 3 ] );
  MItCurveCV cvs( curve, &status );
  RtFloat* cvPtr = CVs.get();

  // Double up start and end to simulate knot, MToor style (we should really be using RiNuCurves) - Paul
  MPoint pt = cvs.position(MSpace::kObject);

  *cvPtr = (RtFloat)pt.x; cvPtr++;
  *cvPtr = (RtFloat)pt.y; cvPtr++;
  *cvPtr = (RtFloat)pt.z; cvPtr++;

  *cvPtr = (RtFloat)pt.x; cvPtr++;
  *cvPtr = (RtFloat)pt.y; cvPtr++;
  *cvPtr = (RtFloat)pt.z; cvPtr++;

  while ( !cvs.isDone() ) 
	{
    pt = cvs.position(MSpace::kObject);
    *cvPtr = (RtFloat)pt.x; cvPtr++;
    *cvPtr = (RtFloat)pt.y; cvPtr++;
    *cvPtr = (RtFloat)pt.z; cvPtr++;
    cvs.next();
  }

  *cvPtr = (RtFloat)pt.x; cvPtr++;
  *cvPtr = (RtFloat)pt.y; cvPtr++;
  *cvPtr = (RtFloat)pt.z; cvPtr++;

  *cvPtr = (RtFloat)pt.x; cvPtr++;
  *cvPtr = (RtFloat)pt.y; cvPtr++;
  *cvPtr = (RtFloat)pt.z; cvPtr++;

  liqTokenPointer pointsPointerPair;

  pointsPointerPair.set( "P", rPoint, nverts[0] );
  pointsPointerPair.setDetailType( rVertex );
  pointsPointerPair.setTokenFloats( CVs ); 
	// Warning: CVs shares ownership with of its data with pointsPointerPair now!
  // Saves us from redundant copying as long as we know what we are doing
  tokenPointerArray.push_back( pointsPointerPair );

	// constant width or not
	float baseWidth( .1 ), tipWidth( .1 );
	bool constantWidth( false );
	MPlug pWidth = nurbs.findPlug( "liquidCurveBaseWidth", status );
	if ( status == MS::kSuccess ) pWidth.getValue( baseWidth );
	pWidth = nurbs.findPlug( "liquidCurveTipWidth", status );
	if ( status == MS::kSuccess ) pWidth.getValue( tipWidth );
	if ( tipWidth == baseWidth ) constantWidth = true;

	if ( constantWidth )
	{
		liqTokenPointer pConstWidthPointerPair;
		pConstWidthPointerPair.set( "constantwidth", rFloat );
		pConstWidthPointerPair.setDetailType( rConstant );
		pConstWidthPointerPair.setTokenFloat( 0, baseWidth );
		tokenPointerArray.push_back( pConstWidthPointerPair );
	}
	else
	{
		NuCurveWidth = shared_array< RtFloat >( new RtFloat[ nverts[ 0 ] - 2 ] );
		unsigned k(0);

		// easy way just linear - will have to be refined
		MItCurveCV itCurve( curve );
		NuCurveWidth[k++] = baseWidth;
		NuCurveWidth[k++] = baseWidth;
		for ( unsigned n( 3 ); n < nverts[0] - 3; n++ )
		{
			float difference = tipWidth - baseWidth;
			if ( difference < 0 ) difference *= -1;
			float basew ( baseWidth );
			if ( baseWidth > tipWidth ) NuCurveWidth[k++] = basew - ( n - 2 ) * difference / ( nverts[0] - 5 );
			else                				 NuCurveWidth[k++] = basew + ( n - 2 ) * difference / ( nverts[0] - 5 );
		}
		NuCurveWidth[k++] = tipWidth;
		NuCurveWidth[k++] = tipWidth;
		liqTokenPointer widthPointerPair;
		widthPointerPair.set( "width", rFloat, nverts[ 0 ] - 2 );
		widthPointerPair.setDetailType( rVarying );
		widthPointerPair.setTokenFloats( NuCurveWidth );
		tokenPointerArray.push_back( widthPointerPair );
	}
	addAdditionalSurfaceParameters( curve );
}
/**  Write the RIB for this curve.
 */
void liqRibNuCurveData::write()
{
  LIQDEBUGPRINTF( "-> writing nurbs curve\n" );

  unsigned numTokens( tokenPointerArray.size() );
  scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
  scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
  assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

  RiCurvesV( "cubic", ncurves, nverts.get(), "nonperiodic", numTokens, tokenArray.get(), pointerArray.get() );
}
/** Compare this curve to the other for the purpose of determining
 *  if it is animated.
 */
bool liqRibNuCurveData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing nurbs curve\n");
  if ( otherObj.type() != MRT_NuCurve ) return false;
  const liqRibNuCurveData & other = (liqRibNuCurveData&)otherObj;

  if ( ( nverts[0] != other.nverts[0] ) ) return false;

  // Check CVs
  unsigned last = nverts[0] * 3;
  for ( unsigned i = 0; i < last; ++i ) 
		if ( !equiv( CVs[i], other.CVs[i] ) ) return false;
  
  return true;
}
/** Return the geometry type.
 */
ObjectType liqRibNuCurveData::type() const
{
  LIQDEBUGPRINTF( "-> returning nurbs curve type\n" );
  return MRT_NuCurve;
}
