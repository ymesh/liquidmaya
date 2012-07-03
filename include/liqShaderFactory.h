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

#ifndef __LIQ_SHADER_FACTORY_H__
#define __LIQ_SHADER_FACTORY_H__


#include <liquid.h>
//#include <liqShader.h>
#include <liqGenericShader.h>
#include <maya/MString.h>


class liqShaderFactory
{
public:
    inline static liqShaderFactory &instance()
    {
        if( !_instance )
            _instance = new liqShaderFactory();
        return *_instance;
    }
    inline static void deleteInstance()
    {
        if( _instance )
            delete _instance;
    }
    
	virtual ~liqShaderFactory();

	liqGenericShader &getShader(MObject shaderObj, bool withAllParameters = false);
	MString getShaderId( MObject shaderObj );

	MString getUniqueShaderHandler();

	void clearShaders();

	//inline void setBuildShadersWithAllParameters(bool b){buildShadersWithAllParameters = b;}
private:
	liqShaderFactory();
	static liqShaderFactory *_instance;
	int shaderHandlerId;
	vector<liqGenericShader*> m_shaders;
	//bool buildShadersWithAllParameters;
};

#endif
