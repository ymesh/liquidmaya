/*
**
**
** The contents of this file are subject to the Mozilla Public License Version
** 1.1  (the "License"); you may not use this file except in compliance with
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
** Contributor(s): Berj Bannayan.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1 989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#include <liqRibShaderTranslator.h>
#include <liqShaderFactory.h>

#include <maya/MSelectionList.h>


liqRibShaderTranslator::liqRibShaderTranslator()
{
}


liqRibShaderTranslator::~liqRibShaderTranslator()
{
}


/**
 * Creates a new instance of the translator.
 */
void *liqRibShaderTranslator::creator()
{
  return new liqRibShaderTranslator();
}


MStatus liqRibShaderTranslator::doIt(const MArgList& args )
{
	MStatus status;
	MString output = "";
	MStringArray nodes;
	unsigned int i;
	for ( i=0; i<args.length(); i++ )
	{
		MString arg = args.asString( i, &status );
		if ( ( arg == "-o" ) || ( arg == "-output" ) )
			output = args.asString( ++i, &status );
		else if ( ( arg == "-n" ) || ( arg == "-node" ) )
			nodes.append( args.asString( ++i, &status ) );
		else
			displayWarning("[liqRibShaderTranslator] unknown arg '" + arg + "' ... skip");
	}
	// check args
	if ( output == "" )
	{
		displayWarning("[liqRibShaderTranslator] no output file");
		return MS::kFailure;
	}
	if ( nodes.length() == 0 )
	{
		displayWarning("[liqRibShaderTranslator] no shaders to export");
		return MS::kFailure;
	}
	// check file
	FILE *testHandler = fopen(output.asChar(), "w+");
	if ( !testHandler )
	{
		displayWarning( "[liqRibShaderTranslator] cannot open file " + output);
		return MS::kFailure;
	}
	fclose( testHandler );
	// get MObjects from names
	MSelectionList selectionList;
	for ( i = 0 ; i < nodes.length() ; i++ ) selectionList.add( nodes[ i ] );
	// clear shaders
	liqShaderFactory::instance().clearShaders();
	// export shaders
	RtString format[1] = {"ascii"};
	RiOption("rib", "format", (RtPointer)format, RI_NULL);
	RiBegin(const_cast<char*>(output.asChar()));
	for ( i = 0 ; i < selectionList.length( ); i++ )
	{
		MObject depNode;
		selectionList.getDependNode(i, depNode);
		liqGenericShader &shader = liqShaderFactory::instance().getShader( depNode );
		shader.write(0, 0);
	}
	RiEnd();
	setResult(output);
	return MS::kSuccess;
}


