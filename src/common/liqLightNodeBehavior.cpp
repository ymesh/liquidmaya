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
** Contributor(s): Philippe Leprince.
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
** Liquid Light Node Drag and Drop Behaviour Source
** ______________________________________________________________________
*/


#include <liqLightNodeBehavior.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDagNode.h>
#include <maya/MObjectArray.h>

liqLightNodeBehavior::liqLightNodeBehavior()
{
}

liqLightNodeBehavior::~liqLightNodeBehavior()
{
}

void *liqLightNodeBehavior::creator()
{
  return new liqLightNodeBehavior;
}

/*	Overloaded function from MPxDragAndDropBehavior
 *	this method will return true if it is going to handle the connection
 *	between the two nodes given.
 */

bool liqLightNodeBehavior::shouldBeUsedFor( MObject &sourceNode, MObject &destinationNode, MPlug &sourcePlug, MPlug &destinationPlug)
{
  bool result = false;

  if ( MFnDependencyNode(sourceNode).typeName() == "liquidLight" ) 
  {
    if ( destinationNode.hasFn(MFn::kLight ) ) 
    {
      //cout <<"dropping on a light !"<<endl;
      result = true;
    }
  }
  return result;
}

MStatus liqLightNodeBehavior::connectNodeToNode( MObject &sourceNode, MObject &destinationNode, bool force )
{
  MStatus result = MS::kFailure;
  MFnDependencyNode src(sourceNode);

  if (src.typeName() == "liquidLight")
  {
    // if we are dragging from a liquidLight
    // than we want to see what we are dragging onto 
    if ( true ) //destinationNode.hasFn(MFn::kLight))
    {
      /* if the user is dragging onto a light
      then make the connection from the worldMesh
      to the dirtyShader plug on the slopeShader */

      MFnDependencyNode dest(destinationNode);
      MPlug srcPlug = src.findPlug("assignedObjects");
      MPlug destPlug = dest.findPlug("liquidLightShaderNode");

      if ( destPlug.isNull() ) 
      {
        /* the attribute does not exist yet : create it ! */
        //cout <<"need to create liquidLightShaderNode attribute on "<<dest.name()<<"..."<<endl;
        MString mel = "addAttr -at message -ln liquidLightShaderNode " + dest.name() +";";
        result = MGlobal::executeCommand(mel);
        if ( result == MS::kSuccess ) 
        {
          //cout <<"attribute liquidLightShaderNode successfully created"<<endl;
          result = MS::kFailure;
          destPlug = dest.findPlug("liquidLightShaderNode");
        }
      }

      if ( !srcPlug.isNull() && !destPlug.isNull() )
      {
        MString boolean = (force)? "true":"false";
        MString cmd = "connectAttr ";
        //cmd += "-force " + boolean + " ";
        cmd += srcPlug.name() + " ";
        cmd += destPlug.name();
        result = MGlobal::executeCommand(cmd);
      }
    }
  }
  //if ( result == MS::kSuccess ) cout <<"connection successfull"<<endl;
  //else cout <<"connection failed !"<<endl;
  return result;
}

/*  Overloaded function from MPxDragAndDropBehavior
 *  this method will assign the correct output from the liquidLight
 *  onto the given attribute.
 */

MStatus liqLightNodeBehavior::connectNodeToAttr( MObject &sourceNode, MPlug &destinationPlug, bool force )
{
  MStatus result = MS::kFailure;
  MFnDependencyNode src(sourceNode);

  /*
  if we are dragging from a liquidLight
  to a light then connect the assignedObjects
  plug to the plug being passed in
  */

  if ( destinationPlug.node().hasFn(MFn::kLight) ) 
  {
    if ( src.typeName() == "liquidLight" ) 
    {
      MPlug srcPlug = src.findPlug( "assignedObjects" );

      MObject dstNode = destinationPlug.node();
      MFnDependencyNode dst( dstNode );
      MPlug dstPlug = dst.findPlug( "liquidLightShaderNode" );

      if ( !srcPlug.isNull() && !destinationPlug.isNull() && destinationPlug == dstPlug ) 
      {
        //MString boolean = (force)? "true":"false";
        MString cmd = "connectAttr ";
        //cmd += "-force " + boolean + " ";
        cmd += srcPlug.name() + " ";
        cmd += destinationPlug.name();
        result = MGlobal::executeCommand(cmd);
      }
    }
  } 
  else 
  {
    /* in all of the other cases we do not need the plug just the node
    that it is on                                                    */
    MObject destinationNode = destinationPlug.node();
    result = connectNodeToNode( sourceNode, destinationNode, force );
  }

  return result;
}
