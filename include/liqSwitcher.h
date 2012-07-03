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

#ifndef __LIQ_SWITCHER_H__
#define __LIQ_SWITCHER_H__


#include <liqGenericShader.h>

#include <vector>

class liqShader;

class liqSwitcher : public liqGenericShader
{
public :
    liqSwitcher();
    liqSwitcher( const liqSwitcher & src );
    liqSwitcher & operator=( const liqSwitcher & src );
    liqSwitcher ( MObject shaderObj, bool outputAllParameters=false );
    virtual ~liqSwitcher();

	void *write(bool shortShaderNames, unsigned int indentLevel, SHADER_TYPE forceAs=SHADER_TYPE_UNKNOWN);

	virtual bool isShader();
	virtual bool isSwitcher();
	virtual liqShader* asShader();
	virtual liqSwitcher* asSwitcher();
	virtual RtColor& getColor();
	virtual RtColor& getOpacity();

private :
	MObject m_defaultShader;
	vector<MString> m_userAttributes;
	vector<MString> m_customUserAttributes;
	vector<MString> m_conditions;
	vector<MString> m_userValues;
	vector<MObject> m_shaders;
};


#endif // __LIQ_SWITCHER_H__
