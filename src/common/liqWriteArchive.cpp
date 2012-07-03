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

#include <liquid.h>
#include <liqWriteArchive.h>
#include <liqRibNode.h>
#include <liqRibObj.h>
#include <liqGlobalHelpers.h>
#include <liqIOStream.h>
#include <liqShaderFactory.h>

#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MStatus.h>
#include <maya/MSelectionList.h>
#include <maya/MMatrix.h>
#include <maya/MFnDagNode.h>
#include <maya/MArgParser.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnSet.h>
#include <maya/MPlug.h>

#include <ri.h>


// RI_VERBATIM is in the current RenderMan spec but
// some RIB libraries don't know about it
#ifndef RI_VERBATIM
  #define RI_VERBATIM "verbatim"
#endif


MSyntax liqWriteArchive::m_syntax;


extern bool liqglo_outputMeshAsRMSArrays;            // true => write uvs as arrays
liqWriteArchive::liqWriteArchive() : m_indentLevel(0), m_outputFilename("/tmp/tmprib.rib"), m_exportTransform(1)
{
	m_exportSurface = 0;
	m_exportDisplace = 0;
	m_exportVolume = 0;
	m_shortShaderNames = 0;
	liqglo_outputMeshAsRMSArrays = true;
}


liqWriteArchive::~liqWriteArchive()
{
}


void* liqWriteArchive::creator()
{
	return new liqWriteArchive();
}


MSyntax liqWriteArchive::syntax()
{
	MSyntax &syn = liqWriteArchive::m_syntax;

	syn.useSelectionAsDefault(true);
	syn.setObjectType(MSyntax::kStringObjects, 0);

	syn.addFlag("b", "binary");
	syn.addFlag("d", "debug");
	syn.addFlag("o", "output", MSyntax::kString);

	syn.addFlag("es", "exportSurface", MSyntax::kBoolean);
	syn.addFlag("ed", "exportDisplace", MSyntax::kBoolean);
	syn.addFlag("ev", "exportVolume", MSyntax::kBoolean);
	syn.addFlag("ssn", "shortShaderNames", MSyntax::kBoolean);

	return syn;
}


MStatus liqWriteArchive::parseArguments(const MArgList& args)
{
	MStatus status;
	MArgParser argParser(syntax(), args);
	unsigned int flagIndex;
	//for(int i=0; i<args.length(); i++)
	//{
	//	MString tmp = args.asString(i);
	//	printf("ARG %d : %s \n", i, tmp.asChar());
	//}
	// flag debug
	m_debug = false;
	flagIndex = args.flagIndex("d", "debug");
	if ( flagIndex != MArgList::kInvalidArgIndex )m_debug = true;
	
	// flag binary
	m_binaryRib = false;
	flagIndex = args.flagIndex("b", "binary");
	if ( flagIndex != MArgList::kInvalidArgIndex ) m_binaryRib = true;
	
	// flag output
	flagIndex = args.flagIndex("o", "output");
	if ( flagIndex == MArgList::kInvalidArgIndex )
	{
		displayError("[liqWriteArchive::doIt] Must provide the output rib : liquidWriteArchive -o /mon/rib.rib");
		return MS::kInvalidParameter;
	}
	m_outputFilename = args.asString(flagIndex+1);
	// flag exportSurface
	flagIndex = args.flagIndex("es", "exportSurface");
	if ( flagIndex != MArgList::kInvalidArgIndex ) m_exportSurface = args.asInt(flagIndex+1);
	
	// flag exportDisplace
	flagIndex = args.flagIndex("ed", "exportDisplace");
	if ( flagIndex != MArgList::kInvalidArgIndex ) m_exportDisplace = args.asInt(flagIndex+1);
	
	// flag exportVolume
	flagIndex = args.flagIndex("ev", "exportVolume");
	if ( flagIndex != MArgList::kInvalidArgIndex ) m_exportVolume = args.asInt(flagIndex+1);
	
	// flag shortShaderNames
	flagIndex = args.flagIndex("ssn", "shortShaderNames");
	if ( flagIndex != MArgList::kInvalidArgIndex ) m_shortShaderNames = args.asInt(flagIndex+1);
	
	// get objetcs
	unsigned int i;
	MStringArray listToBeExported;
	status = argParser.getObjects( listToBeExported );
	if ( status!=MS::kSuccess )
	{
		displayError("[liqWriteArchive::doIt] Must provide objects to export : liquidWriteArchive {\"obj1\", \"obj2\", \"obj3\"}");
		return MS::kInvalidParameter;
	}
	//listToBeExported = stringArrayRemoveDuplicates(listToBeExported);
	
	// if provided list is empty, get selection
	if ( listToBeExported.length() == 0 )
	{
		MSelectionList list;
		MGlobal::getActiveSelectionList(list);
		list.getSelectionStrings(listToBeExported);
	}
	m_objectNames = listToBeExported;
	if ( m_debug )
	{
		printf("[liqWriteArchive::doIt] exporting objects :\n");
		for ( i = 0; i < listToBeExported.length() ; i++ )
		{
			printf("    '%d' : '%s' \n", i, listToBeExported[i].asChar() );
		}
		printf("et c'est tout\n");
	}
	return MS::kSuccess;
}


MStatus liqWriteArchive::doIt(const MArgList& args)
{
	MStatus status;
	status = parseArguments(args);
	if ( status != MS::kSuccess ) return status;
	
	// clear shaders
	liqShaderFactory::instance().clearShaders();
	unsigned int i;
	unsigned int j;
	std::vector<MDagPath> objDb;
	std::vector<MObject> setsDn;
	// building an array with the MDagPaths to export
	for ( i = 0 ; i < m_objectNames.length(); i++ )
	{
		// get a handle on the named object
		MSelectionList selList;
		selList.add(m_objectNames[i]);
		MDagPath objDagPath;
		status = selList.getDagPath(0, objDagPath);
		if ( !status )
		{
			MObject depNode;
			status = selList.getDependNode(0, depNode);
			if ( !status )
			{
				MGlobal::displayWarning("[liqWriteArchive::doIt] Error retrieving object " + m_objectNames[i]);
			}
			else
			{
				MFnDependencyNode fnDepNode(depNode);
				MString type = fnDepNode.typeName();
				//printf("OBJ %s : type=%s \n", fnDepNode.name().asChar(), type.asChar());
				if ( type == "objectSet" ) setsDn.push_back( depNode );
			}
		}
		else
			objDb.push_back( objDagPath );
	}
	if ( !objDb.size() && !setsDn.size() )
	{
		MGlobal::displayError("[liqWriteArchive::doIt] no objetcs to export");
		return MS::kFailure;
	}

	// test that the output file is writable
	FILE *f = fopen( m_outputFilename.asChar(), "w" );
	if ( !f )
	{
		MGlobal::displayError( "[liqWriteArchive::doIt] Error writing to output file " + m_outputFilename + ". Check file permissions there" );
		return MS::kFailure;
	}
	fclose( f );

	// binary or ascii
#if defined( PRMAN ) || defined( DELIGHT )
	RtString format[ 1 ] = { "ascii" };
	if ( m_binaryRib ) format[ 0 ] = "binary";
	
	RiOption( "rib", "format", ( RtPointer )&format, RI_NULL);
#endif
	// write the RIB file
	if ( m_debug )
	{
		cout << "[liquidWriteArchive::doIt] Writing on file : " << m_outputFilename.asChar() << endl;
	}
	RiBegin( const_cast< RtToken >( m_outputFilename.asChar() ) );

	for ( i = 0 ; i < objDb.size() ; i++ )
	{
		if ( m_debug )
		{
			printf("[liqWriteArchive::doIt] Export object '%s' \n", m_objectNames[i].asChar());
		}
		writeObjectToRib( objDb[i], m_exportTransform );
	}
	for( i = 0; i < setsDn.size() ; i++ )
	{
		MFnSet fnSet(setsDn[i], &status);
		if ( !status )
		{
			MGlobal::displayWarning("[liqWriteArchive::doIt] Error init fnSet on object " + m_objectNames[i]);
			continue;
		}
		if ( m_debug )
		{
			printf("[liqWriteArchive::doIt] Export set '%s' \n", m_objectNames[i].asChar());
		}
		MSelectionList memberList;
		fnSet.getMembers(memberList, true);
		MDagPath objDagPath;
		for ( j = 0; j < memberList.length() ; j++ )
		{
			status = memberList.getDagPath(j, objDagPath);
			if ( m_debug )
			{
				printf("    - Export object '%s' \n", objDagPath.fullPathName().asChar());
			}
			writeObjectToRib( objDagPath, m_exportTransform );
		}
	}
	RiEnd();
	if ( m_debug )
	{
		printf("[liqWriteArchive::doIt] Export done \n");
	}
	return MS::kSuccess;
}

MStringArray liqWriteArchive::stringArrayRemoveDuplicates( MStringArray src )
{
	unsigned int i;
	unsigned int j;
	MStringArray dst;
	for( i = 0; i < src.length() ; i++ )
	{
		bool yetIn = 0;
		for ( j = 0 ; j < dst.length() ; j++ )
			if ( src[i] == dst[j] ) yetIn = 1;
		if ( !yetIn ) dst.append( src[ i ] );
	}
	return dst;
}

void liqWriteArchive::writeObjectToRib(const MDagPath &objDagPath, bool writeTransform)
{
	MStatus status;

	if ( !isObjectVisible( objDagPath ) ) return;

	if ( objDagPath.node().hasFn(MFn::kShape) || MFnDagNode( objDagPath ).typeName() == "liquidCoorSys" )
	{
		// we're looking at a shape node, so write out the geometry to the RIB
		if ( m_debug )
		{
			cout << "[liqWriteArchive::writeObjectToRib] writing shape object '" << objDagPath.fullPathName().asChar() <<"'"<< endl;
		}
		outputObjectName(objDagPath);
		liqRibNode ribNode;
		ribNode.set(objDagPath, 0, MRT_Unknown);

		MFnDagNode fnDagNode( objDagPath );

		// don't write out clipping planes
		if ( ribNode.object(0)->type == MRT_ClipPlane ) return;
		
		// write shading
		if ( !ribNode.object(0)->ignore )
		{
			if ( m_exportSurface )
			{
				if (	!ribNode.assignedShader.object().isNull() )
				{
					liqShader assignedShader( ribNode.assignedShader.object() );
					assignedShader.write( m_shortShaderNames, m_indentLevel );
				}
			}
			if ( m_exportDisplace )
			{
				if (	!ribNode.assignedDisp.object().isNull() )
				{
					liqShader assignedDisplace( ribNode.assignedDisp.object() );
					assignedDisplace.write( m_shortShaderNames, m_indentLevel );
				}
			}
			if ( m_exportVolume )
			{
				if (	!ribNode.assignedVolume.object().isNull() )
				{
					liqShader assignedVolume( ribNode.assignedVolume.object() );
					assignedVolume.write( m_shortShaderNames, m_indentLevel );
				}
			}
		}		

		// write ribboxs
		if ( ribNode.rib.box != "" && ribNode.rib.box != "-" )
		{
			outputIndentation();
			RiArchiveRecord( RI_COMMENT, "Additional RIB:\n%s", ribNode.rib.box.asChar() );
		}
		if ( ribNode.rib.readArchive != "" && ribNode.rib.readArchive != "-" )
		{
			// the following test prevents a really nasty infinite loop !!
			if ( ribNode.rib.readArchive != m_outputFilename )
			{
				outputIndentation();
				RiArchiveRecord( RI_COMMENT, "Read Archive Data: \nReadArchive \"%s\"", ribNode.rib.readArchive.asChar() );
			}
		}
		if ( ribNode.rib.delayedReadArchive != "" && ribNode.rib.delayedReadArchive != "-" )
		{
			// the following test prevents a really nasty infinite loop !!
			if ( ribNode.rib.delayedReadArchive != m_outputFilename )
			{
				outputIndentation();
				RiArchiveRecord( RI_COMMENT, "Delayed Read Archive Data: \nProcedural \"DelayedReadArchive\" [ \"%s\" ] [ %f %f %f %f %f %f ]", ribNode.rib.delayedReadArchive.asChar(), ribNode.bound[0], ribNode.bound[3], ribNode.bound[1], ribNode.bound[4], ribNode.bound[2], ribNode.bound[5]);
			}
		}
		// If it's a curve we should write the basis function
		if ( ribNode.object(0)->type == MRT_NuCurve )
		{
			outputIndentation();
			RiBasis( RiBSplineBasis, 1, RiBSplineBasis, 1 );
		}
		// write geometry
		if ( !ribNode.object(0)->ignore )
		{
			outputIndentation();
			ribNode.object(0)->writeObject();
		}
	}
	else
	{
		// we're looking for a transform node
		if ( m_debug )
		{
			cout << "[liqWriteArchive::writeObjectToRib] writing transform object '" << objDagPath.fullPathName().asChar() <<"'"<< endl;
		}
		bool wroteTransform = false;
		if ( writeTransform && ( objDagPath.apiType() == MFn::kTransform ) )
		{
			if ( m_debug )
			{
				cout << "liquidWriteArchive: writing transform: " << objDagPath.fullPathName().asChar() << endl;
			}
			// push the transform onto the RIB stack
			outputObjectName(objDagPath);
			//MFnDagNode mfnDag(objDagPath);
			//MMatrix tm = mfnDag.transformationMatrix();
			MMatrix tm = objDagPath.inclusiveMatrix(&status);
			if ( status != MS::kSuccess )
			{
				cout << "[liqWriteArchive::writeObjectToRib] error while getting transform for '" << objDagPath.fullPathName().asChar() <<"'"<< endl;
			}
			if ( true )   // (!tm.isEquivalent(MMatrix::identity)) {
			{
				RtMatrix riTM;
				tm.get(riTM);
				wroteTransform = true;
				outputIndentation();
				RiAttributeBegin();
				m_indentLevel++;
				outputIndentation();
				RiConcatTransform(riTM);
			}
		}
		// go through all the children of this node and deal with each of them
		int nChildren = objDagPath.childCount();
		if ( m_debug )
		{
			cout << "[liqWriteArchive::writeObjectToRib] object " << objDagPath.fullPathName().asChar() << " has " << nChildren << " children " << endl;
		}
		for ( int i=0; i<nChildren; ++i )
		{
			if ( m_debug )
			{
				cout << "[liqWriteArchive::writeObjectToRib] writing child number " << i << endl;
			}
			MDagPath childDagNode;
			MStatus stat = MDagPath::getAPathTo(objDagPath.child(i), childDagNode);
			if ( stat )
				writeObjectToRib(childDagNode, m_exportTransform);
			else
				MGlobal::displayWarning("[liqWriteArchive::writeObjectToRib] Error getting a dag path to child node of object " + objDagPath.fullPathName());
		}
		if ( wroteTransform )
		{
			m_indentLevel--;
			outputIndentation();
			RiAttributeEnd();
		}
	}
	if ( m_debug )
	{
		cout << "[liqWriteArchive::writeObjectToRib] finished writing object: " << objDagPath.fullPathName().asChar() << endl;
	}
}


//void liqWriteArchive::writeSurface(liqRibNode &ribNode)
//{
//	if(	ribNode.assignedShader.object().isNull() )
//	{
//		return;
//	}
//	liqShader assignedShader( ribNode.assignedShader.object() );
//	
//	//assignedShader.getCoShaders();
//	
//	scoped_array< RtToken > tokenArray( new RtToken[ assignedShader.tokenPointerArray.size() ] );
//	scoped_array< RtPointer > pointerArray( new RtPointer[ assignedShader.tokenPointerArray.size() ] );
//	assignTokenArrays( assignedShader.tokenPointerArray.size(), &assignedShader.tokenPointerArray[ 0 ], tokenArray.get(), pointerArray.get() );
//	char* shaderFileName;
//	LIQ_GET_SHADER_FILE_NAME( shaderFileName, m_shortShaderNames, assignedShader );
//	if( assignedShader.shaderSpace != "" )
//	{
//		RiTransformBegin();
//		RiCoordSysTransform( ( RtString )assignedShader.shaderSpace.asChar() );
//	}
//	// output shader
//	// its one less as the tokenPointerArray has a preset size of 1 not 0
//	int shaderParamCount = assignedShader.tokenPointerArray.size() - 1;
//	RiSurfaceV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
//	if( assignedShader.shaderSpace != "" )
//	{
//		RiTransformEnd();
//	}
//}
//
//
//void liqWriteArchive::writeDisplace(liqRibNode &ribNode)
//{
//	if(	ribNode.assignedDisp.object().isNull() )
//	{
//		return;
//	}
//	liqShader assignedShader( ribNode.assignedDisp.object() );
//	scoped_array< RtToken > tokenArray( new RtToken[ assignedShader.tokenPointerArray.size() ] );
//	scoped_array< RtPointer > pointerArray( new RtPointer[ assignedShader.tokenPointerArray.size() ] );
//	assignTokenArrays( assignedShader.tokenPointerArray.size(), &assignedShader.tokenPointerArray[ 0 ], tokenArray.get(), pointerArray.get() );
//	char* shaderFileName;
//	LIQ_GET_SHADER_FILE_NAME( shaderFileName, m_shortShaderNames, assignedShader );
//	if( assignedShader.shaderSpace != "" )
//	{
//		RiTransformBegin();
//		RiCoordSysTransform( ( RtString )assignedShader.shaderSpace.asChar() );
//	}
//	// output shader
//	// its one less as the tokenPointerArray has a preset size of 1 not 0
//	int shaderParamCount = assignedShader.tokenPointerArray.size() - 1;
//	RiDisplacementV( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
//	if( assignedShader.shaderSpace != "" )
//	{
//		RiTransformEnd();
//	}
//}
//
//
//void liqWriteArchive::writeVolume(liqRibNode &ribNode)
//{
//	if(	ribNode.assignedVolume.object().isNull() )
//	{
//		return;
//	}
//	liqShader assignedShader( ribNode.assignedVolume.object() );
//	scoped_array< RtToken > tokenArray( new RtToken[ assignedShader.tokenPointerArray.size() ] );
//	scoped_array< RtPointer > pointerArray( new RtPointer[ assignedShader.tokenPointerArray.size() ] );
//	assignTokenArrays( assignedShader.tokenPointerArray.size(), &assignedShader.tokenPointerArray[ 0 ], tokenArray.get(), pointerArray.get() );
//	char* shaderFileName;
//	LIQ_GET_SHADER_FILE_NAME( shaderFileName, m_shortShaderNames, assignedShader );
//	if( assignedShader.shaderSpace != "" )
//	{
//		RiTransformBegin();
//		RiCoordSysTransform( ( RtString )assignedShader.shaderSpace.asChar() );
//	}
//	// output shader
//	// its one less as the tokenPointerArray has a preset size of 1 not 0
//	int shaderParamCount = assignedShader.tokenPointerArray.size() - 1;
//	RiAtmosphereV( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
//	if( assignedShader.shaderSpace != "" )
//	{
//		RiTransformEnd();
//	}
//}


void liqWriteArchive::outputIndentation()
{
	for (unsigned int i=0; i<m_indentLevel; ++i) RiArchiveRecord(RI_VERBATIM, "\t");
}


void liqWriteArchive::outputObjectName(const MDagPath &objDagPath)
{
	MString name = sanitizeNodeName( objDagPath.fullPathName() );
	RiArchiveRecord(RI_VERBATIM, "\n");
	outputIndentation();
	RtString ribname = const_cast< char* >( name.asChar() );
	RiAttribute( "identifier", "name", &ribname, RI_NULL );
}
