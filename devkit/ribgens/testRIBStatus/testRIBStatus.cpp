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
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <process.h>
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#include <alloca.h>
#include <pwd.h>
#endif



#include <math.h>
#include <sys/stat.h>
#include <string>
#include <iostream>


#include <maya/MObject.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

using namespace std;

#include <liqRibGen.h>
#include <liqRibStatus.h>
extern "C" {
	#include <ri.h>
}

class testRIBStatus: public liqRibGen 
{
  public:
    testRIBStatus();
    virtual ~testRIBStatus();
    
    virtual int SetArgs( liqRibStatus *, int n, RtToken tokens[], RtPointer values[] ){ return 0; };
    virtual void Bound( liqRibStatus *, RtBound b ) {};
    virtual int GenRIB( liqRibStatus * );
};

extern "C" liqRibGen*  RIBGenCreate();
extern "C" void  RIBGenDestroy( liqRibGen *g );

/*---------------------------------------------------------*/
liqRibGen  *RIBGenCreate() 
{
    return new testRIBStatus();
}

void RIBGenDestroy(liqRibGen * g) 
{
    delete dynamic_cast<testRIBStatus *>(g);
}

/*---------------------------------------------------------*/
testRIBStatus::testRIBStatus() 
{
}

testRIBStatus::~testRIBStatus() 
{
}

void 
initRibGen( liqRibStatus * ribStatus )
{

}

int testRIBStatus::GenRIB( liqRibStatus * c ) 
{
	
  
  MCommandResult *r;
  std :: string errorString;
  std :: string cmd;

  //char** errorString;
  
  /* initialize the rib generator! */
	
  // initRibGen( ribStatus );
	
	MFnDagNode dagNode( c->dagPath );
  
  c->ReportError(liqRibStatus :: reInfo, "Output from = %s ver.%d \n", "testRIBStatus",32 );
  c->ReportError(liqRibStatus :: reInfo, "dagNode.fullPathName() = %s\n", dagNode.fullPathName().asChar() );
  c->ReportError(liqRibStatus :: reInfo, "objectName = %s\n", c->objectName.c_str() );

  // cmd = "ls -dag";
  cmd = "getAttr \"" + c->objectName + ".worldMatrix\"";

  r = c -> ExecuteHostCmd( cmd.c_str(), errorString );
  if ( r != NULL )
  {
    MString resType;
    MCommandResult::Type t = r->resultType();
    switch( t )
    {
      case MCommandResult::kInt:
        resType = MString( "kInt" ); break;
      case MCommandResult::kIntArray:
        resType = MString( "kIntArray" ); break;
      case MCommandResult::kDouble:
        resType = MString( "kDouble" ); break;
      case MCommandResult::kDoubleArray:
        resType = MString( "kDoubleArray" ); break;
      case MCommandResult::kString:
        resType = MString( "kString" ); break;
      case MCommandResult::kStringArray:
        resType = MString( "kStringArray" ); break;
      case MCommandResult::kVector:
        resType = MString( "kVector" ); break;
      case MCommandResult::kVectorArray:
        resType = MString( "kVectorArray" ); break;
      case MCommandResult::kMatrix:
        resType = MString( "kMatrix" ); break;
      case MCommandResult::kInvalid:
      default:
        resType = MString( "kInvalid" ); break;

    }
    c->ReportError(liqRibStatus :: reInfo, "ResultType()  = %s\n", resType.asChar() );
    if ( t == MCommandResult::kStringArray ) 
    {
      MStatus stat;
      MStringArray result; 
      stat = r->getResult( result );
      if ( stat == MS::kSuccess )
      {
        for( unsigned i(0) ; i < result.length() ; i++ )
          printf( " %s\n", result[i].asChar() );
      }
    }
  }
  
  c->Comment( "============= testRIBStatus output ================\n" );
  c->AttributeBegin();
    c->Comment( "Liquid RibGen Test!\n" );
  c->AttributeEnd();

  /*
  RiArchiveRecord( RI_COMMENT, "Liquid Rib Generation Test!" );
	RiAttributeBegin();
	RtColor col = { 1.0, 1.0, 1.0};
	RiColor( col );
	RiAttributeEnd();
	MString comment("Dag Path: ");
	comment += ribStatus->dagPath.fullPathName();
	comment += "   Frame : ";
	comment += ribStatus->sampleTimes[0];
	RiArchiveRecord( RI_COMMENT, (char *)comment.asChar());
	RtColor myColor;
	myColor[0] = 0.1; 
	myColor[1] = 0.2; 
	myColor[2] = 0.3; 
	RiColor( myColor );
	*/
  return 1;
}
