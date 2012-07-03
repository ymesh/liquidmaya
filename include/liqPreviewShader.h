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

#ifndef liqPreviewShader_H
#define liqPreviewShader_H

/* ______________________________________________________________________
**
** Liquid Get .slo Info Header File
** ______________________________________________________________________
*/

#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MSyntax.h>

enum PrimitiveType {
  SPHERE       = 0,
  CYLINDER     = 1,
  TORUS        = 2,
  CUBE         = 3,
  PLANE        = 4,
  TEAPOT       = 5,
  CUSTOM       = 6
};

class liqPreviewShader : public MPxCommand {
public:
  liqPreviewShader() {};
  virtual ~liqPreviewShader() {};
  static void* creator();

  static MSyntax syntax();
  MStatus doIt( const MArgList& args );

private:
  void resetIt();

};

#endif
