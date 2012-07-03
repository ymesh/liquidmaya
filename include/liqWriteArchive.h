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
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#ifndef liqWriteArchive_H
#define liqWriteArchive_H

/* ______________________________________________________________________
**
** Liquid Get .slo Info Header File
** ______________________________________________________________________
*/


#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MStringArray.h>
#include <maya/MDagPath.h>
#include <maya/MSyntax.h>

class liqRibNode;

class liqWriteArchive : public MPxCommand
{
public:
	liqWriteArchive();
	virtual ~liqWriteArchive();

	static void* creator();
	static MSyntax syntax();

	MStatus doIt(const MArgList& args);
	MStatus parseArguments(const MArgList& args);

private:
	void outputIndentation();
	void outputObjectName(const MDagPath &objDagPath);
	void writeObjectToRib(const MDagPath &objDagPath, bool writeTransform);
	MStringArray stringArrayRemoveDuplicates(MStringArray src);

	//void writeSurface(liqRibNode &);
	//void writeDisplace(liqRibNode &);
	//void writeVolume(liqRibNode &);

private:
	static MSyntax m_syntax;
	unsigned int  m_indentLevel;
	MStringArray   m_objectNames;
	MString        m_outputFilename;
	//bool           outputRootTransform;
	//bool           outputChildTransforms;
	bool           m_exportTransform;
	bool           m_binaryRib;
	bool           m_debug;
	
	bool           m_exportSurface;
	bool           m_exportDisplace;
	bool           m_exportVolume;
	bool           m_shortShaderNames;
};


#endif
