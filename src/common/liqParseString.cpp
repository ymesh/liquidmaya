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
** Contributor(s): Baptiste Sansierra.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1 989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#include <liqParseString.h>
#include <liqGlobalHelpers.h>

#include <maya/MArgList.h>
#include <maya/MString.h>


liqParseString::liqParseString()
{
}


liqParseString::~liqParseString()
{
}


void *liqParseString::creator()
{
	return new liqParseString();
}


MStatus liqParseString::doIt(const MArgList& args )
{
	MStatus status;
	unsigned int i;
	bool doEscape = true;
	bool doRemoveEscapes = true;
	MString stringToParse = "";
	MString stringParsed;
	for ( i = 0 ; i < args.length() ; i++ )
	{
		MString arg = args.asString( i, &status );
		if ( ( arg == "-s" ) || ( arg == "-string") )
		{
			stringToParse = args.asString( ++i, &status );
		}
		else if ( ( arg == "-de" ) || ( arg == "-doEscape" ) )
		{
			doEscape = args.asInt( ++i, &status );
		}
		else if ( (arg == "-dre" ) || ( arg == "-doRemoveEscapes" ) )
		{
			doRemoveEscapes = args.asInt( ++i, &status );
		}
		else
		{
			displayWarning( "[liqParseString] unknown arg '" + arg + "' ... skip" );
		}
	}
	if ( stringToParse == "" )
	{
		setResult( "" );
		return MS::kSuccess;
	}
	stringParsed = parseString( stringToParse, doEscape );
	if( doRemoveEscapes )
	{
		stringParsed = removeEscapes( stringParsed );
	}
	setResult( stringParsed );
	return MS::kSuccess;
}


