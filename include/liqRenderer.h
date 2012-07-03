/*
**
** The contents of this file are subject to the Mozilla Public License Version
** 1.1 (the "License"); you may not use this file except in compliance with
** the License.  You may obtain a copy of the License at
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

#ifndef liqRenderer_H
#define liqRenderer_H

#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MObject.h>


class liqRenderer {

public:

  liqRenderer();
  virtual ~liqRenderer();

  void    setRenderer();
  MObject initGlobals();

  // renderer and related utilities
  MString renderName;
  MString renderHome;
  MString renderCommand;
  MString renderPreview;
  MString renderCmdFlags;
  MString shaderExtension;
  MString shaderInfo;
  MString shaderCompiler;
  MString textureMaker;
  MString textureViewer;
  MString textureExtension;

  // optional capabilities
  bool supports_BLOBBIES;
  bool supports_POINTS;
  bool supports_EYESPLITS;
  bool supports_RAYTRACE;
  bool supports_DOF;
  bool supports_ADVANCED_VISIBILITY;
  bool supports_DISPLAY_CHANNELS;

  // pixel filters
  bool pixelfilter_BOX;
  bool pixelfilter_TRIANGLE;
  bool pixelfilter_CATMULLROM;
  bool pixelfilter_GAUSSIAN;
  bool pixelfilter_SINC;
  bool pixelfilter_BLACKMANHARRIS;
  bool pixelfilter_MITCHELL;
  bool pixelfilter_SEPCATMULLROM;
  bool pixelfilter_LANCZOS;
  bool pixelfilter_BESSEL;
  bool pixelfilter_DISK;

  MStringArray pixelFilterNames;

  // hiders
  bool hider_HIDDEN;
  bool hider_PHOTON;
  bool hider_ZBUFFER;
  bool hider_RAYTRACE;
  bool hider_OPENGL;
  bool hider_DEPTHMASK;

  // renderer requirement
  bool requires_SWAPPED_UVS; // transpose u & v direction on NURBS
  bool requires__PREF;       // use __Pref instead of Pref
  bool requires_MAKESHADOW;  // requires MakeShadow to convert zfile to shadow

  // Deep Shadow Display
  MString dshDisplayName;
  MString dshImageMode;
};


#endif
