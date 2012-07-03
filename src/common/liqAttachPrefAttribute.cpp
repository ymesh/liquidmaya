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
** The RenderMan (R) Interface Procedures and Protocol are: Copyright 1988,
** 1989, Pixar All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
**
*/

/* ______________________________________________________________________
**
** Liquid Attach __Pref Attribute Source
** ______________________________________________________________________
*/

// Maya's Headers
#include <maya/MSelectionList.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MArgList.h>
#include <maya/MArgParser.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MPlug.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItSurfaceCV.h>
#include <maya/MPointArray.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MDagPath.h>

#include <liqAttachPrefAttribute.h>
#include <liqRenderer.h>


extern liqRenderer liquidRenderer;


liqAttachPrefAttribute::~liqAttachPrefAttribute()
{
  // nothing else needed
}

void* liqAttachPrefAttribute::creator()
{
  return new liqAttachPrefAttribute;
}

MSyntax liqAttachPrefAttribute::syntax()
{
  MSyntax syn;

  syn.setObjectType( MSyntax::kStringObjects, 1 ); // object name
  syn.useSelectionAsDefault( true );

  syn.addFlag("ws", "worldSpace");
  syn.addFlag("en", "exportN");

  return syn;
}

MStatus liqAttachPrefAttribute::doIt(const MArgList& args)
{
  MStatus status;
  MArgParser argParser(syntax(), args);

  MString tempStr;
  status = argParser.getObjects(objectNames);
  if ( !status ) 
  {
    MGlobal::displayError("error parsing object list");
    return MS::kFailure;
  }

  worldSpace = false;
  int flagIndex = args.flagIndex("ws", "worldSpace");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
    worldSpace = true;

  exportN = false;
  flagIndex = args.flagIndex("en", "exportN");
  if ( flagIndex != MArgList::kInvalidArgIndex ) 
    exportN = true;

  //cout <<">> got "<<objectNames.length()<<" objects to PREF !"<<endl;

  return redoIt();
}

MStatus	liqAttachPrefAttribute::redoIt()
{
  MFnTypedAttribute tAttr;
  MStatus status;

  for ( unsigned i( 0 ); i < objectNames.length(); i++ ) 
  {
    MSelectionList		nodeList;
    nodeList.add( objectNames[i] );
    MObject depNodeObj;
    nodeList.getDependNode( 0, depNodeObj );
    MDagPath dagNode;
    nodeList.getDagPath( 0, dagNode );
    MFnDependencyNode depNode( depNodeObj );
    MObject prefAttr;
    MString attrName, varName;

    // make sure the renderer description is up to date
    liquidRenderer.setRenderer();

    // build the name of the attribute
    varName = ( ( exportN && depNodeObj.hasFn( MFn::kMesh ) )? "N":"P" );
    attrName = "rman";
    attrName += varName;
    attrName += ( ( liquidRenderer.requires__PREF )? "__":"" );
    attrName += varName + "ref";

    // create the attribute
    prefAttr = tAttr.create( attrName, attrName, MFnData::kPointArray );


    if ( depNodeObj.hasFn( MFn::kNurbsSurface ) ) 
    {
      MFnNurbsSurface nodeFn( depNodeObj );
      MPointArray nodePArray;
      MItSurfaceCV cvs( dagNode, MObject::kNullObj, liquidRenderer.requires_SWAPPED_UVS == false, &status );

      while ( !cvs.isDone() ) 
      {
        while ( !cvs.isRowDone() ) 
        {
          MPoint pt = (worldSpace)? cvs.position( MSpace::kWorld ) : cvs.position( MSpace::kObject );
          nodePArray.append( pt );
          cvs.next();
        }
        cvs.nextRow();
      }

      nodeFn.addAttribute( prefAttr );
      MFnPointArrayData pArrayData;

      MObject prefDefault = pArrayData.create( nodePArray );
      MPlug nodePlug( depNodeObj, prefAttr );
      nodePlug.setValue( prefDefault );
    } 
    else if ( depNodeObj.hasFn( MFn::kNurbsCurve ) ) 
    {
      // Carsten: added support for PREF on nurbs curves
      //
      MFnNurbsCurve nodeFn( depNodeObj );
      MPointArray nodePArray;
      nodeFn.getCVs( nodePArray );

      nodeFn.addAttribute( prefAttr );
      MFnPointArrayData pArrayData;

      MObject prefDefault = pArrayData.create( nodePArray );
      MPlug nodePlug( depNodeObj, prefAttr );
      nodePlug.setValue( prefDefault );
    } 
    else if ( depNodeObj.hasFn( MFn::kMesh ) ) 
    {
      MFnMesh nodeFn( depNodeObj );
      // Moritz: modified this line to dim nodePArray -- otherwise
      // nodePArray.set() in the wile loop below throws an exception
      // which was why __Pref didn't work
      MPointArray nodePArray( MFnMesh( depNodeObj ).numVertices() );
      unsigned count;

      nodeFn.addAttribute( prefAttr );

      if ( exportN ) 
      {
        // export Nref
        unsigned vertex;
        unsigned normal;
        unsigned face = 0;
        unsigned faceVertex = 0;
        unsigned int numNormals = nodeFn.numNormals();
        unsigned int numPoints  = nodeFn.numVertices();
        MFloatVectorArray normals;
        MVectorArray normalAttArray;
        nodeFn.getNormals( normals );

        if ( numNormals > numPoints ) 
        {
          // if we get more than 1 normal per vertex,
          // force the arraysize to the full facevarying size
          unsigned faceVaryingCount( 0 );
          for ( unsigned pOn( 0 ); pOn < nodeFn.numPolygons(); pOn++ ) 
            faceVaryingCount += nodeFn.polygonVertexCount( pOn );
          
          normalAttArray.setLength( faceVaryingCount );
        } 
        else 
          normalAttArray.setLength(normals.length());
        
        for ( MItMeshPolygon polyIt ( depNodeObj ); polyIt.isDone() == false; polyIt.next() ) 
        {
          count = polyIt.polygonVertexCount();
          while ( count > 0 ) 
          {
            --count;
            normal = polyIt.normalIndex( count );
            vertex = polyIt.vertexIndex( count );

            if ( numNormals == numPoints )
              normalAttArray.set(normals[normal], vertex);
            else
              normalAttArray.set(normals[normal], faceVertex);

            ++faceVertex;
          }
          ++face;
        }

        MFnVectorArrayData pArrayData;
        MObject prefDefault = pArrayData.create( normalAttArray );
        MPlug nodePlug( depNodeObj, prefAttr );
        nodePlug.setValue( prefDefault );

      } 
      else 
      {
        // TODO: do we need to account for the altMeshExport algo that's
        // used in liquidRibMeshData?
        // Moritz: no, it's basically the same as the algo below
        for ( MItMeshPolygon polyIt( dagNode, MObject::kNullObj ); !polyIt.isDone(); polyIt.next()) 
        {
          count = polyIt.polygonVertexCount();

          while ( count > 0 ) 
          {
            --count;
            unsigned	vertexIndex = polyIt.vertexIndex( count );
            MPoint nodePoint = (worldSpace)? polyIt.point( count, MSpace::kWorld ) : polyIt.point( count, MSpace::kObject );
            // Moritz: this returns MS::kFailure but seems to work?!
            nodePArray.set( nodePoint, vertexIndex );
          }
        }

        MFnPointArrayData pArrayData;
        MObject prefDefault = pArrayData.create( nodePArray );
        MPlug nodePlug( depNodeObj, prefAttr );
        nodePlug.setValue( prefDefault );

      }
    } else cerr << "Neither a Nurbs nor a Mesh !!" << endl;
  }
  return MS::kSuccess;
}
