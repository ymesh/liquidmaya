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
#include <liqAreaLightData.h>

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

/** Create a RIB compatible representation of a Maya polygon mesh.
 */
liqAreaLightData::liqAreaLightData( MObject mesh )
: liqRibMeshData( mesh )
{
  LIQDEBUGPRINTF( "-> creating Area Light\n" );
  MFnMesh fnMesh( mesh );
  objDagPath = fnMesh.dagPath();
  MStatus astatus;
    
  // areaLight = true;
  liquidGetPlugValue( fnMesh, "areaIntensity", areaIntensity, astatus ); 
      
  MTransformationMatrix worldMatrix = objDagPath.inclusiveMatrix();
  MMatrix worldMatrixM = worldMatrix.asMatrix();
  worldMatrixM.get( transformationMatrix );
}

/**      Write the RIB for this mesh.
 */
void liqAreaLightData::write()
{
  LIQDEBUGPRINTF( "-> writing Area Light\n" );
  RtLightHandle handle = NULL;

  // What happens if we're inside a motion block????? This whole approach of Liquid is flawed...
  //	RiAttributeBegin();
  RtString ribname = const_cast< char* >( get_name() );
  RiAttribute( "identifier", "name", &ribname, RI_NULL );
  RtMatrix tmp;
  memcpy( tmp, transformationMatrix, sizeof( RtMatrix ) );
  RiTransform( tmp );

  handle = RiAreaLightSource( "arealight", "intensity", &areaIntensity, RI_NULL );

  writeMesh();
  
  RiIlluminate( handle, 1 );
}

/** Compare this mesh to the other for the purpose of determining
 *  if it's animated.
 */
bool liqAreaLightData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing Area light mesh\n" );
  if ( otherObj.type() != MRT_Light ) return false;
  const liqRibMeshData& other = (liqRibMeshData&)otherObj;
  return compareMesh ( other );
}

/** Return the geometry type.
 */
ObjectType liqAreaLightData::type() const
{
  LIQDEBUGPRINTF( "-> returning area light type\n" );
  return MRT_Light;
}

