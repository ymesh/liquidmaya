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
** Contributor(s): Baptiste Sansierra.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#ifndef liqRibShaderTranslator_H
#define liqRibShaderTranslator_H

/* ______________________________________________________________________
**
** Liquid Rib Shader Translator Header File
** ______________________________________________________________________
*/

#include <liquid.h>
#include <liqRenderer.h>
#include <liqRibHT.h>
#include <liqShader.h>
#include <liqRenderScript.h>
#include <liqRibLightData.h>
#include <liqExpression.h>

#include <maya/MPxCommand.h>
#include <maya/MDagPathArray.h>
#include <maya/MFnCamera.h>
#include <maya/MArgList.h>
#include <maya/MFloatArray.h>

#include <map>
#include <boost/shared_ptr.hpp>


using namespace std;

class liqRibShaderTranslator : public MPxCommand
{
public:
	liqRibShaderTranslator();
	~liqRibShaderTranslator();
	static void * creator();
	//static MSyntax syntax();
	MStatus doIt(const MArgList& args );
private: // Methods
};

#endif
