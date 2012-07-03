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

#ifndef __LIQ_GENERIC_SHADER_H__
#define __LIQ_GENERIC_SHADER_H__


#include <liquid.h>
#include <ri.h>
#include <maya/MString.h>
#include <liqGetSloInfo.h>
#include <string>

class liqShader;
class liqSwitcher;

using namespace std;

class liqGenericShader
{
public:
	liqGenericShader();
	liqGenericShader(const liqGenericShader &src);
	liqGenericShader(MObject shaderObj, bool outputAllParameters );
	
	virtual ~liqGenericShader();
	virtual void *write(bool shortShaderNames, unsigned int indentLevel, SHADER_TYPE forceAs=SHADER_TYPE_UNKNOWN) = 0;

	virtual bool isShader() = 0;
	virtual bool isSwitcher() = 0;
	virtual liqShader* asShader() = 0;
	virtual liqSwitcher* asSwitcher() = 0;
	virtual RtColor& getColor();
	virtual RtColor& getOpacity();

public:
	string name;
	MString shaderHandler;
	MObject m_mObject;
	bool hasErrors;
	bool outputInShadow;
	bool m_outputAllParameters;     // allow to write all shader arguments even if they are on default value
	RtColor rmColor;
	RtColor rmOpacity;
};

#endif
