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

#ifndef liqRibObject_H
#define liqRibObject_H

/* ______________________________________________________________________
**
** Liquid Rib Object Header File
** ______________________________________________________________________
*/

#include <liqRibData.h>
#include <boost/shared_ptr.hpp>

using namespace boost;

class liqRibObj;
typedef boost::shared_ptr< liqRibObj > liqRibObjPtr;

class liqRibObj {
public:
             liqRibObj( const MDagPath &, ObjectType objType );

    AnimType compareMatrix( const liqRibObjPtr, int instance ) const;
    AnimType compareBody( const liqRibObjPtr ) const;
    void     writeObject() const; // write geometry directly
    unsigned granularity() const; // get granularity
    bool     writeNextObjectGrain() const; // write next geometry grain directly
    bool     isNextObjectGrainAnimated() const; // whether the next grain needs to be in a motion block

    int      type;
    int      written;
    bool     ignore;
    bool     ignoreShadow;
    bool     receiveShadow;
    bool     ignoreShapes;

    MMatrix  matrix( int instance ) const;
    void     setMatrix( int instance, MMatrix matrix );
    void     ref();
    void     unref();

    void     setHandle( RtObjectHandle handle );

    RtObjectHandle handle() const;
    RtLightHandle  lightHandle() const;
    //char         **lightSources; // unused?

private:
    vector< MMatrix > instanceMatrices; // Matrices for all instances of this object
    RtObjectHandle    objectHandle;     // Handle used by RenderMan to refer to defined geometry
    int               referenceCount;   // Object's reference count
    liqRibDataPtr     data;             // Geometry or light data
};

#endif
