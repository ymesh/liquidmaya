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
#ifndef liqShader_H_
#define liqShader_H_


#include <maya/MColor.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>

#include <liqTokenPointer.h>
#include <liqGetSloInfo.h>
#include <liqGenericShader.h>
#define MR_SURFPARAMSIZE 1024

#include <string>
#include <vector>

class liqSwitcher;

using namespace std;

class liqShader : public liqGenericShader
{
public :
    liqShader ();
    liqShader ( const liqShader & src );
    liqShader & operator= ( const liqShader & src );
    //liqShader ( MObject shaderObj );
    liqShader ( MObject shaderObj, bool outputAllParameters=false );
    virtual ~liqShader ();
    //MStatus liqShaderParseVectorAttr( const MFnDependencyNode& shaderNode, const string& argName, ParameterType pType );
    //MStatus liqShaderParseVectorArrayAttr( const MFnDependencyNode& shaderNode, const string& argName, ParameterType pType, unsigned int arraySize );

	void appendCoShader ( MObject coshader, MPlug plug );
	void *write ( bool shortShaderNames, unsigned int indentLevel, SHADER_TYPE forceAs=SHADER_TYPE_UNKNOWN );
	void *write ( bool shortShaderNames, unsigned int indentLevel, vector<MString> &yetExportedShaders, SHADER_TYPE forceAs=SHADER_TYPE_UNKNOWN );
	void writeRibAttributes ( MFnDependencyNode &node, SHADER_TYPE shaderType );

	//void writeAsCoShader(bool shortShaderNames, unsigned int indentLevel);
	void outputIndentation(unsigned int indentLevel);

    // void freeShader( void ); -- not needed anymore. vector calls the dtors itself when going out of scope
    // int numTPV; -- handled by tokenPointerArray.size() now

	virtual bool isShader ();
	virtual bool isSwitcher ();
	virtual liqShader* asShader ();
	virtual liqSwitcher* asSwitcher ();
//    string      name;
    string      file;
//    RtColor     rmColor;
//    RtColor     rmOpacity;
    bool        hasShadingRate;
    RtFloat     shadingRate;
    bool        hasDisplacementBound;
    RtFloat     displacementBound;
//    bool        outputInShadow;
//    bool        hasErrors;
    SHADER_TYPE shader_type;
    VOLUME_TYPE volume_type;
    bool        useVisiblePoints; // New for PPMAN 16.x: use VP.. shader version
    MString     shaderSpace;
//    MString     shaderHandler;
    int         evaluateAtEveryFrame;
//    MObject     m_mObject;
//    bool		m_outputAllParameters;     // allow to write all shader arguments even if they are on default value
    float		m_previewGamma;
    
    vector< liqTokenPointer	> tokenPointerArray;
    vector< MObject > m_coShaderArray;
};


#endif // liqShader_H_
