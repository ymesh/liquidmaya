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
** Liquid Surface Shader Node Header
** ______________________________________________________________________
*/

#ifndef liqSwitcherNode_H
#define liqSwitcherNode_H

#include <liquid.h>

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MImage.h>
#include <maya/MFnDependencyNode.h>


class liqSwitcherNode : public MPxNode
{
 public:
    liqSwitcherNode();
    virtual ~liqSwitcherNode();
    virtual MStatus compute( const MPlug&, MDataBlock& );
    virtual void postConstructor();
    static void *creator();
    static MStatus initialize();
    static MTypeId id;
	bool isAbstractClass() {return true;}

 protected :
	static MObject mAcceptedConnectionNodeType;
 private:
	static MObject mDefaultShader;
	static MObject mSwitchShaders;
	static MObject mUserAttribute;
	static MObject mCustomUserAttribute;
	static MObject mCondition;
	static MObject mUserValue;
	static MObject mShader;
};




#endif
