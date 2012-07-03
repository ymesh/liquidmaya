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


#include <liqShaderFactory.h>

#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>

#include <liqShader.h>
#include <liqSwitcher.h>
#include <liqMayaNodeIds.h>

#include <liqSurfaceNode.h>
#include <liqDisplacementNode.h>
#include <liqVolumeNode.h>
#include <liqLightNode.h>
#include <liqCoShaderNode.h>


liqShaderFactory * liqShaderFactory::_instance=NULL;


liqShaderFactory::liqShaderFactory()
{
	shaderHandlerId = 0;
}


liqShaderFactory::~liqShaderFactory()
{
	clearShaders();
}


void liqShaderFactory::clearShaders()
{
	vector<liqGenericShader*>::iterator iter;
	vector<liqGenericShader*>::iterator end = m_shaders.end();
	for( iter=m_shaders.begin(); iter!=end; iter++ )
	{
		if( *iter )
		{
			delete (*iter);
		}
	}
	m_shaders.clear();
	shaderHandlerId = 0;
}


liqGenericShader &liqShaderFactory::getShader( MObject shaderObj, bool withAllParameters )
{
	MString rmShaderStr;
	MFnDependencyNode shaderNode( shaderObj );
	MPlug rmanShaderNamePlug = shaderNode.findPlug( MString( "rmanShaderLong" ) );
	rmanShaderNamePlug.getValue( rmShaderStr );
	LIQDEBUGPRINTF( "-> Using Renderman Shader %s\n",  rmShaderStr.asChar() );
  
	vector<liqGenericShader*>::iterator iter = m_shaders.begin();
	while ( iter != m_shaders.end() )
	{
		//string shaderNodeName = shaderNode.name().asChar();
		if ( (*iter)->m_mObject == shaderObj )
		{
			// Already got it : nothing to do
			return **iter;
		}
		++iter;
	}
	liqGenericShader *currentShader = NULL;

	MTypeId typeId = shaderNode.typeId();

	if(	typeId==liqSurfaceNode::id ||
		typeId==liqDisplacementNode::id ||
		typeId==liqVolumeNode::id ||
		typeId==liqLightNode::id || 
		typeId==liqCoShaderNode::id
		)	// classic shader
	{
		currentShader = new liqShader( shaderObj, withAllParameters );
	}
	else	// switcher
	{
		currentShader = new liqSwitcher( shaderObj, withAllParameters );
	}
	if( currentShader->hasErrors )
	{
		printf("[liqShaderFactory] error while creating liqObject for node '%s'\n", shaderNode.name().asChar() );
	}
	m_shaders.push_back( currentShader );
	//fflush(stdout);
	//fflush(stderr);
	return *(m_shaders.back());
}


MString liqShaderFactory::getShaderId( MObject shaderObj )
{
	liqGenericShader &shader = liqShaderFactory::getShader( shaderObj );
	return shader.shaderHandler;
}


MString liqShaderFactory::getUniqueShaderHandler()
{
	shaderHandlerId++;
	char shaderHandler[512];
	sprintf(shaderHandler, "CO_SHADER_%d", shaderHandlerId);
	return shaderHandler;
}


