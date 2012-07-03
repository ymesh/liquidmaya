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

#ifndef liqRibCustomNode_H
#define liqRibCustomNode_H

/* ______________________________________________________________________
**
** Liquid liqRibCustomNode Header File
** ______________________________________________________________________
*/

#include <maya/MDagPath.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>

#include <liquid.h>
#include <liqRibData.h>
#include <liqCustomNode.h>


// This class represents a custom node that has been added to Maya through
// the Maya API, but has also been written with the Liquid API.
//
// Each liqRibCustomNode object contains a pointer to the corresponding
// custom node object and passes the write()/compare() requests on to it.

class liqRibCustomNode : public liqRibData {
public:
  liqRibCustomNode(MObject /*obj*/, liqCustomNode *node)
    : customNode(node)
  {
    if ( debugMode ) { printf("-> creating custom node object\n"); }
  }

  virtual void write()
  {
    if (customNode) {
      customNode->liquidWrite();
    }
  }

  virtual bool compare( const liqRibData & other ) const
  {
    if (customNode) {
      return customNode->liquidCompare(other);
    }
    return false;
  }

  virtual ObjectType type() const
  {
    if ( debugMode ) { printf("-> returning customnode object type\n"); }
    return MRT_Custom;
  }

private:
  liqCustomNode *customNode;
};


#endif // liqRibCustomNode_H
