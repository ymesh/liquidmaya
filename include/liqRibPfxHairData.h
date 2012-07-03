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
** Contributor(s): philippe leprince.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar

*/

#ifndef liqRibPfxHairData_H
#define liqRibPfxHairData_H

/* ______________________________________________________________________
**
** Liquid pfxToon Node Data Header File
** ______________________________________________________________________
*/

#include <boost/shared_array.hpp>

class liqRibPfxHairData : public liqRibData {
public: // Methods

    liqRibPfxHairData( MObject curve );

    virtual void       write();
    virtual bool       compare( const liqRibData& other ) const;
    virtual ObjectType type() const;

private: // Data

    RtInt ncurves;
    shared_array< RtInt >   nverts;
    shared_array< RtFloat > CVs;
    shared_array< RtFloat > normals;
    shared_array< RtFloat > curveWidth;
    shared_array< RtFloat > cvColor;
    shared_array< RtFloat > cvOpacity;
};

#endif
