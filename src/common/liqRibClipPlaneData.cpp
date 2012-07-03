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
** Liquid Rib Locator Data Source
** ______________________________________________________________________
*/

// Renderman Headers
extern "C" {
  #include <ri.h>
}

#include <maya/MFnDependencyNode.h>

#include <liquid.h>
#include <liqRibClipPlaneData.h>

extern int debugMode;


/** Create a RIB compatible representation of a Maya coordinate system.
 */
liqRibClipPlaneData::liqRibClipPlaneData( MObject coord )
{
  LIQDEBUGPRINTF("-> creating clipPlane\n");
  MFnDependencyNode fnNode( coord );
  this->name = fnNode.name();
  //cout <<"created clipPlane "<<this->name.asChar()<<endl;
}

/** Write the RIB for this coordinate system.
 */
void liqRibClipPlaneData::write()
{
  LIQDEBUGPRINTF("-> writing clipPlane");
  RiArchiveRecord( RI_VERBATIM, "ClippingPlane 0 0 -1 0 0 0\n" );
}

/** Compare this coordinate system to otherObj.
 *  The purpose is usually to determe if the coordinate system is animated.
 */
bool liqRibClipPlaneData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF("-> comparing clipPlane\n"); 
  return ( otherObj.type() != MRT_Coord )? false : true;
}

/** Return the geometry type.
 */
ObjectType liqRibClipPlaneData::type() const
{
  LIQDEBUGPRINTF("-> returning clipPlane type\n"); 
  return MRT_Coord;
}
