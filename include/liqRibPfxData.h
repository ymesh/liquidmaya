/**
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Liquid Rendering Toolkit.
 *
 * The Initial Developer of the Original Code is Colin Doncaster. Portions
 * created by Colin Doncaster are Copyright (C) 2002. All Rights Reserved.
 *
 * Contributor(s): Berj Bannayan.
 *
 *
 * The RenderMan (R) Interface Procedures and Protocol are:
 * Copyright 1988, 1989, Pixar
 * All Rights Reserved
 *
 *
 * RenderMan (R) is a registered trademark of Pixar
 */

#ifndef liqRibPfxData_H
#define liqRibPfxData_H

/* ______________________________________________________________________
**
** Liquid Rib Paint Effects Data Header File
** ______________________________________________________________________
*/

#include <maya/MPlug.h>

#include <boost/shared_array.hpp>

class liqRibPfxData : public liqRibData {
public: // Methods

                          liqRibPfxData( MObject pfxGeo, ObjectType pfxtype );

  virtual void            write();
  virtual unsigned        granularity() const;
  virtual bool            writeNextGrain();
  virtual bool            compare( const liqRibData& other ) const;
  virtual ObjectType      type() const;

private: // Data

  unsigned                grain;
  bool                    hasFeature[ 3 ];
  ObjectType			  pfxtype;
  vector< RtInt >         nverts[ 3 ];
  shared_array< RtFloat > CVs;
  liqTokenPointer::array pfxTokenPointerArrays[ 3 ];
};

#endif // liqRibPfxData_H
