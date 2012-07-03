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
** Liquid Node Swatch Header
** ______________________________________________________________________
*/


#ifndef liqNodeSwatch_H
#define liqNodeSwatch_H

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
#include <maya/MSwatchRenderBase.h>
#include <maya/MFnDependencyNode.h>



#define NODE_NAME(OBJ) {cout<<#OBJ <<":"<<MFnDependencyNode(OBJ).name()<<endl;}

class liqNodeSwatch : public MSwatchRenderBase
{
  public:

    liqNodeSwatch(MObject swatchObj, MObject renderObj, int resolution):
      MSwatchRenderBase( swatchObj, renderObj, resolution ) {
        // Init image
        MImage &img = image();
        img.create(resolution,resolution);
		unsigned char *pixels = img.pixels();
		unsigned char * p = pixels;
		for ( unsigned int i=0; i < resolution * resolution; i++ )
		{
			*p = 0; p++;
			*p = 0; p++;
			*p = 0; p++;
			*p = 1; p++;
		}
		//bzero( pixels, resolution * resolution * 4 );
      }

    virtual         ~liqNodeSwatch () {};

    virtual bool    doIteration ();

    static  MSwatchRenderBase *creator( MObject  swatchObj,  MObject  renderObj, int resolution) {
      return new liqNodeSwatch( swatchObj, renderObj,  resolution);
    };

    MString previewPath;
};




#endif
