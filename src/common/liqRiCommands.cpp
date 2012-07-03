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
** Contributor(s): Alf Kraus.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
**
**
** ______________________________________________________________________
**
** Liquid Ri Mel commands
** 
** ______________________________________________________________________
** 
*/

// Renderman headers
#include <liqRiCommands.h>
extern "C" {
#include <ri.h>
}

#include <fstream>
#include <boost/scoped_array.hpp>
using namespace boost;
using namespace std;

// flags
const char *helpFlag = "-h", *helpLongFlag = "-help";
const char *testModeFlag = "-t", *testModeLongFlag = "-testMode";
const char *typeFlag = "-ty", *typeLongFlag = "-type";
const char *valueFlag = "-v", *valueLongFlag = "-value";
const char *timeStepFlag = "-ti", *timeStepLongFlag = "-timeStep";
const char *tokenFlag = "-to", *tokenLongFlag = "-token";
const char *inlineFlag = "-i", *inlineLongFlag = "-inline";


// ++++++++++++++++++++++++++
// RIArchiveBegin
// ++++++++++++++++++++++++++

MSyntax RIArchiveBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	return syntax;
}

MStatus RIArchiveBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if ( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIArchiveBegin [flags] archiveName\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIArchiveBegin \"myArchiveName\";\n\n\n" );
		return MS::kSuccess;
	}

	MString archiveName;
	status = argData.getCommandArgument( 0, archiveName );
	if ( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIArchiveBegin: no archiveName specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	MString call( "ArchiveBegin \"" + archiveName + "\"" );

	if ( isTest ) MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord( RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIArchiveBegin::undoIt() { return dgMod.undoIt(); }
MStatus RIArchiveBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIArchiveEnd
// ++++++++++++++++++++++++++

MSyntax RIArchiveEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIArchiveEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if ( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIArchiveEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIArchiveEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if ( isTest ) MGlobal::displayInfo( MString( "RIB output: ArchiveEnd" ) );
	else  RiArchiveRecord( RI_VERBATIM, "ArchiveEnd\n" );

	return redoIt();
}
MStatus RIArchiveEnd::undoIt() { return dgMod.undoIt(); }
MStatus RIArchiveEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIArchiveRecord
// ++++++++++++++++++++++++++

MSyntax RIArchiveRecord::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kString );
	return syntax;
}

MStatus RIArchiveRecord::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if ( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIArchiveRecord [flags] type(verbatim, structure, comment) string\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIArchiveRecord \"comment\" \"theComment\";\n\n\n" );
		return MS::kSuccess;
	}

	MString type;
	status = argData.getCommandArgument( 0, type );
	if ( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIArchiveRecord: no type specified" ) );
		return MS::kSuccess;
	}

	MString value;
	status = argData.getCommandArgument( 1, value );
	if ( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIArchiveRecord: no value specified" ) );
		return MS::kSuccess;
	}

	MString call( "" );

	if ( type == "verbatim" ) call = ( value );
	else if( type == "comment" ) call = ( "#" + value );
	else if( type == "structure" ) call = ( "##" + value );
	else
	{
		MGlobal::displayError( MString( "RIArchiveRecord: type must be either \"verbatim\", \"structure\" or \"comment\"!" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if ( isTest ) MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else RiArchiveRecord( RI_VERBATIM, (char*)call.asChar() );
	return redoIt();
}
MStatus RIArchiveRecord::undoIt() { return dgMod.undoIt(); }
MStatus RIArchiveRecord::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIAtmosphere
// ++++++++++++++++++++++++++

MSyntax RIAtmosphere::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIAtmosphere::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if ( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIAtmosphere name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIAtmosphere \"myShader\" -ty \"num\" -to \"float diffIntens\" -v \"0.8\" -ty \"str\" -to \"string colormap\" -v \"themap\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if ( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIAtmosphere: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if ( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIAtmosphere: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for ( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if ( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIAtmosphere: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if ( type == "str" ) tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if ( type == "num" ) tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Atmosphere \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if ( isTest ) MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord( RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIAtmosphere::undoIt() { return dgMod.undoIt(); }
MStatus RIAtmosphere::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIAttribute
// ++++++++++++++++++++++++++

MSyntax RIAttribute::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIAttribute::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if ( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIAttribute name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIAttribute \"displacementbound\" -ty \"num\" -to \"float sphere\" -v \"1\" -ty \"str\" -to \"string coordinatesystem\" -v \"shader\";\n" +
								"   Example: RIAttribute \"user\" -ty \"num\" -to \"vector[2] myAttribute\" -v \"1 2 3 4 5 6\";\n\n\n" );
		return MS::kSuccess;
	}

	MString name;
	status = argData.getCommandArgument( 0, name );
	if ( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIAttribute: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if ( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIAttribute: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for ( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if ( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIAttribute: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if ( type == "str" ) tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		if( type == "num" ) tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Attribute \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if ( isTest ) MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIAttribute::undoIt() { return dgMod.undoIt(); }
MStatus RIAttribute::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIAttributeBegin
// ++++++++++++++++++++++++++

MSyntax RIAttributeBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIAttributeBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if ( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIAttributeBegin [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIAttributeBegin;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if ( isTest ) MGlobal::displayInfo( MString( "RIB output: AttributeBegin" ) );
	else RiAttributeBegin();

	return redoIt();
}
MStatus RIAttributeBegin::undoIt() { return dgMod.undoIt(); }
MStatus RIAttributeBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIAttributeEnd
// ++++++++++++++++++++++++++

MSyntax RIAttributeEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIAttributeEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIAttributeEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIAttributeEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: AttributeEnd" ) );
	else
		RiAttributeEnd();

	return redoIt();
}
MStatus RIAttributeEnd::undoIt() { return dgMod.undoIt(); }
MStatus RIAttributeEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIClipping
// ++++++++++++++++++++++++++

MSyntax RIClipping::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIClipping::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIClipping [flags] near(float) far(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIClipping 0.1 10000;\n\n\n" );
		return MS::kSuccess;
	}

	double near, far;
	status = argData.getCommandArgument( 0, near );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIClipping: no near specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, far );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIClipping: no far specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Clipping " ) + near + " " + far );
	else
		RiClipping( (RtFloat)near, (RtFloat)far );

	return redoIt();
}
MStatus RIClipping::undoIt() { return dgMod.undoIt(); }
MStatus RIClipping::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIColor
// ++++++++++++++++++++++++++

MSyntax RIColor::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIColor::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIColor [flags] r(float) g(float) b(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIColor 1 0 0;\n\n\n" );
		return MS::kSuccess;
	}

	double r, g, b;
	status = argData.getCommandArgument( 0, r );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIColor: no r specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, g );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIColor: no g specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, b );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIColor: no b specified" ) );
		return MS::kSuccess;
	}

	float color[3] = {(float)r, (float)g, (float)b}; 

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Color " ) + r + " " + g + " " + b );
	else
		RiColor( color );

	return redoIt();
}
MStatus RIColor::undoIt() { return dgMod.undoIt(); }
MStatus RIColor::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIConcatTransform
// ++++++++++++++++++++++++++

MSyntax RIConcatTransform::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIConcatTransform::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIConcatTransform [flags] matrix(16 floats)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIConcatTransform 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;\n\n\n" );
		return MS::kSuccess;
	}

	double m[16];
	for( unsigned i( 0 ); i < 16; i++ )
	{
		status = argData.getCommandArgument( i, m[i] );
		if( status != MS::kSuccess )
		{
			MGlobal::displayWarning( MString( "RIConcatTransform: matrix value could not be read: " ) + i );
			return MS::kSuccess;
		}
	}
	RtMatrix mat;
	unsigned k( 0 );
	for( unsigned i( 0 ); i < 4; i++ )
	{
		for( unsigned n( 0 ); n < 4; n++ )
		{
			mat[i][n] = (float)m[k];
			k++;
		}
	}
	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ConcatTransform " )
		+m[0]+" "+m[1]+" "+m[2]+" "+m[3]+" "+m[4]+" "+m[5]+" "+m[6]+" "+m[7]+" "
		+m[8]+" "+m[9]+" "+m[10]+" "+m[11]+" "+m[12]+" "+m[13]+" "+m[14]+" "+m[15] );
	else
		RiConcatTransform( mat );

	return redoIt();
}
MStatus RIConcatTransform::undoIt() { return dgMod.undoIt(); }
MStatus RIConcatTransform::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RICropWindow
// ++++++++++++++++++++++++++

MSyntax RICropWindow::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RICropWindow::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RICropWindow [flags] xmin(float) xmax(float) ymin(float) ymax(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RICropWindow 0.0 0.3 0.0 0.5;\n\n\n" );
		return MS::kSuccess;
	}

	double xmin, xmax, ymin, ymax;
	status = argData.getCommandArgument( 0, xmin );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RICropWindow: no xmin specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, xmax );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RICropWindow: no xmax specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, ymin );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RICropWindow: no ymin specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 3, ymax );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RICropWindow: no ymax specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ScreenWindow " ) + xmin + " " + xmax + " " + ymin + " " + ymax );
	else
		RiCropWindow( (RtFloat)xmin, (RtFloat)xmax, (RtFloat)ymin, (RtFloat)ymax );

	return redoIt();
}
MStatus RICropWindow::undoIt() { return dgMod.undoIt(); }
MStatus RICropWindow::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIDepthOfField
// ++++++++++++++++++++++++++

MSyntax RIDepthOfField::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIDepthOfField::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIDepthOfField [flags] fstop(float) focallength(float) focaldistance(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIDepthOfField 22 45 1200;\n\n\n" );
		return MS::kSuccess;
	}

	double fstop, focallength, focaldistance;
	status = argData.getCommandArgument( 0, fstop );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDepthOfField: no fstop specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, focallength );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDepthOfField: no focallength specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, focaldistance );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDepthOfField: no focaldistance specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: DepthOfField " ) + fstop + " " + focallength + " " + focaldistance);
	else
		RiDepthOfField( (RtFloat)fstop, (RtFloat)focallength , (RtFloat)focaldistance );

	return redoIt();
}
MStatus RIDepthOfField::undoIt() { return dgMod.undoIt(); }
MStatus RIDepthOfField::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIDetail
// ++++++++++++++++++++++++++

MSyntax RIDetail::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIDetail::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIDetail [flags] minx(float) maxx(float) miny(float) maxy(float) minz(float) maxz(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIDetail 1 -1 1 -1 1 -1;\n\n\n" );
		return MS::kSuccess;
	}

	double minx, maxx, miny, maxy, minz, maxz;
	status = argData.getCommandArgument( 0, minx );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetail: no minx specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, maxx );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetail: no maxx specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, miny );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetail: no miny specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 3, maxy );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetail: no maxy specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 4, minz );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetail: no minz specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 5, maxz );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetail: no maxz specified" ) );
		return MS::kSuccess;
	}
	MString call = ( MString( "Detail [" )+minx+" "+maxx+" "+miny+" "+maxy+" "+minz+" "+maxx+"]" );
	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIDetail::undoIt() { return dgMod.undoIt(); }
MStatus RIDetail::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIDetailRange
// ++++++++++++++++++++++++++

MSyntax RIDetailRange::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIDetailRange::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIDetailRange [flags] minvisible(float) lowertransition(float) uppertransition(float) maxvisible(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIDetailRange 0 0 10 20;\n\n\n" );
		return MS::kSuccess;
	}

	double minvisible, lowertransition, uppertransition, maxvisible;
	status = argData.getCommandArgument( 0, minvisible );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetailRange: no minvisible specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, lowertransition );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetailRange: no lowertransition specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, uppertransition );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetailRange: no uppertransition specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 3, maxvisible );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIDetailRange: no maxvisible specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: DetailRange [" )+minvisible+" "+lowertransition+" "+uppertransition+" "+maxvisible+"]" );
	else
		RiDetailRange( (RtFloat)minvisible, (RtFloat)lowertransition, (RtFloat)uppertransition, (RtFloat)maxvisible );

	return redoIt();
}
MStatus RIDetailRange::undoIt() { return dgMod.undoIt(); }
MStatus RIDetailRange::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIDisplacement
// ++++++++++++++++++++++++++

MSyntax RIDisplacement::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIDisplacement::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIDisplacement name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIDisplacement \"myShader\" -ty \"num\" -to \"float diffIntens\" -v \"0.8\" -ty \"str\" -to \"string colormap\" -v \"themap\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIDisplacement: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIDisplacement: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIDisplacement: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Displacement \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIDisplacement::undoIt() { return dgMod.undoIt(); }
MStatus RIDisplacement::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIDisplay
// ++++++++++++++++++++++++++

MSyntax RIDisplay::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIDisplay::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIDisplay name type mode[flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIDisplay \"theImage.tif\" \"framebuffer\" \"rgba\" -ty \"str\" -to \"string filter\" -v \"separable-catmull-rom\" -ty \"num\" -to \"float[2] filterwidth\" -v \"2 2\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name, type, mode;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIDisplay: no name specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, type );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIDisplay: no type specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, mode );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIDisplay: no mode specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIDisplay: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIDisplay: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Display \"" + name + "\" " + "\"" + type + "\" " + "\"" + mode + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIDisplay::undoIt() { return dgMod.undoIt(); }
MStatus RIDisplay::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIDisplayChannel
// ++++++++++++++++++++++++++

MSyntax RIDisplayChannel::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIDisplayChannel::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIDisplayChannel name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIDisplayChannel \"color Rim\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIDisplayChannel: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIDisplayChannel: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIDisplayChannel: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "DisplayChannel \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIDisplayChannel::undoIt() { return dgMod.undoIt(); }
MStatus RIDisplayChannel::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIExterior
// ++++++++++++++++++++++++++

MSyntax RIExterior::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIExterior::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIExterior name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIExterior \"myShader\" -ty \"num\" -to \"float diffIntens\" -v \"0.8\" -ty \"str\" -to \"string colormap\" -v \"themap\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIExterior: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIExterior: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIExterior: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Exterior \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIExterior::undoIt() { return dgMod.undoIt(); }
MStatus RIExterior::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIFormat
// ++++++++++++++++++++++++++

MSyntax RIFormat::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kLong );
	syntax.addArg( MSyntax::kLong );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIFormat::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIFormat [flags] xresolution(int) yresolution(int) pixelaspectratio(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIFormat 512 512 1;\n\n\n" );
		return MS::kSuccess;
	}

	int xresolution, yresolution;
	status = argData.getCommandArgument( 0, xresolution );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIFormat: no xresolution specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, yresolution );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIFormat: no yresolution specified" ) );
		return MS::kSuccess;
	}
	double pixelaspectratio;
	status = argData.getCommandArgument( 2, pixelaspectratio );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIFormat: no pixelaspectratio specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Format " ) + xresolution + " " + yresolution + " " + pixelaspectratio );
	else
		RiFormat( (RtInt)xresolution, (RtInt)yresolution, (RtFloat)pixelaspectratio );

	return redoIt();
}
MStatus RIFormat::undoIt() { return dgMod.undoIt(); }
MStatus RIFormat::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIFrameBegin
// ++++++++++++++++++++++++++

MSyntax RIFrameBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kLong );
	return syntax;
}

MStatus RIFrameBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIFrameBegin [flags] frame\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIFrameBegin 15;\n\n\n" );
		return MS::kSuccess;
	}

	int frame;
	status = argData.getCommandArgument( 0, frame );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIFrameBegin: no frame number specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: FrameBegin " ) + frame );
	else
		RiFrameBegin( frame );

	return redoIt();
}
MStatus RIFrameBegin::undoIt() { return dgMod.undoIt(); }
MStatus RIFrameBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIFrameEnd
// ++++++++++++++++++++++++++

MSyntax RIFrameEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIFrameEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIFrameEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIFrameEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: FrameEnd" ) );
	else
		RiFrameEnd();

	return redoIt();
}
MStatus RIFrameEnd::undoIt() { return dgMod.undoIt(); }
MStatus RIFrameEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIGeometricApproximation
// ++++++++++++++++++++++++++

MSyntax RIGeometricApproximation::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIGeometricApproximation::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIGeometricApproximation [flags] type value(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIGeometricApproximation \"flatness\" 2.5;\n\n\n" );
		return MS::kSuccess;
	}

	MString type;
	status = argData.getCommandArgument( 0, type );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIGeometricApproximation: no type specified" ) );
		return MS::kSuccess;
	}
	double value;
	status = argData.getCommandArgument( 1, value );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIGeometricApproximation: no value specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: GeometricApproximation \"" ) + type + "\" " + value );
	else
		RiGeometricApproximation( (RtToken)type.asChar(), (RtFloat)value );

	return redoIt();
}
MStatus RIGeometricApproximation::undoIt() { return dgMod.undoIt(); }
MStatus RIGeometricApproximation::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIGeometry
// ++++++++++++++++++++++++++

MSyntax RIGeometry::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIGeometry::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIGeometry name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIGeometry \"cube\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIGeometry: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIGeometry: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIGeometry: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( MString( "Geometry \"" ) + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord( RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIGeometry::undoIt() { return dgMod.undoIt(); }
MStatus RIGeometry::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIHider
// ++++++++++++++++++++++++++

MSyntax RIHider::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIHider::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIHider name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIHider \"hidden\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIHider: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIHider: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIHider: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Hider \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIHider::undoIt() { return dgMod.undoIt(); }
MStatus RIHider::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIIdentity
// ++++++++++++++++++++++++++

MSyntax RIIdentity::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIIdentity::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIIdentity [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIIdentity;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Identity" ) );
	else
		RiIdentity();

	return redoIt();
}
MStatus RIIdentity::undoIt() { return dgMod.undoIt(); }
MStatus RIIdentity::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIIlluminate
// ++++++++++++++++++++++++++

MSyntax RIIlluminate::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kLong );
	syntax.addArg( MSyntax::kBoolean );
	return syntax;
}

MStatus RIIlluminate::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIIlluminate [flags] sequencenumber(int) onoff(boolean)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIIlluminate 3 off;\n\n\n" );
		return MS::kSuccess;
	}

	int sequencenumber;
	bool onoff;
	status = argData.getCommandArgument( 0, sequencenumber );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIIlluminate: no sequencenumber specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, onoff );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIIlluminate: no onoff specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Illuminate " ) + sequencenumber + " " + onoff );
	else
		RiIlluminate( (RtLightHandle)sequencenumber, (RtBoolean)onoff );

	return redoIt();
}
MStatus RIIlluminate::undoIt() { return dgMod.undoIt(); }
MStatus RIIlluminate::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIInterior
// ++++++++++++++++++++++++++

MSyntax RIInterior::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIInterior::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIInterior name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIInterior \"myShader\" -ty \"num\" -to \"float diffIntens\" -v \"0.8\" -ty \"str\" -to \"string colormap\" -v \"themap\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIInterior: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIInterior: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIInterior: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Interior \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIInterior::undoIt() { return dgMod.undoIt(); }
MStatus RIInterior::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RILightSource
// ++++++++++++++++++++++++++

MSyntax RILightSource::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kLong );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RILightSource::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RILightSource name sequencenumber(int 0-65535)[flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RILightSource \"spotlight\" 2 -ty \"num\" -to \"coneangle\" -v \"5\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RILightSource: no name specified" ) );
		return MS::kSuccess;
	}
	int sequencenumber;
	status = argData.getCommandArgument( 1, sequencenumber );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RILightSource: no sequencenumber specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RILightSource: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RILightSource: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "LightSource \"" + name + "\" " + sequencenumber + " " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RILightSource::undoIt() { return dgMod.undoIt(); }
MStatus RILightSource::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIMatte
// ++++++++++++++++++++++++++

MSyntax RIMatte::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kBoolean );
	return syntax;
}

MStatus RIMatte::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIMatte [flags] onoff(boolean)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIMatte on;\n\n\n" );
		return MS::kSuccess;
	}
	bool onoff;
	status = argData.getCommandArgument( 0, onoff );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIMatte: no onoff specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Matte " ) + onoff );
	else
		RiMatte( (RtBoolean)onoff );

	return redoIt();
}
MStatus RIMatte::undoIt() { return dgMod.undoIt(); }
MStatus RIMatte::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIMotionBegin
// ++++++++++++++++++++++++++

MSyntax RIMotionBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addFlag( timeStepFlag, timeStepLongFlag, MSyntax::kDouble );
	syntax.makeFlagMultiUse( timeStepFlag );
	return syntax;
}

MStatus RIMotionBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIMotionBegin [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n" +
								"  -ti -timeStep Float (multi-use)\n\n" +
								"   Example: RIMotionBegin -ti 1 -ti 2 -ti 3;\n\n\n" );
		return MS::kSuccess;
	}
	
	unsigned number = argData.numberOfFlagUses( timeStepFlag );
	if( !number )
	{
		MGlobal::displayWarning( MString( "RIMotionBegin: no timeStep specified" ) );
		return MS::kSuccess;
	}

	scoped_array< RtFloat > values( new RtFloat[ number ] );
	MString call( MString( "RiMotionBegin [ " ) + number + " " );
	for( unsigned i( 0 ); i < number; i++ )
	{
		argData.getFlagArgumentList( timeStepFlag, i, argList );
		double tmp = argList.asDouble( 0 );
		values[i] = ( float )tmp;
		call = ( call + ( float )tmp + " " );
	}
	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call + "]" );
	else
		RiMotionBeginV( number, values.get() );

	return redoIt();
}
MStatus RIMotionBegin::undoIt() { return dgMod.undoIt(); }
MStatus RIMotionBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIMotionEnd
// ++++++++++++++++++++++++++

MSyntax RIMotionEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIMotionEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIMotionEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIMotionEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: MotionEnd" ) );
	else
		RiMotionEnd();

	return redoIt();
}
MStatus RIMotionEnd::undoIt() { return dgMod.undoIt(); }
MStatus RIMotionEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIObjectBegin
// ++++++++++++++++++++++++++

MSyntax RIObjectBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIObjectBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIObjectBegin [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIObjectBegin;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ObjectBegin" ) );
	else
		RiObjectBegin();

	return redoIt();
}
MStatus RIObjectBegin::undoIt() { return dgMod.undoIt(); }
MStatus RIObjectBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIObjectEnd
// ++++++++++++++++++++++++++

MSyntax RIObjectEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIObjectEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIObjectEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIObjectEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ObjectEnd" ) );
	else
		RiObjectEnd();

	return redoIt();
}
MStatus RIObjectEnd::undoIt() { return dgMod.undoIt(); }
MStatus RIObjectEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIOpacity
// ++++++++++++++++++++++++++

MSyntax RIOpacity::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIOpacity::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIOpacity [flags] r(float) g(float) b(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIOpacity 1 0 0;\n\n\n" );
		return MS::kSuccess;
	}

	double r, g, b;
	status = argData.getCommandArgument( 0, r );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIOpacity: no r specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, g );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIOpacity: no g specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, b );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIOpacity: no b specified" ) );
		return MS::kSuccess;
	}

	float color[3] = {(float)r, (float)g, (float)b}; 

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Opacity " ) + r + " " + g + " " + b );
	else
		RiOpacity( color );

	return redoIt();
}
MStatus RIOpacity::undoIt() { return dgMod.undoIt(); }
MStatus RIOpacity::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIOption
// ++++++++++++++++++++++++++

MSyntax RIOption::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIOption::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIOption name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIOption \"searchpath\" -ty \"str\" -to \"shader\" -v \"shaderpath\";\n" +
								"   Example: RIOption \"limits\" -ty \"num\" -to \"gridsize\" -v \"32\" -ty \"num\" -to \"bucketsize\" -v \"12 12\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIOption: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIOption: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIOption: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Option \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIOption::undoIt() { return dgMod.undoIt(); }
MStatus RIOption::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIOrientation
// ++++++++++++++++++++++++++

MSyntax RIOrientation::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	return syntax;
}

MStatus RIOrientation::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIOrientation [flags] orientation(outside, inside, lh, rh)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIOrientation \"lh\";\n\n\n" );
		return MS::kSuccess;
	}

	MString orientation;
	status = argData.getCommandArgument( 0, orientation );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIOrientation: no orientation specified" ) );
		return MS::kSuccess;
	}
	if( orientation != "outside" && orientation != "inside" && orientation != "lh" && orientation != "rh"  )
	{
		MGlobal::displayWarning( MString( "RIOrientation: Orientation must be \"outside\", \"inside\", \"lh\" or \"rh\"") );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Orientation \"" ) + orientation + "\"" );
	else
		RiOrientation( (RtToken)orientation.asChar() );

	return redoIt();
}
MStatus RIOrientation::undoIt() { return dgMod.undoIt(); }
MStatus RIOrientation::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIPixelSamples
// ++++++++++++++++++++++++++

MSyntax RIPixelSamples::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIPixelSamples::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIPixelSamples [flags] xsamples(float) ysamples(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIPixelSamples 6 4;\n\n\n" );
		return MS::kSuccess;
	}

	double xsamples, ysamples;
	status = argData.getCommandArgument( 0, xsamples );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIPixelSamples: no xsamples specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, ysamples );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIPixelSamples: no ysamples specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: PixelSamples " ) + xsamples + " " + ysamples );
	else
		RiPixelSamples( (RtFloat)xsamples, (RtFloat)ysamples );

	return redoIt();
}
MStatus RIPixelSamples::undoIt() { return dgMod.undoIt(); }
MStatus RIPixelSamples::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIProcedural
// ++++++++++++++++++++++++++

MSyntax RIProcedural::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kString );
	return syntax;
}

MStatus RIProcedural::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIProcedural [flags] type(DelayedReadArchive, DynamicLoad, RunProgram) fileName bound1 bound2 bound3 bound4 bound5 bound6 [parameter] \nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n\n" +
								"   Example: RIProcedural \"DelayedReadArchive\" \"myArchive\" 1 2 3 4 5 6;\n" +
								"   Example: RIProcedural \"DynamicLoad\" \"myDso\" 1 2 3 4 5 6 \"parameter string\";\n\n\n" );
		return MS::kSuccess;
	}


	MString type;
	status = argData.getCommandArgument( 0, type );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIProcedural: no type specified" ) );
		return MS::kSuccess;
	}

	MString fileName;
	status = argData.getCommandArgument( 1, fileName );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIProcedural: no fileName specified" ) );
		return MS::kSuccess;
	}

	MDoubleArray bbb;
	for( unsigned i(2); i < 8; i++ )
	{
		double tmp;
		status = argData.getCommandArgument( i, tmp );
		if( status != MS::kSuccess )
		{
			MGlobal::displayError( MString( "RIProcedural: Could not read bounding box value: " ) + (i+1) );
			return MS::kSuccess;
		}
		bbb.append( tmp );
	}
	MString bb = ( MString("")+bbb[0]+" "+bbb[1]+" "+bbb[2]+" "+bbb[3]+" "+bbb[4]+" "+bbb[5] );


	MString param;
	MString call( "" );
	if( type == "DynamicLoad" )
	{
		status = argData.getCommandArgument( 8, param );
		if( status != MS::kSuccess )
		{
			MGlobal::displayError( MString( "RIProcedural: no parameter specified" ) );
			return MS::kSuccess;
		}
		call = ( "Procedural \"" + type + "\" [\"" + fileName + "\" \"" + param + "\"] [" + bb + "]" );
	}
	else if( type == "DelayedReadArchive" || type == "RunProgram" )
		call = ( "Procedural \"" + type + "\" [\"" + fileName + "\"] [" + bb + "]" );
	else
	{
		MGlobal::displayError( MString( "RIProcedural: type must be \"DelayedReadArchive\", \"DynamicLoad\" or \"RunProgram\"" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord(RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIProcedural::undoIt() { return dgMod.undoIt(); }
MStatus RIProcedural::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIProjection
// ++++++++++++++++++++++++++

MSyntax RIProjection::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIProjection::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIProjection name(perspective, orthographic) [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIProjection \"perspective\" -ty \"num\" -to \"fov\" -v \"45\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name("");
	status = argData.getCommandArgument( 0, name );

	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIProjection: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIProjection: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Projection \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIProjection::undoIt() { return dgMod.undoIt(); }
MStatus RIProjection::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIReadArchive
// ++++++++++++++++++++++++++

MSyntax RIReadArchive::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addFlag( inlineFlag, inlineLongFlag );
	syntax.addArg( MSyntax::kString );
	return syntax;
}

MStatus RIReadArchive::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIReadArchive [flags] archivename\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"   -i -inline, inlines the Read Archive into the current RIB\n\n" +
								"   Example: RIReadArchive \"myArchive\";\n\n\n" );
		return MS::kSuccess;
	}

	MString archiveName;
	status = argData.getCommandArgument( 0, archiveName );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIReadArchive: no archive name specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	bool isInline = argData.isFlagSet( inlineFlag );

	if( isTest && !isInline )
		MGlobal::displayInfo( MString( "RIB output: ReadArchive \"" ) + archiveName.asChar() + "\"" );
	else if( !isInline )
		RiReadArchive( const_cast< RtToken >( archiveName.asChar() ), NULL, RI_NULL );
	else
	{
		const char *fileName = archiveName.asChar();
		std::ifstream inFile( fileName, std::ios::in );

		if( !inFile.is_open() )
		{
			MGlobal::displayError( MString( "RIReadArchive: Could not open " ) + archiveName );
			inFile.close();
			return status;
		}
		else
		{
			unsigned long length;
			char * buffer;
			inFile.seekg( 0, ios::end );
			length = inFile.tellg();
			inFile.seekg( 0, ios::beg );
			buffer = new char[ length + 1 ];
			inFile.read( buffer, length );
			buffer[ length ] = '\0';
			inFile.close();
			if( isTest )
				MGlobal::displayInfo( buffer );
			else
			{
				MString inlined( MString( " Inlined Read Archive: " ) + archiveName );
				RiArchiveRecord( RI_VERBATIM, "\n" );
				RiArchiveRecord( RI_COMMENT, (char*)inlined.asChar() );
				RiArchiveRecord( RI_VERBATIM, buffer );
			}
			delete buffer;
		}
	}

	return redoIt();
}
MStatus RIReadArchive::undoIt() { return dgMod.undoIt(); }
MStatus RIReadArchive::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIRelativeDetail
// ++++++++++++++++++++++++++

MSyntax RIRelativeDetail::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIRelativeDetail::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIRelativeDetail [flags] relativedetail(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIRelativeDetail 0.6;\n\n\n" );
		return MS::kSuccess;
	}

	double relativedetail;
	status = argData.getCommandArgument( 0, relativedetail );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIRelativeDetail: no relativedetail specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: RelativeDetail " ) + relativedetail );
	else
		RiRelativeDetail( (RtFloat)relativedetail );

	return redoIt();
}
MStatus RIRelativeDetail::undoIt() { return dgMod.undoIt(); }
MStatus RIRelativeDetail::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIResource
// ++++++++++++++++++++++++++

MSyntax RIResource::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RIResource::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIResource handle(string) type(string)[flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RIResource \"greenmarble\" \"attributes\" -ty \"str\" -to \"string operation\" -v \"save\";\n\n\n" );
		return MS::kSuccess;
	}
	MString handle, type;
	status = argData.getCommandArgument( 0, handle );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIResource: no handle specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, type );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RIResource: no type specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RIResource: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RIResource: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Resource \"" + handle + "\" \"" + type + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIResource::undoIt() { return dgMod.undoIt(); }
MStatus RIResource::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIResourceBegin
// ++++++++++++++++++++++++++

MSyntax RIResourceBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIResourceBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIResourceBegin [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIResourceBegin;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ResourceBegin" ) );
	else
	{
		RiArchiveRecord( RI_VERBATIM, "ResourceBegin" );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIResourceBegin::undoIt() { return dgMod.undoIt(); }
MStatus RIResourceBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIResourceEnd
// ++++++++++++++++++++++++++

MSyntax RIResourceEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIResourceEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIResourceEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIResourceEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ResourceEnd" ) );
	else
	{
		RiArchiveRecord( RI_VERBATIM, "ResourceEnd" );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RIResourceEnd::undoIt() { return dgMod.undoIt(); }
MStatus RIResourceEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIReverseOrientation
// ++++++++++++++++++++++++++

MSyntax RIReverseOrientation::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIReverseOrientation::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIReverseOrientation [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIReverseOrientation;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ReverseOrientation" ) );
	else
		RiReverseOrientation();

	return redoIt();
}
MStatus RIReverseOrientation::undoIt() { return dgMod.undoIt(); }
MStatus RIReverseOrientation::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIRotate
// ++++++++++++++++++++++++++

MSyntax RIRotate::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIRotate::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIRotate [flags] angle(float) dx(float) dy(float) dz(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIRotate 90 0 1 0;\n\n\n" );
		return MS::kSuccess;
	}

	double angle, dx, dy, dz;
	status = argData.getCommandArgument( 0, angle );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIRotate: no angle specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, dx );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIRotate: no dx specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, dy );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIRotate: no dy specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 3, dz );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIRotate: no dz specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Rotate " ) + angle + " " + dx + " " + dy + " " + dz );
	else
		RiRotate( (RtFloat)angle, (RtFloat)dx, (RtFloat)dy, (RtFloat)dz );

	return redoIt();
}
MStatus RIRotate::undoIt() { return dgMod.undoIt(); }
MStatus RIRotate::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIScale
// ++++++++++++++++++++++++++

MSyntax RIScale::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIScale::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIScale [flags] sx(float) sy(float) sz(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIScale 0.5 1 1;\n\n\n" );
		return MS::kSuccess;
	}

	double sx, sy, sz;
	status = argData.getCommandArgument( 0, sx );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIScale: no sx specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, sy );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIScale: no sy specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, sz );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIScale: no sz specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Scale " ) + sx + " " + sy + " " + sz );
	else
		RiScale( (RtFloat)sx, (RtFloat)sy, (RtFloat)sz );

	return redoIt();
}
MStatus RIScale::undoIt() { return dgMod.undoIt(); }
MStatus RIScale::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIScreenWindow
// ++++++++++++++++++++++++++

MSyntax RIScreenWindow::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIScreenWindow::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIScreenWindow [flags] left(float) right(float) bot(float) top(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIScreenWindow -1.33333 1.33333 -1 1;\n\n\n" );
		return MS::kSuccess;
	}

	double left, right, bot, top;
	status = argData.getCommandArgument( 0, left );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIScreenWindow: no left specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, right );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIScreenWindow: no right specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, bot );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIScreenWindow: no bot specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 3, top );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIScreenWindow: no top specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ScreenWindow " ) + left + " " + right + " " + bot + " " + top );
	else
		RiScreenWindow( (RtFloat)left, (RtFloat)right, (RtFloat)bot, (RtFloat)top );

	return redoIt();
}
MStatus RIScreenWindow::undoIt() { return dgMod.undoIt(); }
MStatus RIScreenWindow::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIShadingInterpolation
// ++++++++++++++++++++++++++

MSyntax RIShadingInterpolation::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	return syntax;
}

MStatus RIShadingInterpolation::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIShadingInterpolation [flags] type(constant, smooth)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIShadingInterpolation \"constant\";\n\n\n" );
		return MS::kSuccess;
	}

	MString type;
	status = argData.getCommandArgument( 0, type );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIShadingInterpolation: no type specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ShadingInterpolation \"" ) + type );
	else
		RiShadingInterpolation( (RtToken)type.asChar() );

	return redoIt();
}
MStatus RIShadingInterpolation::undoIt() { return dgMod.undoIt(); }
MStatus RIShadingInterpolation::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIShadingRate
// ++++++++++++++++++++++++++

MSyntax RIShadingRate::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIShadingRate::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIShadingRate [flags] size(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIShadingRate 1.0;\n\n\n" );
		return MS::kSuccess;
	}

	double size;
	status = argData.getCommandArgument( 0, size );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIShadingRate: no size specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: ShadingRate " ) + size );
	else
		RiShadingRate( (RtFloat)size );

	return redoIt();
}
MStatus RIShadingRate::undoIt() { return dgMod.undoIt(); }
MStatus RIShadingRate::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIShutter
// ++++++++++++++++++++++++++

MSyntax RIShutter::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RIShutter::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIShutter [flags] min(float) max(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIShutter 0.1 0.9;\n\n\n" );
		return MS::kSuccess;
	}

	double min, max;
	status = argData.getCommandArgument( 0, min );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIShutter: no min specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, max );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RIShutter: no max specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Shutter " ) + min + " " + max );
	else
		RiShutter( (RtFloat)min, (RtFloat)max );

	return redoIt();
}
MStatus RIShutter::undoIt() { return dgMod.undoIt(); }
MStatus RIShutter::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RISides
// ++++++++++++++++++++++++++

MSyntax RISides::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kLong );
	return syntax;
}

MStatus RISides::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RISides [flags] sides(1, 2)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RISides 2;\n\n\n" );
		return MS::kSuccess;
	}

	int sides;
	status = argData.getCommandArgument( 0, sides );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISides: no sides specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Sides " ) + sides );
	else
		RiSides( (RtInt)sides );

	return redoIt();
}
MStatus RISides::undoIt() { return dgMod.undoIt(); }
MStatus RISides::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RISkew
// ++++++++++++++++++++++++++

MSyntax RISkew::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RISkew::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RISkew [flags] angle(float) dx1(float) dy1(float) dz1(float) dx2(float) dy2(float) dz2(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RISkew 45 0 1 0 1 0 0;\n\n\n" );
		return MS::kSuccess;
	}
	double angle, dx1, dy1, dz1, dx2, dy2, dz2;
	status = argData.getCommandArgument( 0, angle );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISkew: no angle specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, dx1 );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISkew: no dx1 specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, dy1 );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISkew: no dy1 specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 3, dz1 );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISkew: no dz1 specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 4, dx2 );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISkew: no dx2 specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 5, dy2 );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISkew: no dy2 specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 6, dz2 );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISkew: no dz2 specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Skew " )+angle+" "+dx1+" "+dy1+" "+dz1+" "+dx2+" "+dy2+" "+dz2 );
	else
		RiSkew( (RtFloat)angle, (RtFloat)dx1, (RtFloat)dy1, (RtFloat)dz1, (RtFloat)dx2, (RtFloat)dy2, (RtFloat) dz2 );

	return redoIt();
}
MStatus RISkew::undoIt() { return dgMod.undoIt(); }
MStatus RISkew::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RISolidBegin
// ++++++++++++++++++++++++++

MSyntax RISolidBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	return syntax;
}

MStatus RISolidBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RISolidBegin [flags] operation(primitive, intersection, union, difference)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RISolidBegin \"intersection\";\n\n\n" );
		return MS::kSuccess;
	}

	MString operation;
	status = argData.getCommandArgument( 0, operation );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RISolidBegin: no operation specified" ) );
		return MS::kSuccess;
	}
	if( operation != "primitive" && operation != "intersection" && operation != "union" && operation != "difference" )
	{
		MGlobal::displayWarning( MString( "RISolidBegin: Operation must be either \"primitive\", \"intersection\", \"union\" or \"difference\"" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: SolidBegin \"" ) + operation + "\"" );
	else
		RiSolidBegin( (char*)operation.asChar() );

	return redoIt();
}
MStatus RISolidBegin::undoIt() { return dgMod.undoIt(); }
MStatus RISolidBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RISolidEnd
// ++++++++++++++++++++++++++

MSyntax RISolidEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RISolidEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RISolidEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RISolidEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: SolidEnd" ) );
	else
		RiSolidEnd();

	return redoIt();
}
MStatus RISolidEnd::undoIt() { return dgMod.undoIt(); }
MStatus RISolidEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RISphere
// ++++++++++++++++++++++++++

MSyntax RISphere::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RISphere::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RISphere radius(float) zmin(float) zmax(float) thetamax(float) [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RISphere 0.5 0 0.5 360;\n\n\n" );
		return MS::kSuccess;
	}
	double radius, zmin, zmax, thetamax;
	status = argData.getCommandArgument( 0, radius );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RISphere: no radius specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, zmin );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RISphere: no zmin specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, zmax );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RISphere: no zmax specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 3, thetamax );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RISphere: no thetamax specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RISphere: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RISphere: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( MString( "Sphere " )+radius+" "+zmin+" "+zmax+" "+thetamax+" "+tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RISphere::undoIt() { return dgMod.undoIt(); }
MStatus RISphere::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RISurface
// ++++++++++++++++++++++++++

MSyntax RISurface::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kString );
	syntax.addFlag( typeFlag, typeLongFlag, MSyntax::kString );
	syntax.addFlag( tokenFlag, tokenLongFlag, MSyntax::kString );
	syntax.addFlag( valueFlag, valueLongFlag, MSyntax::kString );
	syntax.makeFlagMultiUse( tokenFlag );
	syntax.makeFlagMultiUse( valueFlag );
	syntax.makeFlagMultiUse( typeFlag );
	return syntax;
}

MStatus RISurface::doIt( const MArgList &args )
{
	MStatus status( MS::kSuccess );
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RISurface name [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode, prints the RIB output in the script editor\n" +
								"  -ty -type  String (multi-use), must be \"str\" or \"num\"\n" +
								"  -to -token String (multi-use)\n" +
								"   -v -value String (multi-use)\n\n" +
								"   Example: RISurface \"myShader\" -ty \"num\" -to \"float diffIntens\" -v \"0.8\" -ty \"str\" -to \"string colormap\" -v \"themap\";\n\n\n" );
		return MS::kSuccess;
	}
	MString name;
	status = argData.getCommandArgument( 0, name );
	if( status != MS::kSuccess )
	{
		MGlobal::displayError( MString( "RISurface: no name specified" ) );
		return MS::kSuccess;
	}
	unsigned numTokens = argData.numberOfFlagUses( tokenFlag );
	unsigned numTypes = argData.numberOfFlagUses( typeFlag );
	unsigned numValues = argData.numberOfFlagUses( valueFlag );
	if( numTokens != numValues || numValues != numTypes )
	{
		MGlobal::displayError( MString( "RISurface: token, type and value flag counts must match!" ) );
		return( MS::kSuccess );
	}

	MStringArray tokens;
	MStringArray types;
	MString tokenValues;
	for( unsigned i( 0 ); i < numTokens; i++ )
	{
		MString type;
		argData.getFlagArgumentList( tokenFlag, i, argList );
		tokenValues = ( tokenValues + "\"" + argList.asString( 0 ) + "\" " );
		argData.getFlagArgumentList( typeFlag, i, argList );
		type = argList.asString( 0 );
		if( type != "str" && type != "num" )
		{
			MGlobal::displayError( MString( "RISurface: type must be either \"str\" or \"num\"!" ) );
			return( MS::kSuccess );
		}
		argData.getFlagArgumentList( valueFlag, i, argList );
		if( type == "str" )
			tokenValues = ( tokenValues + "[\"" + argList.asString( 0 ) + "\"] " );
		if( type == "num" )
			tokenValues = ( tokenValues + "[" + argList.asString( 0 ) + "] " );
	}
	MString call = ( "Surface \"" + name + "\" " + tokenValues );

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: " ) + call );
	else
	{
		RiArchiveRecord(RI_VERBATIM, (char*)call.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}
	return redoIt();
}
MStatus RISurface::undoIt() { return dgMod.undoIt(); }
MStatus RISurface::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RITransformBegin
// ++++++++++++++++++++++++++

MSyntax RITransformBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RITransformBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RITransformBegin [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RITransformBegin;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: TransformBegin" ) );
	else
		RiTransformBegin();

	return redoIt();
}
MStatus RITransformBegin::undoIt() { return dgMod.undoIt(); }
MStatus RITransformBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RITransformEnd
// ++++++++++++++++++++++++++

MSyntax RITransformEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RITransformEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RITransformEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RITransformEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: TransformEnd" ) );
	else
		RiTransformEnd();

	return redoIt();
}
MStatus RITransformEnd::undoIt() { return dgMod.undoIt(); }
MStatus RITransformEnd::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RITranslate
// ++++++++++++++++++++++++++

MSyntax RITranslate::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	syntax.addArg( MSyntax::kDouble );
	return syntax;
}

MStatus RITranslate::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RITranslate [flags] dx(float) dy(float) dz(float)\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RITranslate 0 5 4;\n\n\n" );
		return MS::kSuccess;
	}

	double dx, dy, dz;
	status = argData.getCommandArgument( 0, dx );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RITranslate: no dx specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 1, dy );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RITranslate: no dy specified" ) );
		return MS::kSuccess;
	}
	status = argData.getCommandArgument( 2, dz );
	if( status != MS::kSuccess )
	{
		MGlobal::displayWarning( MString( "RITranslate: no dz specified" ) );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );
	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: Translate " ) + dx + " " + dy + " " + dz );
	else
		RiTranslate( (RtFloat)dx, (RtFloat)dy, (RtFloat)dz );

	return redoIt();
}
MStatus RITranslate::undoIt() { return dgMod.undoIt(); }
MStatus RITranslate::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIWorldBegin
// ++++++++++++++++++++++++++

MSyntax RIWorldBegin::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIWorldBegin::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIWorldBegin [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIWorldBegin;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if( isTest )
		MGlobal::displayInfo( MString( "RIB output: WorldBegin" ) );
	else
		RiWorldBegin();

	return redoIt();
}
MStatus RIWorldBegin::undoIt() { return dgMod.undoIt(); }
MStatus RIWorldBegin::redoIt() { return dgMod.doIt(); }


// ++++++++++++++++++++++++++
// RIWorldEnd
// ++++++++++++++++++++++++++

MSyntax RIWorldEnd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag( helpFlag, helpLongFlag );
	syntax.addFlag( testModeFlag, testModeLongFlag );
	return syntax;
}

MStatus RIWorldEnd::doIt( const MArgList &args )
{
	MStatus status;
	MArgList argList;
	MArgDatabase argData( syntax(), args );

	bool isHelp = argData.isFlagSet( helpFlag );
	if ( isHelp )
	{
		MGlobal::displayInfo( MString( "\n\nSynopsis: RIWorldEnd [flags]\nFlags:\n" ) +
								"   -h -help\n" +
								"   -t -testMode\n\n" +
								"   Example: RIWorldEnd;\n\n\n" );
		return MS::kSuccess;
	}

	bool isTest = argData.isFlagSet( testModeFlag );

	if ( isTest ) MGlobal::displayInfo( MString( "RIB output: WorldEnd" ) );
	else RiWorldEnd();

	return redoIt();
}
MStatus RIWorldEnd::undoIt() { return dgMod.undoIt(); }
MStatus RIWorldEnd::redoIt() { return dgMod.doIt(); }

