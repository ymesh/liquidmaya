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
** Contributor(s): Philippe Leprince.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

/* ______________________________________________________________________
**
** Liquid Surface Shader Node Source
** ______________________________________________________________________
*/

#include <liqSurfaceSwitcherNode.h>
#include <liqMayaNodeIds.h>

#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFloatVector.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnLightDataAttribute.h>


MTypeId liqSurfaceSwitcherNode::id( liqSurfaceSwitcherNodeId );

// maya shader attributes
MObject liqSurfaceSwitcherNode::aColor;
MObject liqSurfaceSwitcherNode::aTransparency;
MObject liqSurfaceSwitcherNode::aOpacity;
MObject liqSurfaceSwitcherNode::aMayaIgnoreLights;
MObject liqSurfaceSwitcherNode::aMayaKa;
MObject liqSurfaceSwitcherNode::aMayaKd;
MObject liqSurfaceSwitcherNode::aNormalCameraX;
MObject liqSurfaceSwitcherNode::aNormalCameraY;
MObject liqSurfaceSwitcherNode::aNormalCameraZ;
MObject liqSurfaceSwitcherNode::aNormalCamera;
MObject liqSurfaceSwitcherNode::aLightDirectionX;
MObject liqSurfaceSwitcherNode::aLightDirectionY;
MObject liqSurfaceSwitcherNode::aLightDirectionZ;
MObject liqSurfaceSwitcherNode::aLightDirection;
MObject liqSurfaceSwitcherNode::aLightIntensityR;
MObject liqSurfaceSwitcherNode::aLightIntensityG;
MObject liqSurfaceSwitcherNode::aLightIntensityB;
MObject liqSurfaceSwitcherNode::aLightIntensity;
MObject liqSurfaceSwitcherNode::aLightAmbient;
MObject liqSurfaceSwitcherNode::aLightDiffuse;
MObject liqSurfaceSwitcherNode::aLightSpecular;
MObject liqSurfaceSwitcherNode::aLightShadowFraction;
MObject liqSurfaceSwitcherNode::aPreShadowIntensity;
MObject liqSurfaceSwitcherNode::aLightBlindData;
MObject liqSurfaceSwitcherNode::aLightData;
// output attributes
MObject liqSurfaceSwitcherNode::aOutColor;
MObject liqSurfaceSwitcherNode::aOutTransparency;

#define MAKE_INPUT(attr)		\
	CHECK_MSTATUS(attr.setKeyable(true ) ); 		\
	CHECK_MSTATUS(attr.setStorable(true ) );		\
	CHECK_MSTATUS(attr.setReadable(true ) ); 		\
	CHECK_MSTATUS(attr.setWritable(true ) );

#define MAKE_NONKEYABLE_INPUT(attr)		\
	CHECK_MSTATUS(attr.setKeyable(false ) ); 		\
	CHECK_MSTATUS(attr.setStorable(true ) );		\
	CHECK_MSTATUS(attr.setReadable(true ) ); 		\
	CHECK_MSTATUS(attr.setWritable(true ) );

#define MAKE_OUTPUT(attr)		\
	CHECK_MSTATUS(attr.setKeyable(false ) ); 		\
	CHECK_MSTATUS(attr.setStorable(false ) );		\
	CHECK_MSTATUS(attr.setReadable(true ) ); 		\
	CHECK_MSTATUS(attr.setWritable(false ) );

liqSurfaceSwitcherNode::liqSurfaceSwitcherNode()
{
}


liqSurfaceSwitcherNode::~liqSurfaceSwitcherNode()
{
}


void* liqSurfaceSwitcherNode::creator()
{
	return new liqSurfaceSwitcherNode();
}


MStatus liqSurfaceSwitcherNode::initialize()
{
	MStatus status;
	status = liqSwitcherNode::initialize();
	if(status != MS::kSuccess)
	{
		return status;
	}
	MFnNumericAttribute nAttr;
	MFnLightDataAttribute lAttr;

	mAcceptedConnectionNodeType = nAttr.create("acceptedType", "act", MFnNumericData::kInt);
	nAttr.setKeyable(false);
	nAttr.setStorable(false);
	nAttr.setReadable(true);
	nAttr.setWritable(false);
	nAttr.setDefault(liqSurfaceNodeId);
	CHECK_MSTATUS( addAttribute( mAcceptedConnectionNodeType ) );


	// create attributes for maya renderer
	aColor = nAttr.createColor("color", "cs");
	nAttr.setDefault( 1.0, 1.0, 1.0 );
	MAKE_INPUT(nAttr);

	aOpacity = nAttr.createColor("opacity", "os");
	nAttr.setDefault( 1.0, 1.0, 1.0 );
	MAKE_INPUT(nAttr);

	aTransparency = nAttr.createColor("transparency", "ts"); // Needed by Maya for Open Gl preview in "5" mode, invert opacity in compute
	nAttr.setDefault( 0.0, 0.0, 0.0 );
	MAKE_INPUT(nAttr);

	// lambertian control
	aMayaIgnoreLights = nAttr.create("mayaIgnoreLights", "mil",  MFnNumericData::kBoolean, 0.0, &status);
	MAKE_INPUT(nAttr);
	aMayaKa = nAttr.create("mayaKa", "mka", MFnNumericData::kFloat, 0.2, &status);
	MAKE_INPUT(nAttr);
	aMayaKd = nAttr.create("mayaKd", "mkd", MFnNumericData::kFloat, 0.8, &status);
	MAKE_INPUT(nAttr);

	// Camera Normals
	aNormalCameraX = nAttr.create( "normalCameraX", "nx", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aNormalCameraY = nAttr.create( "normalCameraY", "ny", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aNormalCameraZ = nAttr.create( "normalCameraZ", "nz", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aNormalCamera = nAttr.create( "normalCamera","n", aNormalCameraX, aNormalCameraY, aNormalCameraZ, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f, 1.0f, 1.0f ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );

	// Light Direction
	aLightDirectionX = nAttr.create( "lightDirectionX", "ldx", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aLightDirectionY = nAttr.create( "lightDirectionY", "ldy", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aLightDirectionZ = nAttr.create( "lightDirectionZ", "ldz", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aLightDirection = nAttr.create( "lightDirection", "ld", aLightDirectionX, aLightDirectionY, aLightDirectionZ, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f, 1.0f, 1.0f ) );

	// Light Intensity
	aLightIntensityR = nAttr.create( "lightIntensityR", "lir", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aLightIntensityG = nAttr.create( "lightIntensityG", "lig", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aLightIntensityB = nAttr.create( "lightIntensityB", "lib", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aLightIntensity = nAttr.create( "lightIntensity", "li", aLightIntensityR, aLightIntensityG, aLightIntensityB, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f, 1.0f, 1.0f ) );

	// Light
	aLightAmbient = nAttr.create( "lightAmbient", "la", MFnNumericData::kBoolean, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( true ) );
	aLightDiffuse = nAttr.create( "lightDiffuse", "ldf", MFnNumericData::kBoolean, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( true ) );
	aLightSpecular = nAttr.create( "lightSpecular", "ls", MFnNumericData::kBoolean, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( false ) );
	aLightShadowFraction = nAttr.create( "lightShadowFraction", "lsf", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setReadable( true ) );
	CHECK_MSTATUS( nAttr.setWritable( true ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );
	aPreShadowIntensity = nAttr.create( "preShadowIntensity", "psi", MFnNumericData::kFloat, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 1.0f ) );

#if MAYA_API_VERSION >= 800
	aLightBlindData = nAttr.createAddr( "lightBlindData", "lbld", 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( (void*) 0 ) );
#else
	aLightBlindData = nAttr.create( "lightBlindData", "lbld", MFnNumericData::kLong, 0, &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( nAttr.setStorable( false ) );
	CHECK_MSTATUS( nAttr.setHidden( true ) );
	CHECK_MSTATUS( nAttr.setReadable( false ) );
	CHECK_MSTATUS( nAttr.setDefault( 0 ) );
#endif
	aLightData = lAttr.create( "lightDataArray", "ltd", aLightDirection,
				  aLightIntensity, aLightAmbient, aLightDiffuse, aLightSpecular,
				  aLightShadowFraction, aPreShadowIntensity, aLightBlindData,
				  &status );
	CHECK_MSTATUS( status );
	CHECK_MSTATUS( lAttr.setArray( true ) );
	CHECK_MSTATUS( lAttr.setStorable( false ) );
	CHECK_MSTATUS( lAttr.setHidden( true ) );
	CHECK_MSTATUS( lAttr.setDefault( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, true, true, false, 1.0f, 1.0f, 0 ) );

	// Create output attributes
	aOutColor = nAttr.createColor("outColor", "oc");
	MAKE_OUTPUT(nAttr);

	aOutTransparency = nAttr.createColor("outTransparency", "ot");
	MAKE_OUTPUT(nAttr);

	CHECK_MSTATUS( addAttribute( aColor ) );
	CHECK_MSTATUS( addAttribute( aTransparency ) );
	CHECK_MSTATUS( addAttribute( aOpacity ) );
	CHECK_MSTATUS( addAttribute( aMayaIgnoreLights ) );
	CHECK_MSTATUS( addAttribute( aMayaKa ) );
	CHECK_MSTATUS( addAttribute( aMayaKd ) );
	CHECK_MSTATUS( addAttribute( aNormalCamera ) );
	CHECK_MSTATUS( addAttribute( aLightData ) );
	CHECK_MSTATUS( addAttribute( aOutColor ) );
	CHECK_MSTATUS( addAttribute( aOutTransparency ) );

	CHECK_MSTATUS(attributeAffects( aColor,               aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aOpacity,             aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aMayaIgnoreLights,    aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aMayaKa,              aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aMayaKd,              aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightIntensityR,     aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightIntensityB,     aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightIntensityG,     aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightIntensity,      aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aNormalCameraX,       aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aNormalCameraY,       aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aNormalCameraZ,       aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aNormalCamera,        aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightDirectionX,     aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightDirectionY,     aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightDirectionZ,     aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightDirection,      aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightAmbient,        aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightSpecular,       aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightDiffuse,        aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightShadowFraction, aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aPreShadowIntensity,  aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightBlindData,      aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aLightData,           aOutColor ) );
	CHECK_MSTATUS(attributeAffects( aOpacity,             aOutTransparency ) );

	return MS::kSuccess;
}



MStatus liqSurfaceSwitcherNode::compute( const MPlug& plug, MDataBlock& block )
{
	// outColor or individual R, G, B channel
	if(	(plug == aOutColor) ||
		(plug.parent() == aOutColor) ||
		(plug == aOutTransparency) ||
		(plug.parent() == aOutTransparency)
		)
	{ 
		// init shader
		MStatus status;
		MFloatVector& cColor  = block.inputValue(aColor).asFloatVector();
		MFloatVector& cTrans  = block.inputValue(aOpacity).asFloatVector();
		MFloatVector resultColor( 0.0, 0.0, 0.0 );
		MFloatVector resultTrans( cTrans );
		// lambert calc -------------------
		bool&  ignoreLights = block.inputValue( aMayaIgnoreLights, &status ).asBool();
		float& Ka = block.inputValue( aMayaKa, &status ).asFloat();
		float& Kd = block.inputValue( aMayaKd, &status ).asFloat();
		// get surface normal
		MFloatVector& surfaceNormal = block.inputValue( aNormalCamera, &status ).asFloatVector();
		CHECK_MSTATUS( status );

		if( ignoreLights )
		{
			MFloatVector cam( 0.0, 0.0, 1.0 );
			float cosln = cam * surfaceNormal;
			if ( cosln > 0.0f )
			{
				float diff = cosln * cosln * Kd + Ka;
				resultColor = diff * cColor;
			}
		}
		else
		{
			// Get light list
			MArrayDataHandle lightData = block.inputArrayValue( aLightData, &status );
			CHECK_MSTATUS( status );
			int numLights = lightData.elementCount( &status );
			CHECK_MSTATUS( status );

			// Iterate through light list and get ambient/diffuse values
			for( int count=1; count <= numLights; count++ )
			{
				// Get the current light out of the array
				MDataHandle currentLight = lightData.inputValue( &status );
				CHECK_MSTATUS( status );
				// Get the intensity of that light
				MFloatVector& lightIntensity = currentLight.child( aLightIntensity ).asFloatVector();
				// Find ambient component
				if ( currentLight.child( aLightAmbient ).asBool() )
				{
					resultColor += lightIntensity;
				}
				// Find diffuse component
				if ( currentLight.child( aLightDiffuse ).asBool() )
				{
					MFloatVector& lightDirection = currentLight.child( aLightDirection ).asFloatVector();
					float cosln = lightDirection * surfaceNormal;
					if ( cosln > 0.0f )
					{
						resultColor += lightIntensity * cosln * Kd ;
					}
				}
				// Advance to the next light.
				if ( count < numLights )
				{
					status = lightData.next();
					CHECK_MSTATUS( status );
				}
			}
			resultColor[0] *= cColor[0];
			resultColor[1] *= cColor[1];
			resultColor[2] *= cColor[2];
		}
		resultTrans[0] = ( 1.0 - resultTrans[0] );
		resultTrans[1] = ( 1.0 - resultTrans[1] );
		resultTrans[2] = ( 1.0 - resultTrans[2] );


		// set ouput color attribute
		MDataHandle outColorHandle = block.outputValue( aOutColor );
		MFloatVector& outColor = outColorHandle.asFloatVector();

		outColor = resultColor;
		outColorHandle.setClean();

		MDataHandle outTransHandle = block.outputValue( aOutTransparency );
		MFloatVector& outTrans = outTransHandle.asFloatVector();
		outTrans = resultTrans;
		outTransHandle.setClean();
	}
	else
	{
		return MS::kUnknownParameter;
	}
	return MS::kSuccess;
}

