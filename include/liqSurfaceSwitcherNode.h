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


#ifndef __LIQ_SURFACE_SWITCHER_NODE_H__
#define __LIQ_SURFACE_SWITCHER_NODE_H__

#include <liquid.h>
#include <liqSwitcherNode.h>


class liqSurfaceSwitcherNode : public liqSwitcherNode
{
 public:
                      liqSurfaceSwitcherNode();
    virtual          ~liqSurfaceSwitcherNode();
    virtual MStatus   compute( const MPlug&, MDataBlock& );
    static  void *    creator();
    static  MStatus   initialize();
    static  MTypeId   id;
	bool isAbstractClass(){return false;}

 private:
	// maya shader attributes
    static MObject aColor;
    static MObject aOpacity;
    static MObject aTransparency; // Needed to get Open GL transparency updated in "5" mode
    static MObject aMayaIgnoreLights;
    static MObject aMayaKa;
    static MObject aMayaKd;
    static MObject aNormalCameraX;
    static MObject aNormalCameraY;
    static MObject aNormalCameraZ;
    static MObject aNormalCamera;
    static MObject aLightDirectionX;
    static MObject aLightDirectionY;
    static MObject aLightDirectionZ;
    static MObject aLightDirection;
    static MObject aLightIntensityR;
    static MObject aLightIntensityG;
    static MObject aLightIntensityB;
    static MObject aLightIntensity;
    static MObject aLightAmbient;
    static MObject aLightDiffuse;
    static MObject aLightSpecular;
    static MObject aLightShadowFraction;
    static MObject aPreShadowIntensity;
    static MObject aLightBlindData;
    static MObject aLightData;
    // Output attributes
    static MObject aOutColor;
    static MObject aOutTransparency;
};

#endif

