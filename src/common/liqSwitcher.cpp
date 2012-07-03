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

#include <liqSwitcher.h>
#include <liqShaderFactory.h>

#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MStatus.h>
#include <maya/MFnEnumAttribute.h>

extern int debugMode;


liqSwitcher::liqSwitcher() : liqGenericShader()
{
}

liqSwitcher::liqSwitcher( const liqSwitcher& src ) : liqGenericShader((const liqGenericShader&)src)
{
}


liqSwitcher & liqSwitcher::operator=( const liqSwitcher &)
{
	return *this;
}


liqSwitcher::liqSwitcher( MObject shaderObj, bool outputAllParameters ) : liqGenericShader(shaderObj, outputAllParameters)
{
	int i;
	char tmp[512];
	MStatus status;
	MFnDependencyNode shaderNode( shaderObj, &status );
	if( status != MS::kSuccess )
	{
		hasErrors = true;
		return;
	}
	MPlugArray plugArray;
	// get accepted type
	MPlug acceptedTypePlug = shaderNode.findPlug( MString( "acceptedType" ) );
	unsigned int acceptedType = acceptedTypePlug.asInt();
	// get default shader
	MPlug defaultShaderPlug = shaderNode.findPlug( MString( "defaultShader" ) );
	defaultShaderPlug.connectedTo(plugArray, 1, 0, &status);
	if( plugArray.length() > 0 )
	{
		MObject connectedObject = plugArray[0].node();
		MFnDependencyNode connectedDepNode( connectedObject );
		if( connectedDepNode.typeId().id() == acceptedType )
		{
			m_defaultShader = connectedObject;
		}
		else
		{
			sprintf(tmp, "[liqSwitcher] bad connection type on %s.defaultShader! Skip\n", name.c_str());
			MGlobal::displayWarning(tmp);
			printf(tmp);
		}
	}
	// get conditionned shaders
	MPlug switcherPlug = shaderNode.findPlug( MString( "switchShader" ) );
	int numElts = switcherPlug.numElements(&status);
	for(i=0; i<numElts; i++)
	{
		MPlug subSwitcherPlug = switcherPlug.elementByPhysicalIndex(i, &status);
		if(status!=MS::kSuccess)
		{
			printf("[liqSwitcher] Error while getting sub array plug at id '%d' status='%s'!!\n", i, status.errorString().asChar());
			continue;
		}
		MPlug userAttributePlug = subSwitcherPlug.child(0, &status);
		if( status!=MS::kSuccess )
		{
			printf("[liqSwitcher] cannot get child plug %d under switchShader[%d] : %s\n", 0, i, status.errorString().asChar());
			continue;
		}
		MPlug customUserAttributePlug = subSwitcherPlug.child(1, &status);
		if( status!=MS::kSuccess )
		{
			printf("[liqSwitcher] cannot get child plug %d under switchShader[%d] : %s\n", 1, i, status.errorString().asChar());
			continue;
		}
		MPlug conditionPlug = subSwitcherPlug.child(2, &status);
		MPlug userValuePlug = subSwitcherPlug.child(3, &status);
		MPlug shaderPlug = subSwitcherPlug.child(4, &status);
		shaderPlug.connectedTo(plugArray, 1, 0, &status);
		if( plugArray.length() > 0 )
		{
			MObject subConnectedObject = plugArray[0].node();
			MFnDependencyNode subConnectedDepNode( subConnectedObject );
			if( subConnectedDepNode.typeId().id() == acceptedType )
			{
				MFnEnumAttribute userAttributeEnumAttr(userAttributePlug.attribute(), &status);
				MFnEnumAttribute conditionEnumAttr(conditionPlug.attribute(), &status);
				// push values
				m_userAttributes.push_back( userAttributeEnumAttr.fieldName( userAttributePlug.asInt() ) );
				m_customUserAttributes.push_back( customUserAttributePlug.asString() );
				m_conditions.push_back( conditionEnumAttr.fieldName( conditionPlug.asInt() ) );
				m_userValues.push_back( userValuePlug.asString() );
				m_shaders.push_back( subConnectedObject );
			}
			else
			{
				sprintf(tmp,"[liqSwitcher] bad connection type on %s.switchShader[%d].shader (physical id)! Skip\n", name.c_str(), i);
				MGlobal::displayWarning(tmp);
				printf(tmp);
			}
		}
		else
		{
			// skip entry, no shader connection
		}
	}
	fflush(stdout);
	fflush(stderr);
}


liqSwitcher::~liqSwitcher()
{
}


void *liqSwitcher::write(bool shortShaderNames, unsigned int indentLevel, SHADER_TYPE forceAs)
{
	if( hasErrors )
	{
		RiArchiveRecord(RI_VERBATIM, "# Errors in Switcher, skip writing\n");
		return NULL;
	}
	unsigned int i;
	RiArchiveRecord(RI_VERBATIM, "# Start Switcher\n");
	// output shaders
	for(i=0; i<m_shaders.size(); i++ )
	{
		MString userAttribute = m_userAttributes[i];
		MString customUserAttribute = m_customUserAttributes[i];
		if( userAttribute == "custom" )
		{
			userAttribute = customUserAttribute;
		}
		MString condition = m_conditions[i];
		MString userValue = m_userValues[i];
		MObject shaderObj = m_shaders[i];

		// build condition expression
		MString expression = "0 == 1";  // default = false condition
		if( userAttribute != "" )
		{
			if( condition == "is" )
			{
				expression = "defined(" + userAttribute + ") && $user:" + userAttribute + " == '" + userValue + "'";
			}
			else if( condition == "is not" )
			{
				expression = "defined(" + userAttribute + ") && $user:" + userAttribute + " != '" + userValue + "'";
			}
			else if( condition == "contains" )
			{
				expression = "defined(" + userAttribute + ") && $user:" + userAttribute + " =~ '*" + userValue + "*'";
			}
			else
			{
				printf("[liqSwitcher::write] Warning : unknown condition '%s', Skip !\n", condition.asChar());
			}
		}
		else
		{
			printf("[liqSwitcher::write] Warning : userAttribute is empty '%s', Skip !\n", condition.asChar());
		}
		// copy expression in RtToken
		// Why doing a copy where RiIfBegin( ( RtToken ) expression.asChar() ) could work ????
#ifdef DARWIN
		if( i== 0 )
		{
			RiIfBegin( ( RtToken) expression.asChar());
		}
		else
		{
			RiElseIf( ( RtToken) expression.asChar());
		}
		
#else
		int strLen = strlen(expression.asChar());
		//RtToken charExpression = new char[strLen+1];
		char *charExpression = new char[strLen+1];
		strncpy(charExpression, expression.asChar(), sizeof(char)*(strLen) );
		charExpression[strLen] = '\0';
		//printf("WRITE EXPRESSION = (%s) \n", charExpression);
		if( i== 0 )
		{
			RiIfBegin((RtToken)charExpression);
		}
		else
		{
			RiElseIf((RtToken)charExpression);
		}
		delete [] charExpression;
#endif
		liqGenericShader &shader = liqShaderFactory::instance().getShader( shaderObj, m_outputAllParameters );
		shader.write(shortShaderNames, indentLevel, forceAs);
	}
	// if conditions put default in Else
	if( m_userAttributes.size() )
	{
		RiElse();
	}
	// output default shader
	if( !m_defaultShader.isNull() )
	{
		liqGenericShader &shader = liqShaderFactory::instance().getShader( m_defaultShader, m_outputAllParameters );
		shader.write(shortShaderNames, indentLevel, forceAs);
	}
	else
	{
		RiArchiveRecord(RI_VERBATIM, "# no default surface in switcher \n");
	}
	// close if
	if( m_userAttributes.size() )
	{
		RiIfEnd();
	}
	RiArchiveRecord(RI_VERBATIM, "# End Switcher\n");
	return NULL;
}

bool liqSwitcher::isShader()
{
	return 0;
}


bool liqSwitcher::isSwitcher()
{
	return 1;
}


liqShader* liqSwitcher::asShader()
{
	return NULL;
}


liqSwitcher* liqSwitcher::asSwitcher()
{
	return this;
}


RtColor& liqSwitcher::getColor()
{
	if( !m_defaultShader.isNull() )
	{
		liqGenericShader &shader = liqShaderFactory::instance().getShader( m_defaultShader, m_outputAllParameters );
		return shader.getColor();
	}
	return rmColor;
}

RtColor& liqSwitcher::getOpacity()
{
	if( !m_defaultShader.isNull() )
	{
		liqGenericShader &shader = liqShaderFactory::instance().getShader( m_defaultShader, m_outputAllParameters );
		return shader.getOpacity();
	}
	return rmOpacity;
}

