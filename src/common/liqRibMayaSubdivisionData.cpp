
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
** Liquid Rib Subdivision Mesh Data Source
** ______________________________________________________________________
*/


// Renderman headers
extern "C" {
#include <ri.h>
}

// Maya headers
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MItSubdFace.h>
#include <maya/MItSubdEdge.h>
#include <maya/MFnSubdNames.h>
#include <maya/MUint64Array.h>

#include <maya/MFnSubd.h>
#include <maya/MFnSet.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MUintArray.h>

// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibMayaSubdivisionData.h>

// Standard/Boost headers
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>


using namespace boost;

extern int debugMode;
extern bool liqglo_outputMeshUVs;

/** Create a RIB compatible subdivision surface representation using a Maya polygon mesh.
 */
liqRibMayaSubdivisionData::liqRibMayaSubdivisionData( MObject subd )
:	numFaces( 0 ),
	numPoints ( 0 ),
	nverts(),
	verts(),
	vertexParam( NULL ),
	interpolateBoundary( 0 ),
	uvDetail( rFaceVarying ),
	trueFacevarying( false )
{
	LIQDEBUGPRINTF( "-> creating subdivision surface\n" );
	MFnSubd fnSubd( subd );

	name = fnSubd.name();
	longName = fnSubd.fullPathName();

	checkExtraTags( subd );

	int level = 0;
	numPoints = fnSubd.vertexCount( level );
	numFaces = fnSubd.polygonCount( level );

	MItSubdFace itFace( subd );
	unsigned numFaceVertices( 0 );
	for( itFace.reset(); !itFace.isDone(); itFace.next() )
		numFaceVertices += fnSubd.polygonVertexCount( itFace.index() );

	liqTokenPointer pointsPointerPair;
	liqTokenPointer pFaceVertexSPointer;
	liqTokenPointer pFaceVertexTPointer;

	// Allocate memory and tokens
	nverts = shared_array< RtInt >( new RtInt[ numFaces ] );
	verts = shared_array< RtInt >( new RtInt[ numFaceVertices ] );

	pointsPointerPair.set( "P", rPoint, numPoints );
	pointsPointerPair.setDetailType( rVertex );

	std::vector<liqTokenPointer> UVSetsArray;
	UVSetsArray.reserve( 1 );

	liqTokenPointer pFaceVertexPointerPair;

	pFaceVertexPointerPair.set( "st", rFloat, numFaceVertices, 2 );
	pFaceVertexPointerPair.setDetailType( uvDetail );

	UVSetsArray.push_back( pFaceVertexPointerPair );

	if ( liqglo_outputMeshUVs )
	{
		// Match MTOR, which also outputs face-varying STs as well for some reason - Paul
		// not anymore - Philippe
		pFaceVertexSPointer.set( "u", rFloat, numFaceVertices );
		pFaceVertexSPointer.setDetailType( uvDetail );

		pFaceVertexTPointer.set( "v", rFloat, numFaceVertices );
		pFaceVertexTPointer.setDetailType( uvDetail );
	}

	vertexParam = pointsPointerPair.getTokenFloatArray();

	// Read the subd from Maya
	unsigned count;
	unsigned face( 0 );
	MPoint point;
	unsigned faceVertex( 0 );
	unsigned vertex;
	MUint64Array vertIds;
	MFnSubdNames subNames;
	MDoubleArray S, T;
	for ( itFace.reset(); !itFace.isDone(); itFace.next() )
	{
		MUint64 id = itFace.index();
		fnSubd.polygonVertices( id, vertIds );
		count = vertIds.length();
		nverts[face] = count;
		fnSubd.polygonGetVertexUVs( id, S, T );

		for ( unsigned i( 0 ); i < count; i++ )
		{
			
			vertex = fnSubd.vertexBaseIndexFromVertexId( vertIds[i] );
			verts[faceVertex] = vertex;
			fnSubd.vertexPositionGet( vertIds[i], point, MSpace::kObject );
			pointsPointerPair.setTokenFloat( vertex, point.x, point.y, point.z );

			if ( UVSetsArray.size() )
			{
				UVSetsArray[0].setTokenFloat( faceVertex, 0, S[i] );
				UVSetsArray[0].setTokenFloat( faceVertex, 1, T[i] );

				if ( liqglo_outputMeshUVs )
				{
					// Match MTOR, which always outputs face-varying STs as well for some reason - Paul
					pFaceVertexSPointer.setTokenFloat( faceVertex, S[i] );
					pFaceVertexTPointer.setTokenFloat( faceVertex, T[i] );
				}
			}
			++faceVertex;
		}
		++face;
		vertIds.clear();
		S.clear();
		T.clear();
	}

	// Add tokens to array and clean up after
	tokenPointerArray.push_back( pointsPointerPair );

	if ( UVSetsArray.size() )
		tokenPointerArray.insert( tokenPointerArray.end(), UVSetsArray.begin(), UVSetsArray.end() );
	
	if ( liqglo_outputMeshUVs )
	{
		assert( !pFaceVertexSPointer );
		tokenPointerArray.push_back( pFaceVertexSPointer );
		assert( !pFaceVertexTPointer );
		tokenPointerArray.push_back( pFaceVertexTPointer );
	}

	addAdditionalSurfaceParameters( subd );
}


/** Write the RIB for this mesh.
 */
void liqRibMayaSubdivisionData::write()
{
	LIQDEBUGPRINTF( "-> writing subdivision surface\n" );

	unsigned numTokens( tokenPointerArray.size() );
	scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
	scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
	assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

	RiSubdivisionMeshV( "catmull-clark", numFaces, nverts.get(), verts.get(),
                      v_tags.size(), v_tags.size() ? &v_tags[0] : NULL,
                      v_nargs.size() ? &v_nargs[0] : NULL,
                      v_intargs.size() ? &v_intargs[0] : NULL,
                      v_floatargs.size() ? &v_floatargs[0] : NULL,
                      numTokens, tokenArray.get(), pointerArray.get() );
}

/** Compare this mesh to the other for the purpose of determining if its animated
 */
bool liqRibMayaSubdivisionData::compare( const liqRibData & otherObj ) const
{
  unsigned i;
  unsigned numFaceVertices = 0;

  LIQDEBUGPRINTF( "-> comparing mesh\n" );
  if ( otherObj.type() != MRT_Subdivision ) return false;
  const liqRibMayaSubdivisionData & other = (liqRibMayaSubdivisionData&)otherObj;

  if ( numFaces != other.numFaces ) return false;
  if ( numPoints != other.numPoints ) return false;

  for ( i = 0; i < numFaces; ++i ) 
	{
    if ( nverts[i] != other.nverts[i] ) return false;
    numFaceVertices += nverts[i];
  }

  for ( i = 0; i < numFaceVertices; ++i ) 
    if ( verts[i] != other.verts[i] ) return false;

  for ( i = 0; i < numPoints; ++i ) 
	{
    const unsigned a = i * 3;
    const unsigned b = a + 1;
    const unsigned c = a + 2;
    if ( !equiv( vertexParam[a], other.vertexParam[a] ) ||
      !equiv( vertexParam[b], other.vertexParam[b] ) ||
      !equiv( vertexParam[c], other.vertexParam[c] ) )
    {
      return false;
    }
  }
  return true;
}

/** Return the geometry type.
 */
ObjectType liqRibMayaSubdivisionData::type() const
{
  LIQDEBUGPRINTF( "-> returning subdivision surface type\n" );
  return MRT_MayaSubdivision;
}

void liqRibMayaSubdivisionData::checkExtraTags( MObject &subd )
{
	MStatus status = MS::kSuccess;
	MFnSubd fnSubd( subd );

	// this looks redundant but there is no other way to determine
	// if the object has creases at all
	MUint64Array vertexIds, edgeIds;
	status = fnSubd.creasesGetAll( vertexIds, edgeIds );
	if ( status == MS::kSuccess )
	{
		if ( vertexIds.length() ) addExtraTags( subd, 0, TAG_CORNER );
		if ( edgeIds.length() )   addExtraTags( subd, 0, TAG_CREASE );
	}

	// set defaults
	interpolateBoundary = 2;
	uvDetail = rFaceVarying;
	trueFacevarying = true;

	MPlug interpolateBoundaryPlug = fnSubd.findPlug( "liqSubdivInterpolateBoundary", &status );
	if ( status == MS::kSuccess ) interpolateBoundaryPlug.getValue( interpolateBoundary );

	MPlug liqSubdivUVInterpolationPlug = fnSubd.findPlug( "liqSubdivUVInterpolation", &status );
	if ( status == MS::kSuccess )
	{
		int liqSubdivUVInterpolation;
		liqSubdivUVInterpolationPlug.getValue( liqSubdivUVInterpolation );
		switch ( liqSubdivUVInterpolation )
		{
			case 0: // true facevarying
				trueFacevarying = true;
			case 1: //
				uvDetail = rFaceVarying;
				break;
			case 2:
				uvDetail = rFaceVertex;
				break;
		}
	}

	if ( interpolateBoundary ) addExtraTags( subd, interpolateBoundary, TAG_BOUNDARY );
	if ( trueFacevarying )     addExtraTags( subd, 0, TAG_FACEVARYINGBOUNDARY );
}

void liqRibMayaSubdivisionData::addExtraTags( MObject &subd, int extraTagValue, SBD_EXTRA_TAG extraTag )
{
	MStatus status;
	MFnSubd fnSubd( subd );
	MFnSubdNames subNames;
	MUintArray ids;
	MDoubleArray creaseData;

	if ( TAG_BOUNDARY == extraTag )
	{
		v_tags.push_back( "interpolateboundary" );
		v_nargs.push_back( 1 );		// 0 intargs
		v_nargs.push_back( 0 );		// 0 floatargs
		v_intargs.push_back( extraTagValue );
	}

	if ( TAG_FACEVARYINGBOUNDARY == extraTag )
	{
		v_tags.push_back( "facevaryinginterpolateboundary" );
		v_nargs.push_back( 1 );		// 1 intargs
		v_nargs.push_back( 0 );		// 0 floatargs
		v_intargs.push_back( extraTagValue );
	}

	if ( TAG_CREASE == extraTag )
	{
		MUint64Array vertexIds, edgeIds;
		status = fnSubd.creasesGetAll( vertexIds, edgeIds );
		if ( status == MS::kSuccess )
		{
			MUint64 vert1, vert2;
			for ( unsigned i( 0 ); i < edgeIds.length(); i++ )
			{
				fnSubd.edgeVertices( edgeIds[i], vert1, vert2 );
				int baseVert1 = fnSubd.vertexBaseIndexFromVertexId( vert1, &status );
				// only base edges
				if ( status != MS::kSuccess ) continue;

				int baseVert2 = fnSubd.vertexBaseIndexFromVertexId( vert2, &status );
				// only base edges
				if( status != MS::kSuccess ) continue;
				
				//MGlobal::displayInfo( MString( "BASE: " ) + baseVert1 + " " + baseVert2 + " " + i );
				v_tags.push_back( "crease" );
				v_nargs.push_back( 2 );
				v_nargs.push_back( 1 );
				v_intargs.push_back( baseVert1 );
				v_intargs.push_back( baseVert2 );
				v_floatargs.push_back( 7 );
			}
		}
	}

	if ( TAG_CORNER == extraTag )
	{
		MUint64Array vertexIds, edgeIds;
		status = fnSubd.creasesGetAll( vertexIds, edgeIds );
		if ( status == MS::kSuccess )
		{
			for ( unsigned i( 0 ); i < vertexIds.length(); i++ )
			{
				int baseId = fnSubd.vertexBaseIndexFromVertexId( vertexIds[i], &status );

				// only base verts
				if ( status != MS::kSuccess ) continue;

				v_tags.push_back( "corner" );
				v_nargs.push_back( 1 );
				v_nargs.push_back( 1 );
				v_intargs.push_back( baseId );
				v_floatargs.push_back( 7 );
			}
		}
	}
}