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
** Liquid Rib Mesh Data Source
** ______________________________________________________________________
*/


// Renderman Headers
extern "C" {
#include <ri.h>
}

// Maya headers
#include <maya/MPlug.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MMatrix.h>
#include <maya/MFloatPointArray.h>

// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibMeshData.h>

// Standard/Boost headers
#include <vector>
#include <iostream>
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

using namespace boost;
using namespace std;

extern int debugMode;
extern bool liqglo_outputMeshUVs;
extern bool liqglo_outputMeshAsRMSArrays;

/*
 * Create a RIB compatible representation of a Maya polygon mesh.
 */
liqRibMeshData::liqRibMeshData()
: numFaces( 0 ),
  numPoints ( 0 ),
  numNormals ( 0 ),
  nverts(),
  verts(),
  vertexParam(),
  normalParam()
{
}
/*
*
*/
liqRibMeshData::liqRibMeshData( MObject mesh, bool useNormals )
: numFaces( 0 ),
  numPoints ( 0 ),
  numNormals ( 0 ),
  nverts(),
  verts(),
  vertexParam(),
  normalParam()
{
  LIQDEBUGPRINTF( "-> creating mesh\n" );
  if ( getMayaData ( mesh, useNormals ) )
  { 
    addAdditionalSurfaceParameters( mesh );
  }
}
/*
 *  get maya poly mesh data
 */
bool liqRibMeshData::getMayaData( MObject mesh, bool useNormals )
{
  bool ret = true;
  
  // LIQDEBUGPRINTF( "-> mesh getMayaData (useNormals = %s )\n", ( ( useNormals )? "Yes" : "No" ) );

  MFnMesh fnMesh( mesh );
  objDagPath = fnMesh.dagPath();
  MStatus astatus;
  
  name = fnMesh.name();
  longName = fnMesh.fullPathName();

  numPoints = fnMesh.numVertices();
  numFaces = fnMesh.numPolygons();
  if ( useNormals ) numNormals = fnMesh.numNormals();
  
  if ( numPoints < 1 )
	{
	  liquidMessage( "Could not export degenerate mesh " + longName, messageInfo );
		return false;
	}
	// UV sets -------------------
  //
  //const unsigned numSTs( fnMesh.numUVs() );
  const unsigned numUVSets( fnMesh.numUVSets() );
  MString currentUVSetName;
	fnMesh.getCurrentUVSetName( currentUVSetName );
  MStringArray extraUVSetNames;
  MStringArray UVSetNames;
  fnMesh.getUVSetNames( UVSetNames );

  for ( unsigned i( 0 ); i < numUVSets ; i++ ) 
    if ( UVSetNames[i] != currentUVSetName ) 
      extraUVSetNames.append( UVSetNames[i] );
  
  const unsigned numFaceVertices( fnMesh.numFaceVertices() );
  unsigned face ( 0 );
  unsigned faceVertex ( 0 );
  unsigned count;
  unsigned vertex;
  unsigned normal;
  float S;
  float T;
  MPoint point;
  liqTokenPointer pointsPointerPair;
  liqTokenPointer normalsPointerPair;
  liqTokenPointer pFaceVertexSPointer;
  liqTokenPointer pFaceVertexTPointer;
  
  // Allocate memory and tokens
  nverts = shared_array< RtInt >( new RtInt[ numFaces ] );
  verts = shared_array< RtInt >( new RtInt[ numFaceVertices ] );

  pointsPointerPair.set( "P", rPoint, numPoints );
  pointsPointerPair.setDetailType( rVertex );

  if ( useNormals )
  {
    if ( numNormals == numPoints ) 
    {
      normalsPointerPair.set( "N", rNormal, numPoints );
      normalsPointerPair.setDetailType( rVertex );
    } 
    else 
    {
      normalsPointerPair.set( "N", rNormal, numFaceVertices );
      normalsPointerPair.setDetailType( rFaceVarying );
    }
  }
  // uv
  std::vector<liqTokenPointer> UVSetsArray;
  UVSetsArray.reserve( 1 + extraUVSetNames.length() );
	liqTokenPointer currentUVSetUPtr;
	liqTokenPointer currentUVSetVPtr;
	liqTokenPointer currentUVSetNamePtr;
	liqTokenPointer extraUVSetsUPtr;
	liqTokenPointer extraUVSetsVPtr;
	liqTokenPointer extraUVSetsNamePtr;
	
	if ( liqglo_outputMeshAsRMSArrays )
	{
		currentUVSetUPtr.set( "s", rFloat, numFaceVertices );
		currentUVSetUPtr.setDetailType( rFaceVarying );

		currentUVSetVPtr.set( "t", rFloat, numFaceVertices );
		currentUVSetVPtr.setDetailType( rFaceVarying );

		currentUVSetNamePtr.set( "currentUVSet", rString, 1 );
		currentUVSetNamePtr.setDetailType( rConstant );
		
		if ( numUVSets > 1 )
		{
			extraUVSetsUPtr.set( "u_uvSet", rFloat, numFaceVertices, numUVSets-1 );
			extraUVSetsUPtr.setDetailType( rFaceVarying );

			extraUVSetsVPtr.set( "v_uvSet", rFloat, numFaceVertices, numUVSets-1 );
			extraUVSetsVPtr.setDetailType( rFaceVarying );

			extraUVSetsNamePtr.set( "extraUVSets", rString, numUVSets-1 );
			extraUVSetsNamePtr.setDetailType( rConstant );
		}
	}
  else
	{
		if ( numUVSets > 0 )
  	{
   	 	liqTokenPointer pFaceVertexPointerPair;

    	pFaceVertexPointerPair.set( "st", rFloat, numFaceVertices, 2 );
    	pFaceVertexPointerPair.setDetailType( rFaceVarying );

    	UVSetsArray.push_back( pFaceVertexPointerPair );

    	for ( unsigned j( 0 ); j < extraUVSetNames.length() ; j++ ) 
    	{
      	liqTokenPointer pFaceVertexPointerPair;

      	pFaceVertexPointerPair.set( extraUVSetNames[j].asChar(), rFloat, numFaceVertices, 2 );
      	pFaceVertexPointerPair.setDetailType( rFaceVarying );

      	UVSetsArray.push_back( pFaceVertexPointerPair );
    	}

    	if ( liqglo_outputMeshUVs ) 
    	{
      	// Match MTOR, which also outputs face-varying STs as well for some reason - Paul
      	// not anymore - Philippe
      	pFaceVertexSPointer.set( "u", rFloat, numFaceVertices );
      	pFaceVertexSPointer.setDetailType( rFaceVarying );

      	pFaceVertexTPointer.set( "v", rFloat, numFaceVertices );
      	pFaceVertexTPointer.setDetailType( rFaceVarying );
    	}
		}
  }
  vertexParam = pointsPointerPair.getTokenFloatArray();
  MFloatVectorArray normals;

  // Read the mesh normals from Maya
  if ( useNormals ) 
  { 
    normalParam = normalsPointerPair.getTokenFloatArray();
    fnMesh.getNormals( normals );
  }
  for ( MItMeshPolygon polyIt ( mesh ); polyIt.isDone() == false ; polyIt.next() ) 
  {
    count = polyIt.polygonVertexCount();
    nverts[face] = count;
    unsigned j, i = count;
	  
    // printf("poly count = %d\n", count );
    
    while ( i )
    {
      --i;
      vertex = polyIt.vertexIndex( i );
      verts[faceVertex] = vertex;
      point = polyIt.point( i, MSpace::kObject );
      pointsPointerPair.setTokenFloat( vertex, point.x, point.y, point.z );
      
      if ( useNormals )
      {
        normal = polyIt.normalIndex( i );
        if ( numNormals == numPoints ) 
          normalsPointerPair.setTokenFloat( vertex, normals[normal].x, normals[normal].y, normals[normal].z );
        else 
          normalsPointerPair.setTokenFloat( faceVertex, normals[normal].x, normals[normal].y, normals[normal].z );
      }
      
      if ( liqglo_outputMeshAsRMSArrays )
			{
				for ( j = 0 ; j < numUVSets ; j++ )
				{
					if ( j == 0)
					{
						MString uvSetName = currentUVSetName;
						// set uvSet name
						currentUVSetNamePtr.setTokenString( 0, currentUVSetName.asChar() );
						// set uv values
						fnMesh.getPolygonUV( face, i, S, T, &uvSetName );

						currentUVSetUPtr.setTokenFloat( faceVertex, S );
						currentUVSetVPtr.setTokenFloat( faceVertex, 1-T );
					}
					else
					{
						MString uvSetName = extraUVSetNames[j-1];
						// set uvSet name
						extraUVSetsNamePtr.setTokenString( j-1, extraUVSetNames[j-1].asChar() );
						// set uv values
						fnMesh.getPolygonUV( face, i, S, T, &uvSetName );
						extraUVSetsUPtr.setTokenFloat( (numFaceVertices*(j-1)) + faceVertex, S );
						extraUVSetsVPtr.setTokenFloat( (numFaceVertices*(j-1)) + faceVertex, 1-T );
					}
				}
			}
			else
			{
				if ( numUVSets  )
				{
					for( j = 0; j < numUVSets; j++ )
					{
						MString uvSetName = ( j == 0 )? currentUVSetName: extraUVSetNames[ j - 1 ] ;

						fnMesh.getPolygonUV( face, i, S, T, &uvSetName );
						UVSetsArray[j].setTokenFloat( faceVertex, 0, S );
						UVSetsArray[j].setTokenFloat( faceVertex, 1, 1 - T );
						//printf("V%d  %s : %f %f  =>  %f %f \n", i, uvSetName.asChar(), S, T, S, 1-T);

						if ( liqglo_outputMeshUVs && j==0)
						{
							// Match MTOR, which always outputs face-varying STs as well for some reason - Paul
							pFaceVertexSPointer.setTokenFloat( faceVertex, S );
							pFaceVertexTPointer.setTokenFloat( faceVertex, 1 - T );
						}
					}
				}
			}
			++faceVertex;
		}
		++face;
	}
  // Add tokens to array and clean up after
  tokenPointerArray.push_back( pointsPointerPair );
  if ( useNormals ) 
    tokenPointerArray.push_back( normalsPointerPair );
	
	if ( liqglo_outputMeshAsRMSArrays )
	{
		tokenPointerArray.push_back( currentUVSetNamePtr );
		tokenPointerArray.push_back( currentUVSetUPtr );
		tokenPointerArray.push_back( currentUVSetVPtr );
		if ( numUVSets > 1 )
		{
			tokenPointerArray.push_back( extraUVSetsNamePtr );
			tokenPointerArray.push_back( extraUVSetsUPtr );
			tokenPointerArray.push_back( extraUVSetsVPtr );
		}
	}
	else
	{
  	if ( UVSetsArray.size() ) 
    	tokenPointerArray.insert( tokenPointerArray.end(), UVSetsArray.begin(), UVSetsArray.end() );
  
  	if ( liqglo_outputMeshUVs ) 
  	{
    	tokenPointerArray.push_back( pFaceVertexSPointer );
    	tokenPointerArray.push_back( pFaceVertexTPointer );
  	}
	}
  return ret;  
}
/**      Print data about this mesh.
 */
/*
void liqRibMeshData::printMesh()
{
	int i;
  scoped_array< RtInt > nloops( new RtInt[ numFaces ] );
	unsigned numTokens( tokenPointerArray.size() );

	printf ( "numFace %d \n", numFaces );
	printf ( "nloops (%d) [ ", numFaces );
	for ( i = 0; i < numFaces; i++ ) printf ( " %d", nloops[i] );
	printf ("]\n" );

	int nvertsSize = nverts.use_count();
	RtInt *nvertsPtr = nverts.get();
	printf("nverts (%d) [ ", nvertsSize);
	for ( i = 0; i < nvertsSize ; i++ ) printf ( " %d", nvertsPtr[i] );
	printf ( "]\n" );

	int vertsSize = verts.use_count();
	RtInt *vertsPtr = verts.get();
	printf("nverts (%d) [ ", vertsSize);
	for ( i = 0; i < vertsSize ; i++ ) printf ( " %d", vertsPtr[i] );
	printf ( "]\n" );

	printf ( "numTokens (%d)\n", numTokens );
	printf ( "\n" );
	// print tokens & pointers
}
*/

/*
 *  Write the RIB for this mesh.
 */
void liqRibMeshData::write()
{
  LIQDEBUGPRINTF( "-> writing mesh\n" );
  writeMesh();
}

void liqRibMeshData::writeMesh()
{
//  if ( numPoints > 1 ) 
  // Each loop has one polygon, so we just want an array of 1's of
  // the correct size. Stack version.
  //vector< RtInt > nloops( numFaces, 1 );
  // Alternatively (heap version):
  scoped_array< RtInt > nloops( new RtInt[ numFaces ] );
  fill( nloops.get(), nloops.get() + numFaces, ( RtInt )1 );

  unsigned numTokens( tokenPointerArray.size() );
  scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
  scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
  assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

  RiPointsGeneralPolygonsV( numFaces,
                            &nloops[ 0 ],
                            nverts.get(),
                            verts.get(),
                            numTokens,
                            tokenArray.get(),
                            pointerArray.get() );
}

/*
 * Compare this mesh to the other for the purpose of determining
 *  if it's animated.
 */
bool liqRibMeshData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing mesh\n" );
  if ( otherObj.type() != MRT_Mesh ) return false;
  const liqRibMeshData& other = ( liqRibMeshData& )otherObj;
  return compareMesh ( other );
}

/*
 * Compare this mesh to the other for the purpose of determining
 *  if it's animated.
 */
bool liqRibMeshData::compareMesh( const liqRibMeshData & other, bool useNormals ) const
{
  unsigned numFaceVertices( 0 );

  if ( numFaces != other.numFaces )     return false;
  if ( numPoints != other.numPoints )   return false;
  if ( useNormals && numNormals != other.numNormals ) return false;

  for ( unsigned i( 0 ); i < numFaces; ++i ) 
  {
    if ( nverts[i] != other.nverts[i] ) return false;
    numFaceVertices += nverts[i];
  }

  for ( unsigned i( 0 ); i < numFaceVertices; ++i ) 
    if ( verts[i] != other.verts[i] ) 
      return false;

  for ( unsigned i( 0 ); i < numPoints; ++i ) 
  {
    const unsigned a( i * 3 );
    const unsigned b( a + 1 );
    const unsigned c( a + 2 );
    if (  !equiv( vertexParam[a], other.vertexParam[a] ) ||
          !equiv( vertexParam[b], other.vertexParam[b] ) ||
          !equiv( vertexParam[c], other.vertexParam[c] ) )
    {
      return false;
    }
  }

  if ( useNormals )
  {
    for ( unsigned i( 0 ); i < numNormals; ++i ) 
    {
      const unsigned a = i * 3;
      const unsigned b = a + 1;
      const unsigned c = a + 2;
      if (  !equiv( normalParam[a], other.normalParam[a] ) ||
            !equiv( normalParam[b], other.normalParam[b] ) ||
            !equiv( normalParam[c], other.normalParam[c] ) )
      {
        return false;
      }
    }
  }
  return true;
}
/*
 * Return the geometry type.
 */
ObjectType liqRibMeshData::type() const
{
  LIQDEBUGPRINTF( "-> returning mesh type\n" );
  return MRT_Mesh;
}

