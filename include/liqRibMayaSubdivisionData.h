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

#ifndef liqRibMayaSubdivisionData_H
#define liqRibMayaSubdivisionData_H

/* ______________________________________________________________________
**
** Liquid Rib Maya Subdivision Data Header File
** ______________________________________________________________________
*/

#include <maya/MUint64Array.h>
#include <vector>
#include <liqRibData.h>

#include <boost/shared_array.hpp>

using namespace boost;

typedef struct tagMayaPolyEdgeIndx {
    RtInt	vtx0;
    RtInt	vtx1;
} PolyMayaEdgeIndx;

typedef RtInt PolyMayaVertexIndx;
typedef RtInt PolyMayaFaceIndx;

typedef struct tagMayaSbdExtraTag {
    RtFloat	value;		// hardness for creases and corners
    RtInt	length;		// number of elements
    union tagExtraData {
        PolyMayaEdgeIndx	*edges;
        PolyMayaVertexIndx	*vertices;
        PolyMayaFaceIndx	*faces;
    } ExtraData;
} MayaSbdExtraTag;

class liqRibMayaSubdivisionData : public liqRibData {
public: // Methods
  liqRibMayaSubdivisionData( MObject mesh );

  virtual void       write();
  virtual bool       compare( const liqRibData & other ) const;
  virtual ObjectType type() const;

private: // Data
  RtInt     numFaces;
  RtInt     numPoints;
  shared_array< RtInt > nverts;
  shared_array< RtInt > verts;
  const RtFloat* vertexParam;

  DetailType uvDetail;
  bool trueFacevarying;

  MString   name;
  MString   longName;
  RtMatrix  transformationMatrix;

  int interpolateBoundary; // Now an integer from PRMan 12/3Delight 6

  std::vector <RtToken> v_tags;
  std::vector <RtInt>   v_nargs;
  std::vector <RtInt>   v_intargs;
  std::vector <RtFloat> v_floatargs;

  void checkExtraTags( MObject &mesh );
  void addExtraTags( MObject &mesh, SBD_EXTRA_TAG extraTag );
  void addExtraTags( MObject &mesh, int extraTagValue, SBD_EXTRA_TAG extraTag );
};

#endif
