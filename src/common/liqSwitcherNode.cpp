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

#include <liquid.h>
#include <liqSwitcherNode.h>
#include <liqMayaNodeIds.h>

#include <maya/MGlobal.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnGenericAttribute.h>
#include <maya/MFnCompoundAttribute.h>

//#include <liqIOStream.h>

// id
MTypeId liqSwitcherNode::id( liqSwitcherNodeId );
// input attributes
MObject liqSwitcherNode::mAcceptedConnectionNodeType;
MObject liqSwitcherNode::mDefaultShader;
MObject liqSwitcherNode::mSwitchShaders;
MObject liqSwitcherNode::mUserAttribute;
MObject liqSwitcherNode::mCustomUserAttribute;
MObject liqSwitcherNode::mCondition;
MObject liqSwitcherNode::mUserValue;
MObject liqSwitcherNode::mShader;



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



void liqSwitcherNode::postConstructor( )
{
	setMPSafe(true);
	MGlobal::executeCommandOnIdle( "liquidCheckGlobals()", false );
}


liqSwitcherNode::liqSwitcherNode()
{
}


liqSwitcherNode::~liqSwitcherNode()
{
}


void* liqSwitcherNode::creator()
{
	return new liqSwitcherNode();
}


MStatus liqSwitcherNode::initialize()
{
	MStatus status;
	MFnTypedAttribute tAttr;
	MFnEnumAttribute eAttr;
	MFnGenericAttribute gAttr;
	MFnCompoundAttribute mAttr;

	// Create input attributes
	mDefaultShader = gAttr.create("defaultShader", "ds", &status );
	gAttr.setKeyable(false);
	gAttr.setStorable(true);
	gAttr.setReadable(true);
	gAttr.setWritable(true);
	gAttr.addNumericDataAccept(MFnNumericData::k3Float);

	mUserAttribute = eAttr.create("userAttribute", "ua", 0, &status );
	eAttr.setKeyable(false);
	eAttr.setStorable(true);
	eAttr.setReadable(true);
	eAttr.setWritable(true);
	eAttr.addField("PassType", 0);
	eAttr.addField("PassName", 1);
	eAttr.addField("SubPassType", 2);
	eAttr.addField("SubPassName", 3);
	eAttr.addField("custom", 4);

	mCustomUserAttribute = tAttr.create("customUserAttribute", "cua", MFnData::kString, MObject::kNullObj, &status);
	tAttr.setKeyable(false);
	tAttr.setStorable(true);
	tAttr.setReadable(true);
	tAttr.setWritable(true);

	mCondition = eAttr.create("condition", "cd", 0, &status );
	eAttr.setKeyable(false);
	eAttr.setStorable(true);
	eAttr.setReadable(true);
	eAttr.setWritable(true);
	eAttr.addField("is", 0);
	eAttr.addField("contains", 1);
	eAttr.addField("is not", 2);

	mUserValue = tAttr.create("userValue", "uv", MFnData::kString, MObject::kNullObj, &status);
	tAttr.setKeyable(false);
	tAttr.setStorable(true);
	tAttr.setReadable(true);
	tAttr.setWritable(true);

	mShader = gAttr.create("shader", "sur", &status );
	gAttr.setKeyable(false);
	gAttr.setStorable(true);
	gAttr.setReadable(true);
	gAttr.setWritable(true);
	gAttr.addNumericDataAccept(MFnNumericData::k3Float);

	mSwitchShaders = mAttr.create("switchShader", "ss", &status);
	mAttr.setKeyable(false);
	mAttr.setStorable(true);
	mAttr.setReadable(true);
	mAttr.setWritable(true);
	mAttr.setArray(true);
	mAttr.addChild(mUserAttribute);
	mAttr.addChild(mCustomUserAttribute);
	mAttr.addChild(mCondition);
	mAttr.addChild(mUserValue);
	mAttr.addChild(mShader);


	//////////////////////////////////////
	//
	//    Add attributes
	//

	CHECK_MSTATUS( addAttribute( mDefaultShader ) );
	CHECK_MSTATUS( addAttribute( mSwitchShaders ) );
	CHECK_MSTATUS( addAttribute( mUserAttribute ) );
	CHECK_MSTATUS( addAttribute( mCustomUserAttribute ) );
	CHECK_MSTATUS( addAttribute( mCondition ) );
	CHECK_MSTATUS( addAttribute( mUserValue ) );
	CHECK_MSTATUS( addAttribute( mShader ) );
	
	return MS::kSuccess;
}


MStatus liqSwitcherNode::compute(const MPlug&, MDataBlock&)
{
	return MS::kSuccess;
}



