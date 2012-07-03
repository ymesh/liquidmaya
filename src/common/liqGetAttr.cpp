/*
**
** The contents of this file are subject to the Mozilla Public License Version 1.1 (the
** "License"); you may not use this file except in compliance with the License. You may
** obtain a copy of the License at http://www.mozilla.org/MPL/
**
** Software distributed under the License is distributed on an "AS IS" basis, WITHOUT
** WARRANTY OF ANY KIND, either express or implied. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Code is the Liquid Rendering Toolkit.
**
** The Initial Developer of the Original Code is Colin Doncaster. Portions created by
** Colin Doncaster are Copyright (C) 2002. All Rights Reserved.
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
** Liquid Get Attribute Source
** ______________________________________________________________________
*/

// Maya's Headers
#include <maya/MPxCommand.h>
#include <maya/MCommandResult.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MPlug.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MArgList.h>
#include <maya/MString.h>
#include <maya/MSelectionList.h>

#include <liqGetAttr.h>

void* liqGetAttr::creator()
//
//  Description:
//      Create a new instance of the translator
//
{
    return new liqGetAttr();
}

liqGetAttr::~liqGetAttr()
//
//  Description:
//      Class destructor
//
{
}

MStatus	liqGetAttr::doIt( const MArgList& args )
{

	MStatus					status;
	unsigned				i;
	MString					nodeName, attrName;
	MSelectionList				nodeList;

	for ( i = 0; i < args.length(); i++ ) 
  {
		if ( MString( "-debug" ) == args.asString( i, &status) ) 
    {
		} 
    else if ( MString( "-node" ) == args.asString( i, &status) ) 
    {
			i++;
			nodeName = args.asString( i, &status ) ;
		} 
    else if ( MString( "-attr" ) == args.asString( i, &status) ) 
    {
			i++;
			attrName = args.asString( i, &status );
		}
	}
	nodeList.add( nodeName );
	MObject depNodeObj;
	nodeList.getDependNode(0, depNodeObj);
	MFnDependencyNode depNode( depNodeObj );
	MPlug attrPlug = depNode.findPlug( attrName );
	MObject plugObj;
	attrPlug.getValue( plugObj );
	if ( plugObj.apiType() == MFn::kDoubleArrayData ) 
  {
	  MFnDoubleArrayData fnDoubleArrayData( plugObj );
	  const MDoubleArray& doubleArrayData( fnDoubleArrayData.array( &status ) );
	  for ( i = 0; i < doubleArrayData.length(); i++ ) 
		  appendToResult( doubleArrayData[i] );
	}
	return MS::kSuccess;
};
