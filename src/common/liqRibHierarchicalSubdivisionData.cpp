
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
** Liquid Rib Hierarchical Subdivision Mesh Data Source
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
#include <liqRibHierarchicalSubdivisionData.h>
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
liqRibHierarchicalSubdivisionData::liqRibHierarchicalSubdivisionData()
  : liqRibSubdivisionData()
{
  //initializeSubdivParameters();
}
/*
 *
 */
liqRibHierarchicalSubdivisionData::liqRibHierarchicalSubdivisionData( MObject mesh, bool initSubdivData )
  : liqRibSubdivisionData( mesh, false )  // no init data
{
	LIQDEBUGPRINTF( "=> creating hierarchical subdiv\n" );
  // initializeSubdivParameters();
  //addExtraTag( "chaikin", TAG_CREASEMETHOD );
	//addExtraTag( 1, TAG_FACEVARYINGPROPAGATECORNERS );
	//addExtraTag( 1, TAG_BOUNDARY );
  if ( initSubdivData )
  {
    cerr << "liqRibHierarchicalSubdivisionData::initSubdivData" << endl << flush;
    checkExtraTags( mesh );
  }
}

// liqRibHierarchicalSubdivisionData::~liqRibHierarchicalSubdivisionData() {}

void liqRibHierarchicalSubdivisionData::initializeSubdivParameters()
{
	subdivScheme = "catmull-clark";
	addExtraTag( "chaikin", TAG_CREASEMETHOD );
	addExtraTag( 1, TAG_FACEVARYINGPROPAGATECORNERS );
	addExtraTag( 1, TAG_BOUNDARY );
}
/*
 * Write the RIB for this mesh.
 */
void liqRibHierarchicalSubdivisionData::write()
{
  LIQDEBUGPRINTF( "-> writing hierarchical subdiv\n" );

  unsigned numTokens( tokenPointerArray.size() );
  scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
  scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
  assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

  RiHierarchicalSubdivisionMeshV(	subdivScheme, 
									get_numFaces(),
									get_nverts(), 
									get_verts(),
									v_tags.size(), 
                  v_tags.size() ? &v_tags[0] : NULL,
                  v_nargs.size() ? &v_nargs[0] : NULL,
                  v_intargs.size() ? &v_intargs[0] : NULL,
                  v_floatargs.size() ? &v_floatargs[0] : NULL,
                  v_stringargs.size() ? &v_stringargs[0] : NULL,
									numTokens,
									tokenArray.get(),
									pointerArray.get() );
}

/*
 * Compare this mesh to the other for the purpose of determining if its animated
 */
bool liqRibHierarchicalSubdivisionData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing hierarchical subdiv\n" );
  if( otherObj.type() != MRT_Subdivision ) return false;
  const liqRibMeshData& other = ( liqRibMeshData& )otherObj;
  return compareMesh ( other, false );
}

/** Return the geometry type.
 */
ObjectType liqRibHierarchicalSubdivisionData::type() const
{
  LIQDEBUGPRINTF( "-> returning hierarchical subdiv type\n" );
  return MRT_Subdivision;
}

/*
 *
 */
void liqRibHierarchicalSubdivisionData::addExtraTag( int intValue, SBD_EXTRA_TAG extraTag )
{
  cerr << "liqRibHierarchicalSubdivisionData::addExtraTag int" << endl << flush;
  
  if ( TAG_BOUNDARY == extraTag ) v_tags.push_back( "interpolateboundary" );
  else if ( TAG_FACEVARYINGBOUNDARY == extraTag ) v_tags.push_back( "facevaryinginterpolateboundary" );
  else if ( TAG_FACEVARYINGPROPAGATECORNERS == extraTag ) v_tags.push_back( "facevaryingpropagatecorners" );
  else if ( TAG_HOLE == extraTag ) v_tags.push_back( "hole" );
  v_nargs.push_back( 1 );		// 1 intargs
  v_nargs.push_back( 0 );		// 0 floatargs
  v_nargs.push_back( 0 );		// 0 stringargs
  v_intargs.push_back( intValue );
}
/*
 *
 */
void liqRibHierarchicalSubdivisionData::addCornerTag ( int intValue, float floatValue )
{
  v_tags.push_back( "corner" );
  v_nargs.push_back( 1 );		// 1 intargs
  v_nargs.push_back( 1 );		// 1 floatargs
  v_nargs.push_back( 0 );   // 0 stringargs
  v_intargs.push_back( intValue );
  v_floatargs.push_back( floatValue );
}
/*
 *
 */
void liqRibHierarchicalSubdivisionData::addCreaseTag ( int intValue1, int intValue2, float floatValue )
{
  v_tags.push_back( "crease" );
  v_nargs.push_back( 2 );		// 1 intargs
  v_nargs.push_back( 1 );		// 1 floatargs
  v_nargs.push_back( 0 );   // 0 stringargs
  v_intargs.push_back( intValue1 );
  v_intargs.push_back( intValue2 );
  v_floatargs.push_back( floatValue );
}
/*
 *
 */
void liqRibHierarchicalSubdivisionData::addHoleTag ( MItMeshPolygon &faceIter )
{
  v_tags.push_back( "hole" );
  v_nargs.push_back( faceIter.count() );	// 1 intargs
  v_nargs.push_back( 0 );		// 0 floatargs
  v_nargs.push_back( 0 );   // 0 stringargs
  for (  ; !faceIter.isDone(); faceIter.next() ) 
    v_intargs.push_back( faceIter.index() );
}
/*
 *
 */
void liqRibHierarchicalSubdivisionData::addStitchTag ( MItMeshVertex &vertexIter, int intTagValue )
{
  v_tags.push_back( "stitch" );
  v_nargs.push_back( vertexIter.count() + 1 ); // vertex count in chain + 1 integer curve identifier
  v_nargs.push_back( 0 );                      // 0 floatargs
  v_nargs.push_back( 0 );   // 0 stringargs
  v_intargs.push_back( intTagValue );
  for (  ; !vertexIter.isDone(); vertexIter.next() ) 
    v_intargs.push_back( vertexIter.index() );
}
/*
 *   "vertexedit", "edgeedit", and "faceedit". 
 */
void liqRibHierarchicalSubdivisionData::addExtraTag( const char *stringValue, SBD_EXTRA_TAG extraTag )
{
  cerr << "liqRibHierarchicalSubdivisionData::addExtraTag string" << endl << flush;
  if ( TAG_CREASEMETHOD == extraTag ) // creasemethod args : 1 string : "normal" | "chaikin"  // inutile puisque pas de crease ...
  {
    v_tags.push_back( "creasemethod" );
    v_nargs.push_back( 0 );		// 0 intargs
	  v_nargs.push_back( 0 );		// 0 floatargs
    v_nargs.push_back( 1 );		// 1 stringargs
	  v_stringargs.push_back( (RtString)stringValue );
  }
}
