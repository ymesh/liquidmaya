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
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#ifndef liqCustomNode_H
#define liqCustomNode_H

/* ______________________________________________________________________
** 
** Liquid liqCustomNode Header File
** ______________________________________________________________________
*/

#include <maya/MDagPath.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>

#include <liquid.h>
#include <liqRibData.h>


// This is a virtual base class that custom Maya nodes can inherit
// from to be compatible with Liquid for RIB output.
//
// All you need to do is inherit from this class in your custom 
// Maya node (using multiple inheritance so you're still subclassing
// from Maya's MPxNode) implementing the pure virtual functions and 
// then Liquid will take care of the rest. 
//
// You will also need to link your plugin with liquid.so when 
// compiling (the same binary you use for rendering) or you will get 
// missing symbols when loading into Maya.

class liqCustomNode {
public:
  virtual ~liqCustomNode() {}
  virtual void liquidWrite() = 0;
  virtual bool liquidCompare(const liqRibData &other) = 0;
};


#endif // liqCustomNode_H
