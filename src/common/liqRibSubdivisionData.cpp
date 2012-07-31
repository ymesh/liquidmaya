
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
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MUintArray.h>
// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibSubdivisionData.h>
// Standard/Boost headers
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

using namespace boost;

extern int debugMode;
extern bool liqglo_outputMeshUVs;
extern bool liqglo_outputMayaPolyCreases;  // use maya poly creases instead of liquid crease sets
extern bool liqglo_useMtorSubdiv;  // interpret mtor subdiv attributes
//extern bool liqglo_outputMeshAsRMSArrays;
/*
 * Create a RIB compatible subdivision surface representation using a Maya polygon mesh.
 */
liqRibSubdivisionData::liqRibSubdivisionData()
  : liqRibMeshData(),
    interpolateBoundary( 0 ),
    uvDetail( rFaceVarying ),
    trueFacevarying( false )
{
  subdivScheme = "catmull-clark";
}
/*
 *
 */
liqRibSubdivisionData::liqRibSubdivisionData( MObject mesh, bool initSubdivData )
  : liqRibMeshData( mesh, false ),
    interpolateBoundary( 0 ),
    uvDetail( rFaceVarying ),
    trueFacevarying( false )
{
  LIQDEBUGPRINTF( "=> creating subdiv\n" );
  
  subdivScheme = "catmull-clark";
  if ( initSubdivData )
  {
    cerr << "liqRibSubdivisionData::initSubdivData" << endl << flush;
    checkExtraTags( mesh );
  }
}
/** Write the RIB for this mesh.
 */
void liqRibSubdivisionData::write()
{
  LIQDEBUGPRINTF( "-> writing subdiv\n" );
  
  unsigned numTokens( tokenPointerArray.size() );
  scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
  scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
  assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

  RiSubdivisionMeshV( subdivScheme, 
                      get_numFaces(), 
                      get_nverts(), 
                      get_verts(),
                      v_tags.size(), 
                      v_tags.size() ? &v_tags[0] : NULL,
                      v_nargs.size() ? &v_nargs[0] : NULL,
                      v_intargs.size() ? &v_intargs[0] : NULL,
                      v_floatargs.size() ? &v_floatargs[0] : NULL,
                      numTokens, 
                      tokenArray.get(), 
                      pointerArray.get() );
}

/*
 * Compare this mesh to the other for the purpose of determining if its animated
 */
bool liqRibSubdivisionData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing subdiv\n" );
  if( otherObj.type() != MRT_Subdivision ) return false;
  const liqRibMeshData& other = ( liqRibMeshData& )otherObj;
  return compareMesh ( other, false );
}
/*
 * Return the geometry type.
 */
ObjectType liqRibSubdivisionData::type() const
{
  LIQDEBUGPRINTF( "-> returning subdiv type\n" );
  return MRT_Subdivision;
}

// Creases, corners, holes are organized by MEL script into maya sets
// that are connected to shape and have corresponded attributes:
// liqSubdivCrease, liqSubdivCorner, liqSubdivHole, liqSubdivStitch
// 'stitch' -- new tag introduced in PRman 11 suited for seamless
// stitching two subdiv surfaces aligned by common edge line.
// If global flag liqglo_useMtorSubdiv is set, then procedure looks also
// for analog mtor attributes
//
void liqRibSubdivisionData::checkExtraTags ( MObject &mesh ) 
{
	cerr << "liqRibSubdivisionData::checkExtraTags" << endl << flush;
	
	MStatus status = MS::kSuccess;
  MFnMesh fnMesh ( mesh );
	
	if ( liqglo_outputMayaPolyCreases ) 
    addExtraTagsFromMaya ( mesh );	

	addExtraTagsFromSets ( mesh );

	bool interpolateBoundaryOld = false;
  bool mtor_interpolateBoundary = false;
  int liqSubdivUVInterpolation = -1;
  interpolateBoundary = 2;  
  
  liquidGetPlugValue( fnMesh, "liqSubdivInterpolateBoundary", interpolateBoundary, status );
  liquidGetPlugValue( fnMesh, "interpBoundary", interpolateBoundaryOld, status );
	
	if ( liqglo_useMtorSubdiv ) 
    liquidGetPlugValue( fnMesh, "mtorSubdivInterp", mtor_interpolateBoundary, status );
  if ( mtor_interpolateBoundary || interpolateBoundaryOld ) interpolateBoundary = 2; // Old School

  liquidGetPlugValue( fnMesh, "liqSubdivUVInterpolation", liqSubdivUVInterpolation, status );
	addBoundaryTags ( liqSubdivUVInterpolation );
}
/*
 *
 */
void liqRibSubdivisionData::addBoundaryTags ( int liqSubdivUVInterpolation )
{
  // set defaults
  // interpolateBoundary = 2; should be set before while "liqSubdivInterpolateBoundary" attribute check
  // 
	uvDetail = rFaceVarying;
	trueFacevarying = true;
  
  switch( liqSubdivUVInterpolation )
	{
		case 0: // true facevarying
		  trueFacevarying = true;
		case 1: //
		  uvDetail = rFaceVarying;
      break;
		case 2:
		  uvDetail = rFaceVertex;
      break;
    // liqSubdivUVInterpolation = -1 ( No "liqSubdivUVInterpolation" attribute )
    default: break;
	}
  if ( interpolateBoundary ) addExtraTag( interpolateBoundary, TAG_BOUNDARY );
	if ( trueFacevarying ) addExtraTag( (int)0, TAG_FACEVARYINGBOUNDARY );
}
// this is a temporary solution - the maya2008 polycreases are a bit crap in
// that they cannot be removed and there is no way at the moment to tag
// faces as holes in Maya to we keep the "set" way of doing it - Alf
void liqRibSubdivisionData::addExtraTagsFromMaya ( MObject &mesh )
{
	// this looks redundant but there is no other way to determine
	// if the object has creases at all
	MStatus      status = MS::kSuccess;
  MFnMesh      fnMesh( mesh );
  MUintArray   ids;
	MDoubleArray creaseData;

	status = fnMesh.getCreaseEdges ( ids, creaseData );
	if ( status == MS::kSuccess ) 
  {
    MItMeshEdge itEdge( mesh );
		int prevIndex;
		for ( unsigned i( 0 ); i < ids.length(); i++ )
    {
      itEdge.setIndex( ids[i], prevIndex );
      addCreaseTag ( itEdge.index( 0 ), itEdge.index( 1 ), creaseData[ i ] );
    }
	}
  status = fnMesh.getCreaseVertices( ids, creaseData );
	if ( status == MS::kSuccess ) 
  {
    for ( unsigned i( 0 ); i < ids.length(); i++ )
      addCornerTag ( ids[i], creaseData[i] );
  }
}
/*
 *
 */
void liqRibSubdivisionData::addExtraTagsFromSets ( MObject &mesh )
{
	MStatus     status = MS::kSuccess;
  MFnMesh     fnMesh( mesh );
	MPlugArray  array;
	MFnDependencyNode depNode( mesh );
	depNode.getConnections( array ); 
  
  char *creaseAttrName = "liqSubdivCrease";
  char *cornerAttrName = "liqSubdivCorner";
  char *holeAttrName = "liqSubdivHole";
  char *stichAttrName = "liqSubdivStitch";
  
  if ( liqglo_useMtorSubdiv ) // check mtor subdivisions extra tag
  {
    creaseAttrName = "mtorSubdivCrease";
    cornerAttrName = "mtorSubdivCorner";
    holeAttrName = "mtorSubdivHole";
  }
  // collect all plugs connected to mesh
  // for each plug check if it is connected as dst to maya set
	// and this set has attribute for subdiv extra tag with non zero value
	
  for ( unsigned i = 0 ; i < array.length() ; i++ )
	{
		MPlugArray connections;
		MPlug curPlug = array[i];

		if ( !curPlug.connectedTo( connections, false, true ) ) continue; 
		/* look only for plugs connected as dst (src = false) */

		for ( unsigned i = 0 ; i < connections.length() ; i++ )
		{
			MPlug dstPlug = connections[i];
			MObject dstNode = dstPlug.node();

			if ( dstNode.hasFn( MFn::kSet ) ) /* if connected to set */
			{	
				float floatTagValue;
        int   intTagValue;
        SBD_EXTRA_TAG extraTag = TAG_NONE;
 
				MFnDependencyNode setNode( dstNode, &status );
				if ( status != MS::kSuccess ) continue;
        
        MFnSet elemSet( dstNode, &status ); // dstNode is maya components set
	      if ( status != MS::kSuccess ) continue;
				
        if ( !liqglo_outputMayaPolyCreases )
        {
          if ( liquidGetPlugValue( setNode, creaseAttrName, floatTagValue, status ) == MS::kSuccess )
          { 
            if ( floatTagValue != 0.0 ) extraTag = TAG_CREASE;
          } 
          else if ( liquidGetPlugValue( setNode, cornerAttrName, floatTagValue, status ) == MS::kSuccess )
          { 
            if ( floatTagValue != 0.0  ) extraTag = TAG_CORNER;
          }
        }
        if ( liquidGetPlugValue( setNode, holeAttrName, intTagValue, status ) == MS::kSuccess )
        {
          if ( intTagValue ) extraTag = TAG_HOLE;
        }
				else if ( liquidGetPlugValue( setNode, stichAttrName, intTagValue, status ) == MS::kSuccess )
        { 
          // intTagValue == stitch curve ID
          if ( intTagValue ) extraTag = TAG_STITCH;
        }
			  // if set has attributes for supported extra tag 
        if ( extraTag != TAG_NONE )
        {
          MSelectionList members;
		      status = elemSet.getMembers( members, true ); // get flatten members list
		      if ( status == MS::kSuccess )
		      {
			      for ( unsigned i( 0 ) ; i < members.length() ; i++ ) // iterate through set members
			      {
				      MObject component;
				      MDagPath dagPath;
				      members.getDagPath ( i, dagPath, component );
              if ( component.isNull() ) continue;
				      // since the crease set could contain more that one mesh
				      // we only want the current one - Alf
				      if ( dagPath.fullPathName() != get_longName() ) continue;
				      switch ( extraTag )
				      {
					      case TAG_CREASE:
					        if ( component.hasFn( MFn::kMeshEdgeComponent ) )
					        {
						        MItMeshEdge edgeIter( dagPath, component );
						        for (  ; !edgeIter.isDone(); edgeIter.next() )
						          addCreaseTag ( edgeIter.index( 0 ), edgeIter.index( 1 ), floatTagValue );
					        } break;
					      case TAG_CORNER:
					        if ( component.hasFn( MFn::kMeshVertComponent ) )
					        {
						        MItMeshVertex  vertexIter( dagPath, component );
						        for (  ; !vertexIter.isDone(); vertexIter.next() ) 
                      addCornerTag ( vertexIter.index(), floatTagValue );
					        } break;
					      case TAG_HOLE:
					        if ( component.hasFn( MFn::kMeshPolygonComponent ) )
					        {
						        MItMeshPolygon  faceIter( dagPath, component );
                    addHoleTag ( faceIter );
					        } break;
					      case TAG_STITCH:
					        if ( component.hasFn( MFn::kMeshVertComponent ) )
					        {
						        MItMeshVertex vertexIter( dagPath, component );
						        addStitchTag ( vertexIter, intTagValue );
					        } break;
					      default: break;
				      }
			      }
		      }
        }
      }
		}
	}
}

/*
 *
 */
void liqRibSubdivisionData::addExtraTag ( int intValue, SBD_EXTRA_TAG extraTag )
{
  cerr << "liqRibSubdivisionData::addExtraTag int" << endl << flush;
  
  if ( TAG_BOUNDARY == extraTag ) v_tags.push_back( "interpolateboundary" );
  else if ( TAG_FACEVARYINGBOUNDARY == extraTag ) v_tags.push_back( "facevaryinginterpolateboundary" );
  else if ( TAG_FACEVARYINGPROPAGATECORNERS == extraTag ) v_tags.push_back( "facevaryingpropagatecorners" );
  v_nargs.push_back( 1 );		// 1 intargs
  v_nargs.push_back( 0 );		// 0 floatargs
  v_intargs.push_back( intValue );
}
/*
 *
 */
void liqRibSubdivisionData::addCornerTag ( int intValue, float floatValue )
{
  v_tags.push_back( "corner" );
  v_nargs.push_back( 1 );		// 1 intargs
  v_nargs.push_back( 1 );		// 1 floatargs
  v_intargs.push_back( intValue );
  v_floatargs.push_back( floatValue );
}
/*
 *
 */
void liqRibSubdivisionData::addCreaseTag ( int intValue1, int intValue2, float floatValue )
{
  v_tags.push_back( "crease" );
  v_nargs.push_back( 2 );		// 1 intargs
  v_nargs.push_back( 1 );		// 1 floatargs
  v_intargs.push_back( intValue1 );
  v_intargs.push_back( intValue2 );
  v_floatargs.push_back( floatValue );
}
/*
 *
 */
void liqRibSubdivisionData::addHoleTag ( MItMeshPolygon &faceIter )
{
  v_tags.push_back( "hole" );
  v_nargs.push_back( faceIter.count() );	// 1 intargs
  v_nargs.push_back( 0 );		// 0 floatargs
  for (  ; !faceIter.isDone(); faceIter.next() ) 
    v_intargs.push_back( faceIter.index() );
}
/*
 *
 */
void liqRibSubdivisionData::addStitchTag ( MItMeshVertex &vertexIter, int intTagValue )
{
  v_tags.push_back( "stitch" );
  v_nargs.push_back( vertexIter.count() + 1 ); // vertex count in chain + 1 integer curve identifier
  v_nargs.push_back( 0 );                      // 0 floatargs
  v_intargs.push_back( intTagValue );
  for (  ; !vertexIter.isDone(); vertexIter.next() ) 
    v_intargs.push_back( vertexIter.index() );
}