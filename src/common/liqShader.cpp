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

#include <ri.h>

#include <maya/MPlug.h>
#include <maya/MDoubleArray.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MGlobal.h>
#include <maya/MPlugArray.h>
#include <maya/MFnMatrixData.h>

#include <liquid.h>
#include <liqShader.h>
#include <liqGlobalHelpers.h>
#include <liqMayaNodeIds.h>
#include <liqShaderFactory.h>

#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

extern int debugMode;

liqShader::liqShader() : liqGenericShader()
{
  //numTPV                = 0;
//  name                  = "";
  file                  = "";
  hasShadingRate        = false;
  shadingRate           = 1.0;
  hasDisplacementBound  = false;
  displacementBound     = 0.0;
  //outputInShadow        = false;
  //hasErrors             = false;
  shader_type           = SHADER_TYPE_UNKNOWN;
  volume_type           = VOLUME_TYPE_ATMOSPHERE;
  useVisiblePoints      = false;  // New for PPMAN 16.x: use VP.. shader version
  shaderSpace           = "";
  evaluateAtEveryFrame  = 0;
  tokenPointerArray.push_back( liqTokenPointer() ); // ENsure we have a 0 element
//  shaderHandler         = liqShaderFactory::instance().getUniqueShaderHandler();
//  m_outputAllParameters = false;
  m_previewGamma		= 1;
}

liqShader::liqShader( const liqShader& src ) : liqGenericShader( (const liqGenericShader&)src )
{
  //numTPV               = src.numTPV;
  tokenPointerArray    = src.tokenPointerArray;
//  name                 = src.name;
  file                 = src.file;
  hasShadingRate       = src.hasShadingRate;
  shadingRate          = src.shadingRate;
  hasDisplacementBound = src.hasDisplacementBound;
  displacementBound    = src.displacementBound;
//  outputInShadow       = src.outputInShadow;
  //hasErrors            = src.hasErrors;
  shader_type          = src.shader_type;
  volume_type          = src.volume_type;
  useVisiblePoints     = src.useVisiblePoints;
  shaderSpace          = src.shaderSpace;
  evaluateAtEveryFrame = src.evaluateAtEveryFrame;
//  shaderHandler        = src.shaderHandler;
//  m_mObject            = src.m_mObject;
//  m_outputAllParameters= src.m_outputAllParameters;
  m_previewGamma       = src.m_previewGamma;
}

liqShader & liqShader::operator=( const liqShader & src )
{
  tokenPointerArray     = src.tokenPointerArray;
  name                  = src.name;
  file                  = src.file;
  rmColor[0]            = src.rmColor[0];
  rmColor[1]            = src.rmColor[1];
  rmColor[2]            = src.rmColor[2];
  rmOpacity[0]          = src.rmOpacity[0];
  rmOpacity[1]          = src.rmOpacity[1];
  rmOpacity[2]          = src.rmOpacity[2];
  hasShadingRate        = src.hasShadingRate;
  shadingRate           = src.shadingRate;
  hasDisplacementBound  = src.hasDisplacementBound;
  displacementBound     = src.displacementBound;
  outputInShadow        = src.outputInShadow;
  hasErrors             = src.hasErrors;
  shader_type           = src.shader_type;
  volume_type           = src.volume_type;
  useVisiblePoints     = src.useVisiblePoints;
  shaderSpace           = src.shaderSpace;
  evaluateAtEveryFrame  = src.evaluateAtEveryFrame;
  shaderHandler         = src.shaderHandler;
  m_mObject             = src.m_mObject;
  m_outputAllParameters = src.m_outputAllParameters;
  m_previewGamma        = src.m_previewGamma;
  return *this;
}


liqShader::liqShader( MObject shaderObj, bool outputAllParameters ) : liqGenericShader(shaderObj, outputAllParameters)
{
	MString rmShaderStr;
	MStatus status;
	MFnDependencyNode shaderNode( shaderObj );
	MPlug rmanShaderNamePlug = shaderNode.findPlug( MString( "rmanShaderLong" ) );
	rmanShaderNamePlug.getValue( rmShaderStr );

	LIQDEBUGPRINTF( "-> Using Renderman Shader %s. \n", rmShaderStr.asChar() );

	unsigned numArgs;
	//numTPV = 0;
	hasShadingRate = false;
	hasDisplacementBound = false;
	outputInShadow = false;
	hasErrors = false;
	tokenPointerArray.push_back( liqTokenPointer() );

	file = rmShaderStr.substring( 0, rmShaderStr.length() - 5 ).asChar();

	rmColor[0]            = 1.0;
	rmColor[1]            = 1.0;
	rmColor[2]            = 1.0;
	rmOpacity[0]          = 1.0;
	rmOpacity[1]          = 1.0;
	rmOpacity[2]          = 1.0;

	liqGetSloInfo shaderInfo;

// commented out for it generates errors - Alf
	int success = ( shaderInfo.setShaderNode( shaderNode ) );
	if ( !success )
	{
		liquidMessage( "Problem using shader '" + shaderNode.name() + "'", messageError );
		rmColor[0] = 1.0;
		rmColor[1] = 0.0;
		rmColor[2] = 0.0;
		name = "plastic";
//		numTPV = 0;
		hasErrors = true;
	}
	else
	{
		/* Used to handling shading rates set in the surface shader,
		this is a useful way for shader writers to ensure that their
		shaders are always rendered as they were designed.  This value
		overrides the global shading rate but gets overridden with the
		node specific shading rate. */

		shader_type = shaderInfo.getType();
		// Set RiColor and RiOpacity
		status.clear();
		MPlug colorPlug = shaderNode.findPlug( "color" );
		if ( MS::kSuccess == status )
		{
			colorPlug.child(0).getValue( rmColor[0] );
			colorPlug.child(1).getValue( rmColor[1] );
			colorPlug.child(2).getValue( rmColor[2] );
		}

		status.clear();
		MPlug opacityPlug( shaderNode.findPlug( "opacity" ) );
		// Moritz: changed opacity from float to color in MEL
		if ( MS::kSuccess == status )
		{
		  opacityPlug.child(0).getValue( rmOpacity[0] );
		  opacityPlug.child(1).getValue( rmOpacity[1] );
		  opacityPlug.child(2).getValue( rmOpacity[2] );
		}
    int volumeType = volume_type; 
    liquidGetPlugValue( shaderNode, "volumeType", volumeType, status );
    if ( MS::kSuccess == status ) volume_type = (VOLUME_TYPE)volumeType;

    liquidGetPlugValue( shaderNode, "useVisiblePoints", useVisiblePoints, status );

    liquidGetPlugValue( shaderNode, "shaderSpace", shaderSpace, status );
    liquidGetPlugValue( shaderNode, "outputInShadow", outputInShadow, status );
    liquidGetPlugValue( shaderNode, "evaluateAtEveryFrame", evaluateAtEveryFrame, status );
    liquidGetPlugValue( shaderNode, "previewGamma", m_previewGamma, status );

		// find the parameter details and declare them in the rib stream
		numArgs = shaderInfo.getNumParam();
		for ( unsigned int i( 0 ) ; i < numArgs ; i++ )
		{
			MString paramName = shaderInfo.getArgName(i);
			int arraySize = shaderInfo.getArgArraySize(i);
			SHADER_TYPE shaderParameterType = shaderInfo.getArgType(i);
			SHADER_DETAIL shaderDetail = shaderInfo.getArgDetail(i);
			MString shaderAccept = shaderInfo.getArgAccept(i);
			if ( shaderParameterType == SHADER_TYPE_STRING )
			{
				// check if a string must be used as a shader
				if ( shaderAccept != "" )
				{
					shaderParameterType = SHADER_TYPE_SHADER;
				}
			}

			bool skipToken = false;
			if ( paramName == "liquidShadingRate" )
			{
				// BUGFIX: Monday 6th August - fixed shading rate bug where it only accepted the default value
				MPlug floatPlug = shaderNode.findPlug( paramName, &status );
				if ( MS::kSuccess == status )
				{
					float floatPlugVal;
					floatPlug.getValue( floatPlugVal );
					shadingRate = floatPlugVal;
				}
				else
					shadingRate = shaderInfo.getArgFloatDefault( i, 0 );
				
				hasShadingRate = true;
				continue;
			}
			
			if ( shaderInfo.isOutputParameter(i) && !outputAllParameters )   // throw output parameters
			{
				continue;		
			}
			else if ( shaderInfo.isOutputParameter(i) && outputAllParameters )
			{
				if( arraySize == -1 )    // single value
				{
					switch ( shaderParameterType )
					{
						case SHADER_TYPE_SHADER:
						{
							printf ( "[liqShader] warning cannot write output shader parameters yet. skip param %s on %s\n", paramName.asChar(), shaderNode.name().asChar() );
							continue;
						}
						case SHADER_TYPE_STRING:
						{
							ParameterType parameterType = rString;
							MString s = shaderInfo.getArgStringDefault( i, 0 );
							tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType );
							tokenPointerArray.rbegin()->setTokenString( 0, s.asChar() );
							break;
						}
						case SHADER_TYPE_SCALAR:
						{
							ParameterType parameterType = rFloat;
							float x = shaderInfo.getArgFloatDefault( i, 0 );
							tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType );
							tokenPointerArray.rbegin()->setTokenFloat( 0, x );
							break;
						}
						case SHADER_TYPE_COLOR:
						case SHADER_TYPE_POINT:
						case SHADER_TYPE_VECTOR:
						case SHADER_TYPE_NORMAL:
						{
							ParameterType parameterType;
							if ( shaderParameterType == SHADER_TYPE_COLOR )
							{
								parameterType = rColor;
							}
							else if ( shaderParameterType == SHADER_TYPE_POINT )
							{
								parameterType = rPoint;
							}
							else if ( shaderParameterType == SHADER_TYPE_VECTOR )
							{
								parameterType = rVector;
							}
							else if ( shaderParameterType == SHADER_TYPE_NORMAL )
							{
								parameterType = rNormal;
							}
							float x = shaderInfo.getArgFloatDefault( i, 0 );
							float y = shaderInfo.getArgFloatDefault( i, 1 );
							float z = shaderInfo.getArgFloatDefault( i, 2 );
							tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType );
							tokenPointerArray.rbegin()->setTokenFloat( 0, x, y, z );
							break;
						}
						case SHADER_TYPE_MATRIX:
						{
							printf ( "[liqShader] warning cannot write output matrix parameters yet. skip param %s on %s\n", paramName.asChar(), shaderNode.name().asChar() );
							continue;
						}
						default:
						{
							printf ( "[liqShader] warning unhandled parameters type. skip param %s on %s\n", paramName.asChar(), shaderNode.name().asChar() );
							continue;
						}
					}
				}
				else
				{
					printf ( "[liqShader] warning cannot write output array parameters yet. skip param %s on %s\n", paramName.asChar(), shaderNode.name().asChar() );
					continue;
				}
			}
			else
			{
				switch ( shaderParameterType )
				{
					case SHADER_TYPE_SHADER:
					{
						ParameterType parameterType = rString;  // rShader

						MPlug coShaderPlug = shaderNode.findPlug( paramName, &status );

						if ( status != MS::kSuccess )
						{
							skipToken = true;
							printf ( "[liqShader] error while building shader param %s on %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
						}
						else
						{
							if ( arraySize == 0 )    // dynamic array
							{
								MIntArray indices;
								coShaderPlug.getExistingArrayAttributeIndices(indices);
								if( indices.length() == 0 )
								{
									skipToken = true;
								}
								else
								{
									int maxIndex = 0;
									for ( unsigned int kk( 0 ); kk<indices.length() ; kk++ )
									{
										if ( indices[kk] > maxIndex )
										{
											maxIndex = indices[kk];
										}
									}
									arraySize = maxIndex + 1;								
									tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType, arraySize );
									for ( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
									{
										bool existingIndex = false;
										for (unsigned int kkk( 0 ); kkk<indices.length(); kkk++)
										{
											if ( kk == indices[kkk] )
											{
												existingIndex = true;
												continue;
											}
										}
										if ( existingIndex )  // get plug value
										{
											MPlug argNameElement = coShaderPlug.elementByLogicalIndex(kk);
											MString coShaderHandler;
											
											if ( argNameElement.isConnected() )
											{
												bool asSrc = 0;
												bool asDst = 1;
												MPlugArray connectedPlugArray;
												argNameElement.connectedTo( connectedPlugArray, asDst, asSrc );
												
												MObject coshader = connectedPlugArray[0].node();
												appendCoShader(coshader, connectedPlugArray[0]);
												coShaderHandler = liqShaderFactory::instance().getShaderId(coshader);
											}
											else
											{
												coShaderHandler = "";
											}									
											tokenPointerArray.rbegin()->setTokenString( kk, coShaderHandler.asChar() );
										}
										else  // don't mind about value
										{
											tokenPointerArray.rbegin()->setTokenString( kk, "" );
										}
									}
								}
							}
							else if ( arraySize > 0 )    // static array
							{
								vector<MString> coShaderHandlers;

								for ( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
								{
									MPlug argNameElement = coShaderPlug.elementByLogicalIndex(kk);
									MString coShaderHandler;
									if ( argNameElement.isConnected() )
									{
										bool asSrc = 0;
										bool asDst = 1;
										MPlugArray connectedPlugArray;
										argNameElement.connectedTo( connectedPlugArray, asDst, asSrc );
										
										MObject coshader = connectedPlugArray[0].node();
										appendCoShader(coshader, connectedPlugArray[0]);
										coShaderHandler = liqShaderFactory::instance().getShaderId(coshader);
									}
									else
									{
										coShaderHandler = "";
									}
									coShaderHandlers.push_back(coShaderHandler);
								}
								
								int isDefault = 1;
								for( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
								{
									if ( coShaderHandlers[kk] != "" )
									{
										isDefault = 0;
										continue;
									}
								}
								if ( isDefault && !outputAllParameters )  // skip default
								{
									skipToken = true;
								}
								else  // build non default param
								{
									tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType, arraySize );
									for( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
									{
										tokenPointerArray.rbegin()->setTokenString( kk, coShaderHandlers[kk].asChar() );
									}
								}
							}
							else if ( arraySize == -1 )    // single value
							{
								MPlugArray connectionArray;
								bool asSrc = 0;
								bool asDst = 1;
								coShaderPlug.connectedTo(connectionArray, asDst, asSrc);
								if ( connectionArray.length() == 0 )
								{
									skipToken = true;
								}
								else
								{
									MPlug connectedPlug = connectionArray[0];
									MObject coshader = connectedPlug.node();
									appendCoShader(coshader, coShaderPlug);
									MString coShaderId = liqShaderFactory::instance().getShaderId(coshader);
									if ( coShaderId == "" )
									{
										skipToken = true;
									}
									else
									{
										tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType );
										tokenPointerArray.rbegin()->setTokenString( 0, coShaderId.asChar() );
									}
								}
							}
							else    // unknown type
							{
								skipToken = true;
								printf("[liqShader] error while building shader param %s on %s : undefined array size %d \n", paramName.asChar(), shaderNode.name().asChar(), arraySize );
							}
						}
						break;
					}
					case SHADER_TYPE_STRING:
					{
						MPlug stringPlug = shaderNode.findPlug( paramName, &status );
						if ( status != MS::kSuccess )
						{
							skipToken = true;
							printf("[liqShader] error while building string param %s on %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
						}
						else
						{
							if ( arraySize == 0 )    // dynamic array
							{
								MIntArray indices;
								stringPlug.getExistingArrayAttributeIndices(indices);
								if ( indices.length() == 0 )
								{
									skipToken = true;
								}
								else
								{
									int maxIndex = 0;
									for ( unsigned int kk( 0 ); kk < indices.length(); kk++ )
									{
										if( indices[kk]>maxIndex )
										{
											maxIndex = indices[kk];
										}
									}
									arraySize = maxIndex + 1;

									tokenPointerArray.rbegin()->set( paramName.asChar(), rString, arraySize );
									for ( unsigned int kk( 0 ) ; kk < (unsigned int)arraySize ; kk++ )
									{
										bool existingIndex = false;
										for ( unsigned int kkk( 0 ) ; kkk < indices.length() ; kkk++ )
										{
											if ( kk == indices[kkk] )
											{
												existingIndex = true;
												continue;
											}
										}
										if ( existingIndex )  // get plug value
										{
											MPlug argNameElement = stringPlug.elementByLogicalIndex(kk);
											MString stringPlugVal;
											argNameElement.getValue( stringPlugVal );
											MString stringVal = parseString( stringPlugVal );
											tokenPointerArray.rbegin()->setTokenString( kk, stringVal.asChar() );
										}
										else  // don't mind about value
										{
											tokenPointerArray.rbegin()->setTokenString( kk, "" );
										}
									}
								}
							}
							else if ( arraySize > 0 )    // static array
							{
								bool isArrayAttr( stringPlug.isArray( &status ) );
								if ( isArrayAttr )
								{
									MPlug plugObj;
									// check default
									int isDefault = 1;
									for ( unsigned int kk( 0 ) ; kk < (unsigned int)arraySize ; kk++ )
									{
										plugObj = stringPlug.elementByLogicalIndex( kk, &status );
										MString stringDefault( shaderInfo.getArgStringDefault( i, kk ) );
										if ( plugObj.asString() != stringDefault )
										{
											isDefault = 0;
											continue;
										}
									}
									if ( isDefault && !outputAllParameters )  // skip default
									{
										skipToken = true;
									}
									else  // build non default param
									{
										tokenPointerArray.rbegin()->set( paramName.asChar(), rString, arraySize );
										for ( unsigned int kk( 0 ) ; kk < (unsigned int)arraySize ; kk++ )
										{
											plugObj = stringPlug.elementByLogicalIndex( kk, &status );
											if ( MS::kSuccess == status )
											{
												MString stringPlugVal;
												plugObj.getValue( stringPlugVal );
												MString stringVal = parseString( stringPlugVal );
												tokenPointerArray.rbegin()->setTokenString( kk, stringVal.asChar() );
											}
											else
											{
												printf("[liqShader] error while building param %d : %s \n", kk, stringPlug.name().asChar() );
											}
										}
									}
								}
								else
								{
									printf("[liqShader] error while building string param %s assumed as an array but wasn't...\n", stringPlug.name().asChar() );
								}
							}
							else if ( arraySize == -1 )    // single value
							{
								MString stringPlugVal;
								stringPlug.getValue( stringPlugVal );
								MString stringDefault( shaderInfo.getArgStringDefault( i, 0 ) );
								if ( stringPlugVal == stringDefault && !outputAllParameters )  // skip default
								{
									skipToken = true;
								}
								else  // build non default param
								{
									MString stringVal( parseString( stringPlugVal ) );
									LIQDEBUGPRINTF("[liqShader::liqShader] parsed string for param %s = %s \n", paramName.asChar(), stringVal.asChar() );
									tokenPointerArray.rbegin()->set( paramName.asChar(), rString );
									tokenPointerArray.rbegin()->setTokenString( 0, stringVal.asChar() );
								}
							}
							else    // unknown type
							{
								skipToken = true;
								printf("[liqShader] error while building string param %s on %s : undefined array size %d \n", paramName.asChar(), shaderNode.name().asChar(), arraySize );
							}
						}
						break;
					}
					case SHADER_TYPE_SCALAR:
					{
						MPlug floatPlug( shaderNode.findPlug( paramName, &status ) );
						if ( status != MS::kSuccess )
						{
							skipToken = true;
							printf("[liqShader] error while building float param %s on %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
						}
						else
						{
							if ( arraySize == 0 )    // dynamic array
							{
								MIntArray indices;
								floatPlug.getExistingArrayAttributeIndices(indices);
								if ( indices.length() == 0 )
								{
									skipToken = true;
								}
								else
								{
									int maxIndex = 0;
									for ( unsigned int kk( 0 ) ; kk<indices.length() ; kk++ )
									{
										if ( indices[kk]>maxIndex )
										{
											maxIndex = indices[kk];
										}
									}
									arraySize = maxIndex + 1;

									tokenPointerArray.rbegin()->set( paramName.asChar(), rFloat, false, true, arraySize );
									for ( unsigned int kk( 0  ); kk < (unsigned int)arraySize ; kk++ )
									{
										bool existingIndex = false;
										for ( unsigned int kkk( 0 ) ; kkk<indices.length() ; kkk++ )
										{
											if ( kk == indices[kkk] )
											{
												existingIndex = true;
												continue;
											}
										}
										if ( existingIndex )  // get plug value
										{
											MPlug argNameElement = floatPlug.elementByLogicalIndex(kk);
											float value = argNameElement.asFloat();
											tokenPointerArray.rbegin()->setTokenFloat( kk, value );
										}
										else  // don't mind about value
										{
											tokenPointerArray.rbegin()->setTokenFloat( kk, 0 );
										}
									}
								}
							}
							else if ( arraySize > 0 )    // static array
							{
								bool isArrayAttr( floatPlug.isArray( &status ) );
								if ( isArrayAttr )
								{
									MPlug plugObj;
									// check default
									int isDefault = 1;
									for ( unsigned int kk( 0 ) ; kk < (unsigned int)arraySize ; kk++ )
									{
										plugObj = floatPlug.elementByLogicalIndex( kk, &status );
										float floatDefault = shaderInfo.getArgFloatDefault( i, kk );
										if ( plugObj.asFloat() != floatDefault )
										{
											isDefault = 0;
											continue;
										}
									}
									if ( isDefault && !outputAllParameters ) // skip default
									{
										skipToken = true;
									}
									else  // build non default param
									{
										tokenPointerArray.rbegin()->set( paramName.asChar(), rFloat, false, true, arraySize );
										for ( unsigned int kk( 0 ) ; kk < (unsigned int)arraySize ; kk++ )
										{
											plugObj = floatPlug.elementByLogicalIndex( kk, &status );
											if ( MS::kSuccess == status )
											{
												float x;
												plugObj.getValue( x );
												tokenPointerArray.rbegin()->setTokenFloat( kk, x );
											}
										}
									}
								}
							}
							else if ( arraySize == -1 )    // single value
							{
								float floatPlugVal;
								floatPlug.getValue( floatPlugVal );
								float floatDefault( shaderInfo.getArgFloatDefault( i, 0 ) );
								if ( floatPlugVal == floatDefault && !outputAllParameters )  // skip default
								{
									skipToken = true;
								}
								else  // build non default param
								{
									tokenPointerArray.rbegin()->set( paramName.asChar(), rFloat );
									tokenPointerArray.rbegin()->setTokenFloat( 0, floatPlugVal );
								}
							}
							else    // unknown type
							{
								skipToken = true;
								printf("[liqShader] error while building float param %s on %s : undefined array size %d \n", paramName.asChar(), shaderNode.name().asChar(), arraySize );
							}
						}
						break;
					}
					case SHADER_TYPE_COLOR:
					case SHADER_TYPE_POINT:
					case SHADER_TYPE_VECTOR:
					case SHADER_TYPE_NORMAL:
					{
						ParameterType parameterType;
						if ( shaderParameterType == SHADER_TYPE_COLOR )
						{
							parameterType = rColor;
						}
						else if (shaderParameterType == SHADER_TYPE_POINT)
						{
							parameterType = rPoint;
						}
						else if (shaderParameterType == SHADER_TYPE_VECTOR)
						{
							parameterType = rVector;
						}
						else if (shaderParameterType == SHADER_TYPE_NORMAL)
						{
							parameterType = rNormal;
						}
						MPlug triplePlug( shaderNode.findPlug( paramName, true, &status ) );
						if ( status != MS::kSuccess )
						{
							skipToken = true;
							printf("[liqShader] error while building float[3] param %s on %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
						}
						else
						{
							if ( arraySize == 0 )    // dynamic array
							{
								MIntArray indices;
								triplePlug.getExistingArrayAttributeIndices(indices);
								if ( indices.length() == 0 )
								{
									skipToken = true;
								}
								else
								{
									int maxIndex = 0;
									for ( unsigned int kk( 0 ) ; kk<indices.length( ); kk++ )
									{
										if ( indices[kk]>maxIndex )
										{
											maxIndex = indices[kk];
										}
									}
									arraySize = maxIndex + 1;

									tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType, false, true, arraySize );
									for ( unsigned int kk( 0 ) ; kk < (unsigned int)arraySize ; kk++ )
									{
										bool existingIndex = false;
										for ( unsigned int kkk( 0 ) ; kkk<indices.length() ; kkk++ )
										{
											if ( kk == indices[kkk] )
											{
												existingIndex = true;
												continue;
											}
										}
										if ( existingIndex )  // get plug value
										{
											MPlug argNameElement = triplePlug.elementByLogicalIndex(kk);
											float x, y, z;
											argNameElement.child( 0 ).getValue( x );
											argNameElement.child( 1 ).getValue( y );
											argNameElement.child( 2 ).getValue( z );
											tokenPointerArray.rbegin()->setTokenFloat( kk, x, y, z );
										}
										else  // don't mind about value
										{
											tokenPointerArray.rbegin()->setTokenFloat( kk, 0, 0, 0 );
										}
									}
								}
							}
							else if ( arraySize > 0 )    // static array
							{
								// check default
								int isDefault = 1;
								for ( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
								{
									MPlug argNameElement( triplePlug.elementByLogicalIndex( kk ) );
									float x, y, z;
									argNameElement.child( 0 ).getValue( x );
									argNameElement.child( 1 ).getValue( y );
									argNameElement.child( 2 ).getValue( z );
									float xDefault, yDefault, zDefault;
									xDefault = shaderInfo.getArgFloatDefault(i, (kk*3)+0);
									yDefault = shaderInfo.getArgFloatDefault(i, (kk*3)+1);
									zDefault = shaderInfo.getArgFloatDefault(i, (kk*3)+2);
									if ( x!=xDefault || y!=yDefault || z!=zDefault )
									{
										isDefault = 0;
										continue;
									}
								}
								if ( isDefault && !outputAllParameters ) // skip default
								{
									skipToken = true;
								}
								else  // build non default param
								{
									tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType, false, true, arraySize );
									for ( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
									{
										MPlug argNameElement( triplePlug.elementByLogicalIndex( kk ) );
										float x, y, z;
										argNameElement.child( 0 ).getValue( x );
										argNameElement.child( 1 ).getValue( y );
										argNameElement.child( 2 ).getValue( z );
										tokenPointerArray.rbegin()->setTokenFloat( kk, x, y, z );
									}
								}
							}
							else if ( arraySize == -1 )     // single value
							{
								// check default
								float x, y, z;
								triplePlug.child( 0 ).getValue( x );
								triplePlug.child( 1 ).getValue( y );
								triplePlug.child( 2 ).getValue( z );
								float xDefault, yDefault, zDefault;
								xDefault = shaderInfo.getArgFloatDefault(i, 0);
								yDefault = shaderInfo.getArgFloatDefault(i, 1);
								zDefault = shaderInfo.getArgFloatDefault(i, 2);

								if ( ( x == xDefault && y == yDefault && z == zDefault) && !outputAllParameters ) // skip default
								{
									skipToken = true;
								}
								else  // build non default param
								{
									tokenPointerArray.rbegin()->set( paramName.asChar(), parameterType );
									tokenPointerArray.rbegin()->setTokenFloat( 0, x, y, z );
								}
							}
							else    // unknown type
							{
								skipToken = true;
								printf ( "[liqShader] error while building float[3] param %s on %s : undefined array size %d \n", paramName.asChar(), shaderNode.name().asChar(), arraySize );
							}
						}
						break;
					}
					case SHADER_TYPE_MATRIX:
					{
						MPlug matrixPlug( shaderNode.findPlug( paramName, &status ) );
						if ( MS::kSuccess != status )
						{
							skipToken = true;
							printf ( "[liqShader] error while building float[16] param %s on %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
						}
						else
						{
							if ( arraySize == 0 )    // dynamic array
							{
								MIntArray indices;
								matrixPlug.getExistingArrayAttributeIndices(indices);
								if ( indices.length() == 0 )
								{
									skipToken = true;
								}
								else
								{
									int maxIndex = 0;
									for ( unsigned int kk( 0 ); kk < indices.length(); kk++ )
									{
										if ( indices[kk] > maxIndex )
										{
											maxIndex = indices[kk];
										}
									}
									arraySize = maxIndex + 1;

									tokenPointerArray.rbegin()->set( paramName.asChar(), rMatrix, false, true, arraySize );
									for ( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
									{
										bool existingIndex = false;
										for ( unsigned int kkk( 0 ); kkk < indices.length(); kkk++ )
										{
											if ( kk == indices[kkk] )
											{
												existingIndex = true;
												continue;
											}
										}
										if ( existingIndex )  // get plug value
										{
											MPlug argNameElement = matrixPlug.elementByLogicalIndex(kk);
											MObject matrixObject = argNameElement.asMObject(MDGContext::fsNormal, &status);
											MFnMatrixData matrixData(matrixObject, &status);
											if ( status != MS::kSuccess )
											{
												skipToken = true;
												printf ( "[liqShader] error while initializing MFnMatrixData on param[?] %s on shader %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
												continue;
											}
											else
											{
												MMatrix matrix = matrixData.matrix();
												float x1, y1, z1, w1;
												float x2, y2, z2, w2;
												float x3, y3, z3, w3;
												float x4, y4, z4, w4;
												x1 = matrix(0, 0);
												y1 = matrix(0, 1);
												z1 = matrix(0, 2);
												w1 = matrix(0, 3);
												x2 = matrix(1, 0);
												y2 = matrix(1, 1);
												z2 = matrix(1, 2);
												w2 = matrix(1, 3);
												x3 = matrix(2, 0);
												y3 = matrix(2, 1);
												z3 = matrix(2, 2);
												w3 = matrix(2, 3);
												x4 = matrix(3, 0);
												y4 = matrix(3, 1);
												z4 = matrix(3, 2);
												w4 = matrix(3, 3);
												tokenPointerArray.rbegin()->setTokenFloat( kk, x1, y1, z1, w1, x2, y2, z2, w2, x3, y3, z3, w3, x4, y4, z4, w4 );
											}
										}
										else  // don't mind about value
										{
											tokenPointerArray.rbegin()->setTokenFloat( kk, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
										}
									}
								}
							}
							else if ( arraySize > 0 )    // static array
							{
								// check default
								int isDefault = 1;
								for ( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
								{
									MPlug argNameElement( matrixPlug.elementByLogicalIndex( kk ) );
									MObject matrixObject = argNameElement.asMObject(MDGContext::fsNormal, &status);
									MFnMatrixData matrixData(matrixObject, &status);
									if ( status != MS::kSuccess )
									{
										skipToken = true;
										printf ( "[liqShader] error while initializing MFnMatrixData on param[] %s on shader %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
									}
									else
									{
										MMatrix matrix = matrixData.matrix();
										MMatrix defaultMatrix;
										defaultMatrix(0, 0) = shaderInfo.getArgFloatDefault(i, (kk*16)+0);
										defaultMatrix(0, 1) = shaderInfo.getArgFloatDefault(i, (kk*16)+1);
										defaultMatrix(0, 2) = shaderInfo.getArgFloatDefault(i, (kk*16)+2);
										defaultMatrix(0, 3) = shaderInfo.getArgFloatDefault(i, (kk*16)+3);
										defaultMatrix(1, 0) = shaderInfo.getArgFloatDefault(i, (kk*16)+4);
										defaultMatrix(1, 1) = shaderInfo.getArgFloatDefault(i, (kk*16)+5);
										defaultMatrix(1, 2) = shaderInfo.getArgFloatDefault(i, (kk*16)+6);
										defaultMatrix(1, 3) = shaderInfo.getArgFloatDefault(i, (kk*16)+7);
										defaultMatrix(2, 0) = shaderInfo.getArgFloatDefault(i, (kk*16)+8);
										defaultMatrix(2, 1) = shaderInfo.getArgFloatDefault(i, (kk*16)+9);
										defaultMatrix(2, 2) = shaderInfo.getArgFloatDefault(i, (kk*16)+10);
										defaultMatrix(2, 3) = shaderInfo.getArgFloatDefault(i, (kk*16)+11);
										defaultMatrix(3, 0) = shaderInfo.getArgFloatDefault(i, (kk*16)+12);
										defaultMatrix(3, 1) = shaderInfo.getArgFloatDefault(i, (kk*16)+13);
										defaultMatrix(3, 2) = shaderInfo.getArgFloatDefault(i, (kk*16)+14);
										defaultMatrix(3, 3) = shaderInfo.getArgFloatDefault(i, (kk*16)+15);
										if ( matrix != defaultMatrix )
										{
											isDefault = 0;
											continue;
										}
									}
								}
								if ( isDefault && !outputAllParameters ) // skip default
								{
									skipToken = true;
								}
								else  // build non default param
								{
									tokenPointerArray.rbegin()->set( paramName.asChar(), rMatrix, false, true, arraySize );
									for ( unsigned int kk( 0 ); kk < (unsigned int)arraySize; kk++ )
									{
										MPlug argNameElement( matrixPlug.elementByLogicalIndex( kk ) );
										MObject matrixObject = argNameElement.asMObject(MDGContext::fsNormal, &status);
										MFnMatrixData matrixData(matrixObject, &status);
										MMatrix matrix = matrixData.matrix();
										float x1, y1, z1, w1;
										float x2, y2, z2, w2;
										float x3, y3, z3, w3;
										float x4, y4, z4, w4;
										x1 = matrix(0, 0);
										y1 = matrix(0, 1);
										z1 = matrix(0, 2);
										w1 = matrix(0, 3);
										x2 = matrix(1, 0);
										y2 = matrix(1, 1);
										z2 = matrix(1, 2);
										w2 = matrix(1, 3);
										x3 = matrix(2, 0);
										y3 = matrix(2, 1);
										z3 = matrix(2, 2);
										w3 = matrix(2, 3);
										x4 = matrix(3, 0);
										y4 = matrix(3, 1);
										z4 = matrix(3, 2);
										w4 = matrix(3, 3);
										tokenPointerArray.rbegin()->setTokenFloat( kk, x1, y1, z1, w1, x2, y2, z2, w2, x3, y3, z3, w3, x4, y4, z4, w4 );
									}
								}
							}
							else if ( arraySize == -1 )    // single value
							{
								// check default
								MObject matrixObject = matrixPlug.asMObject(MDGContext::fsNormal, &status);
								MFnMatrixData matrixData(matrixObject, &status);
								if ( status != MS::kSuccess )
								{
									skipToken = true;
									printf("[liqShader] error while initializing MFnMatrixData on param %s on shader %s ...\n", paramName.asChar(), shaderNode.name().asChar() );
								}
								else
								{
									MMatrix matrix = matrixData.matrix();
									MMatrix defaultMatrix;
									defaultMatrix(0, 0) = shaderInfo.getArgFloatDefault(i, 0);
									defaultMatrix(0, 1) = shaderInfo.getArgFloatDefault(i, 1);
									defaultMatrix(0, 2) = shaderInfo.getArgFloatDefault(i, 2);
									defaultMatrix(0, 3) = shaderInfo.getArgFloatDefault(i, 3);
									defaultMatrix(1, 0) = shaderInfo.getArgFloatDefault(i, 4);
									defaultMatrix(1, 1) = shaderInfo.getArgFloatDefault(i, 5);
									defaultMatrix(1, 2) = shaderInfo.getArgFloatDefault(i, 6);
									defaultMatrix(1, 3) = shaderInfo.getArgFloatDefault(i, 7);
									defaultMatrix(2, 0) = shaderInfo.getArgFloatDefault(i, 8);
									defaultMatrix(2, 1) = shaderInfo.getArgFloatDefault(i, 9);
									defaultMatrix(2, 2) = shaderInfo.getArgFloatDefault(i, 10);
									defaultMatrix(2, 3) = shaderInfo.getArgFloatDefault(i, 11);
									defaultMatrix(3, 0) = shaderInfo.getArgFloatDefault(i, 12);
									defaultMatrix(3, 1) = shaderInfo.getArgFloatDefault(i, 13);
									defaultMatrix(3, 2) = shaderInfo.getArgFloatDefault(i, 14);
									defaultMatrix(3, 3) = shaderInfo.getArgFloatDefault(i, 15);
									if ( matrix == defaultMatrix && !outputAllParameters )  // skip default
									{
										skipToken = true;
									}
									else  // build non default param
									{
										float x1, y1, z1, w1;
										float x2, y2, z2, w2;
										float x3, y3, z3, w3;
										float x4, y4, z4, w4;
										x1 = matrix(0, 0);
										y1 = matrix(0, 1);
										z1 = matrix(0, 2);
										w1 = matrix(0, 3);
										x2 = matrix(1, 0);
										y2 = matrix(1, 1);
										z2 = matrix(1, 2);
										w2 = matrix(1, 3);
										x3 = matrix(2, 0);
										y3 = matrix(2, 1);
										z3 = matrix(2, 2);
										w3 = matrix(2, 3);
										x4 = matrix(3, 0);
										y4 = matrix(3, 1);
										z4 = matrix(3, 2);
										w4 = matrix(3, 3);
										tokenPointerArray.rbegin()->set( paramName.asChar(), rMatrix );
										//printf("SET MATRIX : \n %f %f %f %f \n %f %f %f %f \n %f %f %f %f \n %f %f %f %f \n", x1, y1, z1, w1, x2, y2, z2, w2, x3, y3, z3, w3, x4, y4, z4, w4);
										tokenPointerArray.rbegin()->setTokenFloat( 0, x1, y1, z1, w1, x2, y2, z2, w2, x3, y3, z3, w3, x4, y4, z4, w4 );
									}
								}
							}
							else    // unknown type
							{
								skipToken = true;
								printf ( "[liqShader] error while building float[16] param %s on %s : undefined array size %d \n", paramName.asChar(), shaderNode.name().asChar(), arraySize );
							}
							break;
						}
					}
					case SHADER_TYPE_UNKNOWN :
					default:
						liquidMessage ( "Unknown shader type", messageError );
						skipToken = true;
						break;
				}
			}
			if ( !skipToken )
			{
				// set token type
				switch ( shaderDetail )
				{
					case SHADER_DETAIL_UNIFORM:
					{
						tokenPointerArray.rbegin()->setDetailType( rUniform );
						break;
					}
					case SHADER_DETAIL_VARYING:
					{
						tokenPointerArray.rbegin()->setDetailType( rVarying);
						break;
					}
					case SHADER_DETAIL_UNKNOWN:
						tokenPointerArray.rbegin()->setDetailType( rUniform);
						break;
				}
				// create next token
				tokenPointerArray.push_back( liqTokenPointer() );
			}
			else
			{
				// skip parameter : parameter will not be written inside rib
				if ( outputAllParameters )
				{
					char tmp[512];
					sprintf ( tmp, "[liqShader] skipping shader parameter %s on %s (probably an empty dynamic array)\n", paramName.asChar(), shaderNode.name().asChar() );
					liquidMessage( tmp, messageWarning );
				}
			}
		}
	}
	shaderInfo.resetIt();
}


liqShader::~liqShader()
{
}


void liqShader::appendCoShader( MObject coshader, MPlug plug )
{
	// test if it's really a co-shader
	int isLiquidShader = 0;
	MFn::Type objectType = coshader.apiType();
	if ( objectType == MFn::kPluginDependNode )
	{
		MFnDependencyNode fnObject(coshader);
		MTypeId depNodeId = fnObject.typeId();
		//printf("liqSurfaceNodeId=%d  liqDisplacementNodeId=%d  liqVolumeNodeId=%d  liqCoShaderNodeId=%d\n", liqSurfaceNodeId, liqDisplacementNodeId, liqVolumeNodeId, liqCoShaderNodeId);
		if ( depNodeId == liqSurfaceNodeId || 
         depNodeId == liqDisplacementNodeId || 
         depNodeId == liqVolumeNodeId || 
         depNodeId == liqCoShaderNodeId )
		{
			isLiquidShader = 1;
		}
	}
	if ( isLiquidShader )
	{
		MFnDependencyNode fnObject(coshader);
		m_coShaderArray.push_back(coshader);
	}
	else
	{
		printf ( "[liqShader::appendCoShader] Error unsupported connection in plug '%s', abort co-shading for this plug.\n", plug.name().asChar() );
	}
}


void *liqShader::write( bool shortShaderNames, unsigned int indentLevel, SHADER_TYPE forceAs )
{
	vector<MString> yetExportedShaders;
	return write(shortShaderNames, indentLevel, yetExportedShaders, forceAs);
}

void liqShader::writeRibAttributes( MFnDependencyNode &node, SHADER_TYPE shaderType )
{
	MStatus status;
	if ( shaderType == SHADER_TYPE_SHADER )
	{
		return;
	}

	////////////////////////////////////////////
	//			COLOR
	if ( shaderType == SHADER_TYPE_SURFACE || shaderType == SHADER_TYPE_VOLUME )
	{
		RiColor(rmColor);
		RiOpacity(rmOpacity);
	}
	
	////////////////////////////////////////////
	//			RIBBOX
	
	// Try to find a liqRIBBox attribute
	MString shaderRibBox;
	MPlug ribbPlug = node.findPlug( MString( "liqRIBBox" ), &status );
	if ( status == MS::kSuccess )
	{
		ribbPlug.getValue( shaderRibBox );
	}
	// just write the contents of the rib box
	if ( shaderRibBox != "" )
	{
		RiArchiveRecord( RI_VERBATIM, ( char* )shaderRibBox.asChar() );
		RiArchiveRecord( RI_VERBATIM, "\n" );
	}	

	////////////////////////////////////////////
	//			DISPLACE

	// displacement bounds
	if ( shader_type != SHADER_TYPE_LIGHT )
	{  
  	float displacementBounds;
  	MString displacementBoundsSpace;
  	MPlug sDBPlug = node.findPlug( MString( "displacementBound" ), &status );
  	if ( status == MS::kSuccess )
  	{
  		sDBPlug.getValue( displacementBounds );
  	
    	MPlug sDBSPlug = node.findPlug( MString( "displacementBoundSpace" ), &status );
    	if ( status == MS::kSuccess )
    	{
    		sDBSPlug.getValue( displacementBoundsSpace );
    	}
    	if ( displacementBoundsSpace == "" )
    	{
    		displacementBoundsSpace = "shader";
    	}
    	if ( displacementBounds != 0.0 )
    	{
    		RtString coordsys( const_cast< char* >( displacementBoundsSpace.asChar() ) );
    		// RiAttribute( "displacementbound", (RtToken) "sphere", &displacementBounds, "coordinatesystem", &coordsys, RI_NULL );
    		RiAttribute( "displacementbound", "coordinatesystem", &coordsys, RI_NULL );
        RiAttribute( "displacementbound", "sphere", &displacementBounds, RI_NULL );
    	}
    }
  }
}


void *liqShader::write(bool shortShaderNames, unsigned int indentLevel, vector<MString> &yetExportedShaders, SHADER_TYPE forceAs)
{
	void *handle = NULL;
	MStatus status;
	MFnDependencyNode node(m_mObject);
	if ( hasErrors )  // wasn't well initialized, abort
	{
		printf ( "[liqShader::write] Erros occured while initializing shader '%s', won't export shader", node.name().asChar() );
		return NULL;
	}
	// check if shader was yet exported (we don't want to export co-shaders more than one time)
	unsigned int i;
	for ( i=0; i < yetExportedShaders.size(); i++ )
	{
		if ( yetExportedShaders[i] == shaderHandler )
		{
			return NULL; // won't export another time
		}
	}

	// force type : permit to write a co-shader as a Surface/Displace/...
	SHADER_TYPE shaderType = shader_type;
	if ( forceAs != SHADER_TYPE_UNKNOWN )
	{
		shaderType = forceAs;
	}

	// write co-shaders before
	for ( i=0; i<m_coShaderArray.size(); i++ )
	{
		liqGenericShader &genShader = liqShaderFactory::instance().getShader(m_coShaderArray[i]);
		if ( genShader.isShader() )
		{
			liqShader &coShader = *(genShader.asShader());
			if ( coShader.hasErrors )
			{
				char errorMsg[512];
				sprintf ( errorMsg, "[liqShader::write] While initializing coShader for '%s', node couldn't be exported", coShader.name.c_str() );
				liquidMessage( errorMsg, messageError );
			}
			else
			{
				//coShader.writeAsCoShader(shortShaderNames, indentLevel);
				coShader.write ( shortShaderNames, indentLevel, yetExportedShaders, SHADER_TYPE_SHADER );
			}
		}
		else
		{
			char errorMsg[512];
			sprintf ( errorMsg, "[liqShader::write] node '%s', is not a coShader (maybe a switcher)!", genShader.name.c_str() );
			liquidMessage ( errorMsg, messageError );
		}
	}

	// write rib attributes (but not for coshaders)
	writeRibAttributes ( node, shaderType );

	// write shader
	scoped_array< RtToken > tokenArray( new RtToken[ tokenPointerArray.size() ] );
	scoped_array< RtPointer > pointerArray( new RtPointer[ tokenPointerArray.size() ] );
	assignTokenArrays( tokenPointerArray.size(), &tokenPointerArray[ 0 ], tokenArray.get(), pointerArray.get() );
	char* shaderFileName = shortShaderNames ? basename( const_cast<char *>(file.c_str())) : const_cast<char *>(file.c_str());
	if ( shaderSpace != "" )
	{
		RiTransformBegin();
		RiCoordSysTransform( ( RtString )shaderSpace.asChar() );
	}
	// output shader
	// its one less as the tokenPointerArray has a preset size of 1 not 0
	int shaderParamCount = tokenPointerArray.size() - 1;
	switch ( shaderType )
	{
  	case SHADER_TYPE_SURFACE :
  		outputIndentation( indentLevel );
  		if ( useVisiblePoints )
  		#ifdef GENERIC  ||  ( defined( PRMAN ) && defined( RI_VERSION ) &&  RI_VERSION >= 4 )
        RiVPSurfaceV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
      #else
        RiSurfaceV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
      #endif
      else
        RiSurfaceV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
  		break;
  		
  	case SHADER_TYPE_DISPLACEMENT :
  		outputIndentation( indentLevel );
  		RiDisplacementV( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
  		break;
  		
  	case SHADER_TYPE_VOLUME :
  		outputIndentation( indentLevel );
  		switch ( volume_type )
      {
        case VOLUME_TYPE_INTERIOR:
          if ( useVisiblePoints )
          #ifdef GENERIC  ||  ( defined( PRMAN ) && defined( RI_VERSION ) &&  RI_VERSION >= 4 )  
            RiVPInteriorV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() ); 
          #else
            RiInteriorV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
          #endif
          else
            RiInteriorV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() ); 
          break;
        case VOLUME_TYPE_EXTERIOR:
          if ( useVisiblePoints )
          #ifdef GENERIC             
            RiVPExteriorV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
          #else
            // Atleast Prman 16.x haven't this function
            RiExteriorV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );  
          #endif  
          else
            RiExteriorV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() ); 
          break;
        case VOLUME_TYPE_ATMOSPHERE:
        default:
          if ( useVisiblePoints )
          #ifdef GENERIC  ||  ( defined( PRMAN ) && defined( RI_VERSION ) &&  RI_VERSION >= 4 )  
            RiVPAtmosphereV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() ); 
          #else
            RiAtmosphereV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );  
          #endif
          else
            RiAtmosphereV ( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() ); 
          break;
  		}
      break;
  		
  	case SHADER_TYPE_SHADER :
  		outputIndentation( indentLevel );
  		RiShaderV ( shaderFileName, const_cast<char*>(shaderHandler.asChar()), shaderParamCount, tokenArray.get(), pointerArray.get() );
  		break;
  		
  	case SHADER_TYPE_LIGHT :
  		outputIndentation(indentLevel);
  		handle = RiLightSourceV( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
  		/*
   			//!!!! In Generic libRib light handle is unsigned int 
        LIQDEBUGPRINTF( "-> RiLightSourceV shaderFileName = %s\n", shaderFileName );
        LIQDEBUGPRINTF( "-> RiLightSourceV shaderParamCount = %d\n", shaderParamCount );
        LIQDEBUGPRINTF( "-> RiLightSourceV handle = " );
  	    RtLightHandle light_handle = RiLightSourceV( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
  	    if ( light_handle != NULL )
  	    {
          unsigned int handle = (unsigned int)(long)(const void *)light_handle;
  	      LIQDEBUGPRINTF( "%u %u\n", light_handle, handle );
  	      shaderHandler.set( handle );
         
        } 
  		*/
  		break;
  		
  	default :
  		char errorMsg[512];
  		sprintf ( errorMsg, "[liqShader::write] Unknown shader type for %s shader_type=%d", name.c_str(), shader_type );
  		liquidMessage( errorMsg, messageError );
  		break;
	}
	if ( shaderSpace != "" )
	{
		RiTransformEnd();
	}

	yetExportedShaders.push_back ( shaderHandler );
	return handle;
}

#if 0
void liqShader::writeAsCoShader(bool shortShaderNames, unsigned int indentLevel)
{
	MFnDependencyNode node(m_mObject);
	if ( hasErrors )
	{
		printf( "[liqShader::write] Erros occured while initializing shader '%s', won't export shader", node.name().asChar() );
		return;
	}
	// write up co-shaders before
	unsigned int i; 
	for(i=0; i<m_coShaderArray.size(); i++)
	{
		//liqShader coShader(m_coShaderArray[i]);
		liqShader &coShader = liqShaderFactory::instance().getShader(m_coShaderArray[i]);
		coShader.writeAsCoShader(shortShaderNames, indentLevel);
	}
	// write co-shader
	scoped_array< RtToken > tokenArray( new RtToken[ tokenPointerArray.size() ] );
	scoped_array< RtPointer > pointerArray( new RtPointer[ tokenPointerArray.size() ] );
	assignTokenArrays( tokenPointerArray.size(), &tokenPointerArray[ 0 ], tokenArray.get(), pointerArray.get() );
	char* shaderFileName = shortShaderNames ? basename( const_cast<char *>(file.c_str())) : const_cast<char *>(file.c_str());
	if ( shaderSpace != "" )
	{
		RiTransformBegin();
		RiCoordSysTransform( ( RtString )shaderSpace.asChar() );
	}
	// output shader
	// its one less as the tokenPointerArray has a preset size of 1 not 0
	int shaderParamCount = tokenPointerArray.size() - 1;
	char *shaderHandlerPtr = const_cast<char*>(shaderHandler.asChar());
	switch( shader_type )
	{
	case SHADER_TYPE_SHADER :
	case SHADER_TYPE_SURFACE :
	case SHADER_TYPE_DISPLACEMENT :
	case SHADER_TYPE_VOLUME :
		outputIndentation(indentLevel);
		RiShaderV(shaderFileName, shaderHandlerPtr, shaderParamCount, tokenArray.get(), pointerArray.get());
		break;
	default :
		char errorMsg[512];
		sprintf(errorMsg, "[liqShader::writeAsCoShader] Unknown shader type for %s shader_type=%d (%s)", name.c_str(), shader_type, liqGetSloInfo::getTypeStr(shader_type).asChar());
		liquidMessage( errorMsg, messageError );
		break;
	}
	if( shaderSpace != "" )
	{
		RiTransformEnd();
	}
}
#endif

void liqShader::outputIndentation(unsigned int indentLevel)
{
	for ( unsigned int i=0; i < indentLevel; ++i )
	{
		RiArchiveRecord(RI_VERBATIM, "\t");
	}
}

bool liqShader::isShader()
{
	return 1;
}

bool liqShader::isSwitcher()
{
	return 0;
}

liqShader* liqShader::asShader()
{
	return this;
}

liqSwitcher* liqShader::asSwitcher()
{
	return NULL;
}

