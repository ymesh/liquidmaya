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

#ifndef liqRibHierarchicalSubdivisionData_H
#define liqRibHierarchicalSubdivisionData_H

/* ______________________________________________________________________
**
** Liquid Rib Mesh Data Header File
** ______________________________________________________________________
*/
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>

#include <vector>
//#include <liqSubdivStructs.h>
#include <liqRibData.h>
#include <liqRibSubdivisionData.h>
#include <boost/shared_array.hpp>

using namespace boost;

class liqRibHierarchicalSubdivisionData : public virtual liqRibSubdivisionData
{
public: // Methods
  liqRibHierarchicalSubdivisionData();
	liqRibHierarchicalSubdivisionData( MObject mesh, bool initSubdivData = true );
//	virtual ~liqRibHierarchicalSubdivisionData();

	virtual bool       getMayaData( MObject mesh, bool useNormals ){ return liqRibMeshData::getMayaData( mesh, useNormals ); }
	
	virtual void       write();
	virtual bool       compare( const liqRibData & other ) const;
	virtual ObjectType type() const;
	
  std::vector <RtString>  v_stringargs;	

  //virtual void checkExtraTags( MObject &mesh );
  
  //virtual void addExtraTagsFromMaya( MObject &mesh );
  //virtual void addExtraTagsFromSets( MObject &mesh );
 
  virtual void addExtraTag( int intValue, SBD_EXTRA_TAG extraTag );
  virtual void addCornerTag( int intValue, float floatValue );
  virtual void addCreaseTag( int intValue1, int intValue2, float floatValue );
  virtual void addHoleTag( MItMeshPolygon &faceIter );
  virtual void addStitchTag( MItMeshVertex &vertexIter, int intTagValue );
  void addExtraTag( const char *stringValue, SBD_EXTRA_TAG extraTag );
  
private: // Data
  void initializeSubdivParameters();
};

#endif
