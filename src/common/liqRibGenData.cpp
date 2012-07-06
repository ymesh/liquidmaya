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

/* ______________________________________________________________________
**
** Liquid Rib Gen Data Source
** ______________________________________________________________________
*/

#ifndef _WIN32
#  include <dlfcn.h> // dlopen() etc
#endif
#include <stdio.h>
#include <stdarg.h>


// RenderMan headers
extern "C" {
#include <ri.h>
}

// Maya headers
#include <maya/MDagPath.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MSelectionList.h>
#include <maya/MPlugArray.h>

// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibGenData.h>
#include <liqRibGen.h>

extern int debugMode;

extern FILE *liqglo_ribFP;
extern long liqglo_lframe;
extern structJob liqglo_currentJob;
extern bool liqglo_doMotion;
extern bool liqglo_doDef;
extern bool liqglo_doCompression;
extern bool liqglo_doBinary;
extern RtFloat liqglo_sampleTimes[LIQMAXMOTIONSAMPLES];
extern liquidlong liqglo_motionSamples;
extern float liqglo_shutterTime;


/** Create a RIB Gen.
 */
liqRibGenData::liqRibGenData( MObject obj, MDagPath path )
{
  LIQDEBUGPRINTF( "-> creating ribgen\n" );
  MFnDependencyNode fnNode( obj );
  MPlug ribGenNodePlug = fnNode.findPlug( "liquidRibGen" );
  MObject ribGenObj;
  /* check the node to make sure it's not using the old ribGen
   * assignment method, this is for backwards compatibility.  If it's a
   * kTypedAttribute that it's more than likely going to be a string! */
  if ( ribGenNodePlug.attribute().apiType() == MFn::kTypedAttribute ) 
  {
    MString ribGenNode;
    ribGenNodePlug.getValue( ribGenNode );
    MSelectionList ribGenList;
    ribGenList.add( ribGenNode );
    ribGenList.getDependNode( 0, ribGenObj );
  } 
  else 
  {
    if ( ribGenNodePlug.isConnected() ) 
    {
      MPlugArray ribGenNodeArray;
      ribGenNodePlug.connectedTo( ribGenNodeArray, true, true );
      ribGenObj = ribGenNodeArray[0].node();
    }
  }
  MFnDependencyNode fnRibGenNode( ribGenObj );
  MPlug ribGenPlug = fnRibGenNode.findPlug( "RibGenSo" );
  MString plugVal;
  ribGenPlug.getValue( plugVal );
  ribGenSoName = parseString( plugVal, false );
  ribStatus.objectName = fnNode.name().asChar();
  ribStatus.dagPath = path;
  LIQDEBUGPRINTF( "-> ribgen name %s\n", fnNode.name().asChar() );
}

void liqRibGenData::write()
{
  LIQDEBUGPRINTF( "-> writing ribgen\n" ); 
#if defined( PRMAN ) || defined( DELIGHT ) || defined( GENERIC_RIBLIB )
#ifndef _WIN32
  void *handle;
#else
  HINSTANCE handle;
#endif
  char* dlStatus = NULL;
  // Hmmmmmm do not really understand what's going on here?
  ribStatus.ribFP = liqglo_ribFP;
  ribStatus.frame = liqglo_lframe;
  if ( liqglo_currentJob.pass == rpShadowMap ) 
    ribStatus.renderPass = liqRibStatus::rpShadow;
  else 
    ribStatus.renderPass = liqRibStatus::rpFinal;
  
  ribStatus.transBlur = liqglo_doMotion;
  ribStatus.defBlur = liqglo_doDef;
  ribStatus.compressed = liqglo_doCompression;
  ribStatus.binary = liqglo_doBinary;
  liqglo_currentJob.camera[0].mat.get( ribStatus.cameraMatrix );
  ribStatus.sampleTimes = liqglo_sampleTimes;
  if ( liqglo_doMotion || liqglo_doDef ) 
  {
    if ( liqglo_currentJob.pass != rpShadowMap || liqglo_currentJob.shadowType == stDeep ) 
      ribStatus.motionSamples = liqglo_motionSamples;
    else 
      ribStatus.motionSamples = 1;
  } 
  else 
    ribStatus.motionSamples = 1;
  ribStatus.shutterAngle = liqglo_shutterTime;
  /*
  * rib stream connection call from the Affine toolkit.
  * until an equivalent is found in PRMan/3Delight leave commented out.
  * dan-b 7/2/03
  */
  //ribStatus.RiConnection = RiDetach();

  typedef liqRibGen *(*createRibGen)();
  typedef void (*destroyRibGen)( liqRibGen * );

  createRibGen    createRibGenFunc;
  destroyRibGen    destroyRibGenFunc;

#ifndef _WIN32
  handle = dlopen( ribGenSoName.asChar(), RTLD_LAZY );
  dlStatus = dlerror();
  if ( dlStatus == NULL ) {
    createRibGenFunc = (createRibGen)dlsym( handle, "RIBGenCreate" );
    destroyRibGenFunc = (destroyRibGen)dlsym( handle, "RIBGenDestroy" );
    dlStatus = dlerror();
    if ( dlStatus == NULL ) {
#else
  handle = LoadLibrary( ribGenSoName.asChar() );
  LIQDEBUGPRINTF( "-> ribGenSoName %s\n", ribGenSoName.asChar() );
  if ( handle != NULL ) 
  {
    createRibGenFunc = (createRibGen)GetProcAddress( handle, "RIBGenCreate" );
    destroyRibGenFunc = (destroyRibGen)GetProcAddress( handle, "RIBGenDestroy" );
    if ( ( createRibGenFunc != NULL ) && ( destroyRibGenFunc != NULL ) ) 
    {
#endif
      LIQDEBUGPRINTF( "-> call createRibGenFunc\n" );
      liqRibGen *ribGen = (*createRibGenFunc)();
      LIQDEBUGPRINTF( "-> call _GenRIB\n" );
      // SetArgs( &ribStatus, int n, RtToken tokens[], RtPointer values[] );
      // Bound( liqRibStatus *, RtBound b );
      ribGen->GenRIB( &ribStatus );
      LIQDEBUGPRINTF( "-> destroy ribgen\n" );
      (*destroyRibGenFunc)( ribGen );
    } 
    else 
    {
      // string errorString( "Error reading RIBGenCreate or RIBGenDestroy in RIBGen: " );
      // errorString += ribGenSoName.asChar();
      // liquidMessage( errorString, messageError );
      printf( "-> [Error] reading RIBGenCreate or RIBGenDestroy in RIBGen: %s\n", ribGenSoName.asChar() );
    }
#ifndef _WIN32
    dlclose( handle );
#else
    FreeLibrary( handle );
#endif
  } 
  else 
  {
#if defined _WIN32    
    TCHAR szBuf[80]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
    printf( "-> [Error] opening RIBGen %s (%d) %s \n", ribGenSoName.asChar(), dw, lpMsgBuf );
#else
    string errorString( "Opening RIBGen '" );
    errorString += ribGenSoName.asChar();
    errorString += "'(";
    errorString += dlStatus;
    errorString += ") on object '";
    errorString += ribStatus.objectName + "'";
    liquidMessage( errorString, messageError );
#endif
  }

  /*
  * rib stream connection call from the Affine toolkit.
  * until an equivilent is found in prman leave commented out.
  * dan-b 7/2/03
  */
  // RiFreeConnection( ( char * )ribStatus.RiConnection );

#else
    liquidMessage( "Can not handle RIBGen", messageError );
#endif
}

bool liqRibGenData::compare( const liqRibData & otherObj ) const
//
//  Description:
//      Compare this ribgen to the other for the purpose of determining
//      if its animated
//
{
  LIQDEBUGPRINTF( "-> comparing RIBGen\n" );
  return ( otherObj.type() != MRT_RibGen )? false : true;
}

ObjectType liqRibGenData::type() const
//
//  Description:
//      return the geometry type
//
{
  LIQDEBUGPRINTF( "-> returning RibGen type\n" );
  return MRT_RibGen;
}
//
//
//
liqRibGenStatus::liqRibGenStatus()
{
  CmdResult = NULL;
}

liqRibGenStatus::~liqRibGenStatus()
{
  if ( CmdResult != NULL )
    delete CmdResult;
}

void  liqRibGenStatus::ReportError( RenderingError e, const char *fmt, ... )
{
  va_list argptr;
  va_start(argptr, fmt );
  
  string prefix_fmt;

  switch ( e )
  {
    case reInfo:    prefix_fmt = "INFO: ";    break;
    case reWarning: prefix_fmt = "WARNING: "; break;
    case reError:   prefix_fmt = "ERROR: ";   break;
    case reSevere:  prefix_fmt = "SEVERE: ";  break;
  }
  prefix_fmt += string( fmt ); 
  vprintf( prefix_fmt.c_str(), argptr );
}

MCommandResult * liqRibGenStatus::ExecuteHostCmd( const char *cmd, std::string &errstr )
{ 
  MStatus stat;
  MCommandResult* ret = NULL;
  
  if ( CmdResult != NULL ) delete CmdResult;
  CmdResult = new( MCommandResult );

  stat = MGlobal::executeCommand (	MString( cmd ), *CmdResult, true, false );	 
	if ( stat == MS::kSuccess )
  {
    // if ( CmdResult != NULL ) printf("executeCommand= MS::kSuccess (%d)\n", CmdResult->resultType() );
    ret = CmdResult;
  }
  return ret;
}


RtVoid liqRibGenStatus::Comment( RtToken name ) { RiArchiveRecord( RI_COMMENT, name ); }
RtVoid liqRibGenStatus::AttributeBegin() { RiAttributeBegin(); }
RtVoid liqRibGenStatus::AttributeEnd() { RiAttributeEnd(); }