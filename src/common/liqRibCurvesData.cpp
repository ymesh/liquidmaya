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
** Contributor(s): Berj Bannayan, Alf Kraus
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
#include <maya/MObjectArray.h>
#include <maya/MSelectionList.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPlug.h>
#include <maya/MGlobal.h>

// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibData.h>
#include <liqRibCurvesData.h>

// Boost headers
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

using namespace boost;
extern int debugMode;
extern bool liqglo_renderAllCurves;

// Create a RIB compatible representation of a Maya nurbs curve group.

liqRibCurvesData::liqRibCurvesData( MObject curveGroup )
: nverts(),
  CVs(),
  NuCurveWidth()
{
	LIQDEBUGPRINTF( "-> creating nurbs curve group\n" );

	MStatus status( MS::kSuccess );
	MFnDagNode fnDag( curveGroup, &status );
	MFnDagNode fnTrans( fnDag.parent( 0 ) );

	MSelectionList groupList; 
	groupList.add( fnTrans.partialPathName() );
	MDagPath groupPath;
	groupList.getDagPath( 0, groupPath );
	MMatrix m( groupPath.inclusiveMatrix() );

	MStringArray curveList;
	MObjectArray curveObj;
	MDagPathArray curveDag;

	// using the transforms not the nurbsCurves so instances are supported
	MGlobal::executeCommand( MString( "ls -dag -type transform " ) + fnTrans.partialPathName(), curveList );
	for ( unsigned i( 0 ) ; i < curveList.length() ; i++ )
	{
		MSelectionList list; 
		list.add( curveList[i] );
		MObject curveNode;
		MDagPath path;
		list.getDependNode( 0, curveNode );
		list.getDagPath( 0, path );
		MFnDagNode fnCurve( curveNode );
		MObject shape( fnCurve.child( 0 ) );
		if ( shape.hasFn( MFn::kNurbsCurve ) )
		{
			curveObj.append( curveNode );
			curveDag.append( path );
		}
	}

	if ( liqglo_renderAllCurves ) ncurves = curveObj.length();
	else ncurves = 0;  
	  
	if ( !ncurves ) return;

	nverts = shared_array< RtInt >( new RtInt[ ncurves ] );

	unsigned cvcount( 0 );

	unsigned long curvePts(0);
	for ( unsigned i( 0 ); i < ncurves; i++ )
	{
		MFnDagNode fnDag( curveObj[i] );
		MFnNurbsCurve fnCurve( fnDag.child( 0 ) );
		nverts[i] = fnCurve.numCVs() + 4;
		cvcount += nverts[i];
	}

	CVs = shared_array< RtFloat >( new RtFloat[ cvcount * 3 ] );

	unsigned k( 0 );
	for ( unsigned i( 0 ); i < ncurves; i++ )
	{
		MFnDagNode fnCurve( curveObj[i] );
		MMatrix mCurve( curveDag[i].inclusiveMatrix() );
		mCurve -= m;
		mCurve += MMatrix::identity;

		MObject oCurveChild( fnCurve.child( 0 ) );
		MItCurveCV curveIt( oCurveChild );
		MPoint pt = curveIt.position();
		pt *= mCurve;
		CVs[k++] = (float)pt.x;
		CVs[k++] = (float)pt.y;
		CVs[k++] = (float)pt.z;
		CVs[k++] = (float)pt.x;
		CVs[k++] = (float)pt.y;
		CVs[k++] = (float)pt.z;
		for ( curveIt.reset(); !curveIt.isDone(); curveIt.next() )
		{
			pt = curveIt.position();
			pt *= mCurve;
			CVs[k++] = (float)pt.x;
			CVs[k++] = (float)pt.y;
			CVs[k++] = (float)pt.z;
		}
		CVs[k++] = (float)pt.x;
		CVs[k++] = (float)pt.y;
		CVs[k++] = (float)pt.z;
		CVs[k++] = (float)pt.x;
		CVs[k++] = (float)pt.y;
		CVs[k++] = (float)pt.z;
	}

	liqTokenPointer pointsPointerPair;
	pointsPointerPair.set( "P", rPoint, cvcount );
	pointsPointerPair.setDetailType( rVertex );
	pointsPointerPair.setTokenFloats( CVs );

	// Warning: CVs shares ownership with of its data with pointsPointerPair now!
  // Saves us from redundant copying as long as we know what we are doing
	tokenPointerArray.push_back( pointsPointerPair );

	// constant width or not
	float baseWidth( .1 ), tipWidth( .1 );
	bool constantWidth( false );
  liquidGetPlugValue( fnDag, "liquidCurveBaseWidth", baseWidth, status );
  liquidGetPlugValue( fnDag, "liquidCurveTipWidth", tipWidth, status );

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
		NuCurveWidth = shared_array< RtFloat >( new RtFloat[ cvcount - ncurves * 2 ] );
		k = 0;
		for ( unsigned i( 0 ); i < ncurves; i++ )
		{
			// easy way just linear - might have to be refined
			MItCurveCV itCurve( curveObj[i] );
			NuCurveWidth[k++] = baseWidth;
			NuCurveWidth[k++] = baseWidth;
			for ( unsigned n( 3 ); n < nverts[i] - 3; n++ )
			{
				float difference = tipWidth - baseWidth;
				if ( difference < 0 ) difference *= -1;
				float basew ( baseWidth );
				if ( baseWidth > tipWidth ) NuCurveWidth[k++] = basew - ( n - 2 ) * difference / ( nverts[i] - 5 );
				else												 NuCurveWidth[k++] = basew + ( n - 2 ) * difference / ( nverts[i] - 5 );
			}
			NuCurveWidth[k++] = tipWidth;
			NuCurveWidth[k++] = tipWidth;
		}
		liqTokenPointer widthPointerPair;
		widthPointerPair.set( "width", rFloat, cvcount - ncurves * 2 );
		widthPointerPair.setDetailType( rVarying );
		widthPointerPair.setTokenFloats( NuCurveWidth );
		tokenPointerArray.push_back( widthPointerPair );
	}
	addAdditionalSurfaceParameters( curveGroup );
}

//  Write the RIB for this curve.

void liqRibCurvesData::write()
{
	LIQDEBUGPRINTF( "-> writing nurbs curve group\n" );

	// don't write if empty group
	if ( !ncurves ) return;

	unsigned numTokens( tokenPointerArray.size() );

	scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
	scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
	assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

	RiCurvesV( "cubic", ncurves, nverts.get(), "nonperiodic", numTokens, tokenArray.get(), pointerArray.get() );
}


// Compare this curve to the other for the purpose of determining
//  if it is animated.

bool liqRibCurvesData::compare( const liqRibData & otherObj ) const
{
	LIQDEBUGPRINTF( "-> comparing nurbs curve\n");
	if ( otherObj.type() != MRT_Curves ) return false;
	
	const liqRibCurvesData & other = (liqRibCurvesData&)otherObj;

	if ( ( nverts[0] != other.nverts[0] ) ) return false;

	// Check CVs
	unsigned last = nverts[0] * 3;
	for ( unsigned i = 0; i < last; ++i )
		if ( !equiv( CVs[i], other.CVs[i] ) ) return false;
	return true;
}

// Return the geometry type.

ObjectType liqRibCurvesData::type() const
{
	LIQDEBUGPRINTF( "-> returning nurbs curve type\n" );
	return MRT_Curves;
}
