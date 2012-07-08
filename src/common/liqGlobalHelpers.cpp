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

/* ______________________________________________________________________
**
** Liquid Global Helpers
** ______________________________________________________________________
*/

// Maya Headers
#include <maya/MGlobal.h>
#include <maya/MPlug.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnMesh.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MCommandResult.h>
#include <maya/MFnRenderLayer.h>

// Standard headers
#include <set>
#ifdef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h> //for _mdir()
//#include <list>

//#include <process.h>
#include <io.h>

#endif

// Boost headers
#include <boost/tokenizer.hpp>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <liqGlobalHelpers.h>
#include <liqMayaNodeIds.h>
#include <liqRenderer.h>

extern int debugMode;
extern bool liquidBin;

extern long   liqglo_lframe;
extern MString liqglo_sceneName;
extern MString liqglo_ribDir;
extern MString liqglo_projectDir;
extern MString liqglo_textureDir;
extern MString liqglo_DDimageName;


extern MString liqglo_currentNodeName;
extern MString liqglo_currentNodeShortName;
extern MString liqglo_shotName;
extern MString liqglo_shotVersion;
extern MString liqglo_layer;

extern liquidVerbosityType liqglo_verbosity;

extern liqRenderer liquidRenderer;

using namespace boost;
using namespace std;


/**  Check to see if the node NodeFn has any attributes starting with pPrefix and store those
 *  in Matches to return.
 */
MStringArray findAttributesByPrefix( const char* pPrefix, MFnDependencyNode& NodeFn )
{
  MStringArray Matches;

  for ( unsigned i = 0; i < NodeFn.attributeCount(); i++ ) 
  {
    MFnAttribute AttributeFn = NodeFn.attribute(i);
    MString AttributeName = AttributeFn.name();
    if (!strncmp(AttributeName.asChar(), pPrefix, strlen(pPrefix) )) 
      Matches.append(AttributeName);
  }
  return Matches;
}

/** Checks if the given object is double-sided.
 */
bool isObjectTwoSided ( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN ( path );
  
  bool doubleSided = true;
  liquidGetPlugValue ( fnDN, "doubleSided", doubleSided, status );
  return  doubleSided;
}

/** Checks if the given object is reversed.
 *
 *  That is if the opposite attribute is on.
 */
bool isObjectReversed( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN( path );
  bool reversed = false;
  liquidGetPlugValue( fnDN, "opposite", reversed, status );
  return  reversed;
}

/** Check if the given object is visible.
 */
bool isObjectVisible( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN( path );
  // Check the visibility attribute of the node
  LIQDEBUGPRINTF( "-> checking visibility attribute %s\n", path.fullPathName().asChar() );
  bool visible = true;
  liquidGetPlugValue( fnDN, "visibility", visible, status );
  // Moritz:
  // Check for liquidInvisible attribute. Similar to mtorInvis,
  // this attributes allows objects that have their visibility
  // checked to be skipped for Liquid's output
  bool liquidInvisible = false;
  liquidGetPlugValue( fnDN, "liqInvisible", liquidInvisible, status );
  // Also check to see if the node is an intermediate object in
  // a computation.  For example, it could be in the middle of a
  // chain of deformations.  Intermediate objects are not visible
  LIQDEBUGPRINTF( "-> checking intermediate object\n" );
  bool intermediate = false;
  liquidGetPlugValue( fnDN, "intermediateObject", intermediate, status );
  visible = visible && !liquidInvisible && !intermediate;

  LIQDEBUGPRINTF( "-> checking overrideEnabled %s\n", path.fullPathName().asChar() );
  bool isOver = false;
  liquidGetPlugValue( fnDN, "overrideEnabled", isOver, status );
  if( visible && isOver ) 
    liquidGetPlugValue( fnDN, "overrideVisibility", visible, status );
  
  return visible;
}

/** Checks if the given object is visible to the camera.
 */
bool isObjectPrimaryVisible( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN( path );
  
  LIQDEBUGPRINTF( "-> checking overrideEnabled %s\n", path.fullPathName().asChar() );
  bool isOver = false;
  liquidGetPlugValue( fnDN, "overrideEnabled", isOver, status );
  
  LIQDEBUGPRINTF( "-> checking primary Visibility attribute %s\n", path.fullPathName().asChar() );
  bool primaryVisibility = true;
  liquidGetPlugValue( fnDN, "primaryVisibility", primaryVisibility, status );
  
  if( primaryVisibility && isOver ) 
    liquidGetPlugValue( fnDN, "overrideVisibility", primaryVisibility, status );
  
  return  primaryVisibility;
}

/** Checks if the given object is templated.
 */
bool isObjectTemplated( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN( path );
  
  LIQDEBUGPRINTF( "-> checking templated attribute %s\n", path.fullPathName().asChar() );
  bool templated = false;
  liquidGetPlugValue( fnDN, "template", templated, status );
  bool isOver = false;
  liquidGetPlugValue( fnDN, "overrideEnabled", isOver, status );
  
  if ( !templated && isOver ) 
  {
    int displayType = 0;
    liquidGetPlugValue( fnDN, "overrideDisplayType", displayType, status );
    if ( displayType == 1 )  templated = true;
  }
  return  templated;
}
/** Checks if the given object casts shadows.
 */
bool isObjectCastsShadows( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN( path );
  // Check the visibility attribute of the node
  bool castsShadows( true );
  liquidGetPlugValue( fnDN, "castsShadows", castsShadows, status );
  
  bool isOver( false );
  liquidGetPlugValue( fnDN, "overrideEnabled", isOver, status );

  if( castsShadows && isOver ) 
    liquidGetPlugValue( fnDN, "overrideVisibility", castsShadows, status );
  
  status.clear();
  return  castsShadows;
}
/** Check if the given object receives shadows.
 */
bool isObjectReceivesShadows( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN( path );
  bool receivesShadows = true;
  liquidGetPlugValue( fnDN, "receiveShadows", receivesShadows, status );
  return receivesShadows;
}
/** Check if the given object is motion-blurred.
 */
bool isObjectMotionBlur( const MDagPath & path )
{
  MStatus status;
  MFnDagNode fnDN( path );
  // Check the motionBlur attribute of the node
  MString motionBlurAttr;
  if( path.hasFn(MFn::kPfxHair) ||
      path.hasFn(MFn::kPfxHair)  )
    motionBlurAttr = "motionBlurred";
  else
    motionBlurAttr = "motionBlur";
  bool motionBlur = false;
  liquidGetPlugValue( fnDN, motionBlurAttr.asChar(), motionBlur, status );
  
  return  motionBlur;
}
/** Check if this object and all of its parents are visible.
 *
 *  In Maya, visibility is determined by hierarchy.  So, if one of a node's
 *  parents is invisible, then so is the node.
 */
bool areObjectAndParentsVisible( const MDagPath & path )
{
  bool result = true;
  LIQDEBUGPRINTF( "-> getting searchpath\n" );
  MDagPath searchPath( path );
  MStatus status;

  // Philippe:
  // Check if the path belongs to the current render layers
  MFnRenderLayer renderLayer;

  LIQDEBUGPRINTF( "-> stepping through search path\n" );
  bool searching = true;
  bool isInCurrentRenderLayer = true;
  while ( searching ) 
  {
    LIQDEBUGPRINTF( "-> checking visibility\n" );
    isInCurrentRenderLayer = renderLayer.inCurrentRenderLayer( path, &status );
    if ( !isInCurrentRenderLayer || !isObjectVisible( searchPath ) ) 
    {
      result = false;
      searching = false;
    }
    if ( searchPath.length() == 1 ) searching = false;
    searchPath.pop();
  }
  return result;
}
/** Check if this object and all of its parents are templated.
 *
 *  In Maya, visibility is determined by hierarchy.  So, if one of a node's
 *  parents is templated, then so is the node.
 */
bool areObjectAndParentsTemplated( const MDagPath & path )
{
  bool result = true;
  MDagPath searchPath( path );

  while ( true ) 
  {
    if( isObjectTemplated( searchPath ) ) 
    {
      result = false;
      break;
    }
    if( searchPath.length() == 1 ) break;
    searchPath.pop();
  }
  return result;
}
/** Build the correct token/array pairs from the scene data to correctly pass
 *  to RenderMan.
 */
void assignTokenArrays( unsigned int numTokens, 
												const liqTokenPointer tokenPointerArray[], 
												RtToken tokens[], 
												RtPointer pointers[] )
{
  for ( unsigned i( 0 ); i < numTokens; i++ ) 
  {
    tokens[i] = const_cast< RtString >( const_cast< liqTokenPointer* >( &tokenPointerArray[i] )->getDetailedTokenName().c_str() );
    pointers[i] = const_cast< liqTokenPointer* >( &tokenPointerArray[i] )->getRtPointer();
  }
}

/** Build the correct token/array pairs from the scene data to correctly pass
 *  to RenderMan.
 *
 *  This is another version that takes a vector as input instead of a static array.
 */
void assignTokenArraysV( const vector<liqTokenPointer>& tokenPointerArray, 
												 RtToken tokens[], 
												 RtPointer pointers[] )
{
  unsigned i( 0 );
  for ( vector< liqTokenPointer >::const_iterator iter( tokenPointerArray.begin() ); 
				iter != tokenPointerArray.end(); 
				iter++, i++ ) 
  {
    tokens[ i ] = const_cast< RtString >( const_cast< liqTokenPointer* >( &( *iter ) )->getDetailedTokenName().c_str() );
    pointers[ i ] = const_cast< liqTokenPointer* >( &( *iter ) )->getRtPointer();
  }
}

void assignIthTokenArraysV( const vector<liqTokenPointer>& tokenPointerArray,
														RtToken tokens[], 
													  RtPointer pointers[], 
														unsigned int primi )
{
  unsigned i( 0 );
  for ( vector< liqTokenPointer >::const_iterator iter( tokenPointerArray.begin() ); 
				iter != tokenPointerArray.end(); 
				iter++, i++ ) 
	{
    tokens[ i ] = const_cast< RtString >( const_cast< liqTokenPointer* >( &( *iter ) )->getDetailedTokenName().c_str() );
    pointers[ i ] = const_cast< liqTokenPointer* >( &( *iter ) )->getIthRtPointer( primi );
  }
}

MObject findFacetShader( MObject mesh, int polygonIndex )
{
  MFnMesh     fnMesh( mesh );
  MObjectArray shaders;
  MIntArray indices;
  MDagPath path;

  if ( !fnMesh.getConnectedShaders( 0, shaders, indices ) )  
		liquidMessage( "MFnMesh::getConnectedShaders", messageError );

  MObject shaderNode = shaders[ indices[ polygonIndex ] ];
  return shaderNode;
}

/** Checks if a file exists.
 */
bool fileExists( const MString& filename ) 
{
#ifdef _WIN32
  struct _stat sbuf;
  int result = _stat( filename.asChar(), &sbuf );
    // under Win32, stat fails if path is a directory name ending in a slash
    // so we check for DIR/. Which works - go figure
  if ( result &&( filename.rindex( '/' ) == filename.length() - 1 ) ) 
    result = _stat(( filename + "." ).asChar(), &sbuf );
#else
  struct stat sbuf;
  int result = stat( filename.asChar(), &sbuf );
#endif
  return( result == 0 );
}

/** Checks if file1 is newer than file2
 */
bool fileIsNewer( const MString& file1, const MString& file2 ) 
{
#ifdef _WIN32
  struct _stat sbuf1, sbuf2;
  _stat( file1.asChar(), &sbuf1 );
  _stat( file2.asChar(), &sbuf2 );
#else
  struct stat sbuf1, sbuf2;
  stat( file1.asChar(), &sbuf1 );
  stat( file2.asChar(), &sbuf2 );
#endif
  return sbuf1.st_mtime > sbuf2.st_mtime;
}

bool fileFullyAccessible( const MString& path ) 
{
#ifdef _WIN32
  // Read & Write
  //cerr << "Moin " << string( path.asChar() ) << endl;
  //cerr << "Moin " << _access( path.asChar(), 6 ) << endl;
  return _access( path.asChar(), 6 ) != -1;
#else
  return !access( path.asChar(), R_OK | W_OK | X_OK | F_OK );
#endif
}
/** If transforms relative into absolute paths.
 *
 *  @return Full path
 */
MString getFullPathFromRelative( const MString& filename ) 
{
  MString ret = filename;
  
  if ( filename.substring( 1, 1 ) != ":" ) // windows absolute path C:...
  {
	  if( filename.index( '/' ) != 0  ) // relative path, add prefix project folder
	  {  
		  MString projectDir;
		  if ( liquidBin )
		  {
		    projectDir = liqglo_projectDir; //  + "/";
		  }
		  else
		  {
		    MString MELCommand = "workspace -q -rd";
		    MString MELReturn;
		    MGlobal::executeCommand( MELCommand, MELReturn );
		    projectDir = MELReturn; 
		  }
		  ret = projectDir + filename;
	  }
  }
  return ret;
}

MString getFileName( const MString& fullpath ) 
{
  return fullpath.substring( fullpath.rindex('/') + 1, fullpath.length() - 1 );
}

/** Parse strings sent to Liquid to replace defined
 *  characters with specific variables.
 *
 * ymesh: added triming whitespaces
 */
MString parseString( const MString& inString, bool doEscaped )
{
  using namespace std;
  stringstream ss;
  
	MString tokenString;
	int escapedDollar = 0;
  bool inToken = false;
  string prep_str ( inString.asChar() );
  trim( prep_str );
  MString inputString( prep_str.c_str() );
  int sLength = inputString.length();
  int i;

  for ( i = 0 ; i < sLength ; i++ ) 
  {
    MString str = inputString.substring(i, i);
    MString str_inc = inputString.substring(i+1, i+1);
    MString str_dec = inputString.substring(i-1, i-1);
    
    if( str == "\n" || str == "\t" )  // replace delimiters with space
    { 
      ss << " ";
      continue;
    }
		if ( str == "\\" && str_inc == "$" ) escapedDollar = 1;
    else if ( str == "$" ) 
    {
   		if ( escapedDollar ) ss << "$";
			else
			{
				tokenString.clear();
      	inToken = true;
			}
			escapedDollar = 0;
    } 
    else if ( inToken ) 
    {
      tokenString += str;
      if ( tokenString == "F" ) 
      {
        ss << (int)liqglo_lframe;
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "SCN" ) 
      {
        ss << liqglo_sceneName.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "IMG" ) 
      {
        ss << liqglo_DDimageName.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "PDIR" || tokenString == "PROJDIR" ) 
      {
        ss << liqglo_projectDir.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "RDIR" || tokenString == "RIBDIR" ) 
      {
        ss << liqglo_ribDir.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "TDIR" || tokenString == "TEXDIR" ) 
      {
        ss << liqglo_textureDir.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "OBJ" && inputString.substring(i+1, i+4) != "PATH" ) 
      {
        ss << liqglo_currentNodeShortName.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "OBJPATH" ) 
      {
        ss << liqglo_currentNodeName.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "SHOT" ) 
      {
        ss << liqglo_shotName.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "VER" ) 
      {
        ss << liqglo_shotVersion.asChar();
        inToken = false;
        tokenString.clear();
      } 
      else if ( tokenString == "LYR" || tokenString == "LAYER" ) 
      {
        ss << liqglo_layer.asChar();
        inToken = false;
        tokenString.clear();
      }
      else if ( tokenString == "TEX" ) // texture file extension
      {
        ss << liquidRenderer.textureExtension.asChar();
        inToken = false;
        tokenString.clear();
      }
    } 
    else if ( str == "@" && str_dec != "\\" ) ss << (int)liqglo_lframe;
    else if ( str == "#" && str_dec != "\\" ) 
    {
      int paddingSize = 0;
      while ( inputString.substring( i, i ) == "#" ) 
      {
        paddingSize++;
        i++;
      }
      i--;
      if ( paddingSize == 1 ) paddingSize = 4;
      if ( paddingSize > 20 ) paddingSize = 20;
      
      //char paddedFrame[20];
      //sprintf( paddedFrame, "%0*ld", paddingSize, liqglo_lframe );
      //constructedString += paddedFrame;
      ss << setfill('0') << setw( paddingSize ) <<  liqglo_lframe;
    } 
    else if ( str_inc == "%" ) 
    {
      if ( str == "\\" ) 
      {
        ss << "%";
        i++;
      } 
      else 
      {
        ss << inputString.substring( i, i ).asChar();
        i += 2; // move to 1st character after the '%'

        MString envString;
        char* envVal( NULL );

        // loop through the string looking for the closing %
        if ( i < sLength ) 
        {
          while ( i < sLength && inputString.substring( i, i ) != "%" ) 
          {
            envString += inputString.substring( i, i );
            i++;
          }
          envVal = getenv( envString.asChar() );
          if ( envVal != NULL )  ss << envVal;
          // else environment variable doesn't exist.. do nothing
        }
      }
      // else early exit: % was the last character in the string.. do nothing
    } 
    else if ( str_inc == "#" && str == "\\" ) 
    {
      // do nothing
    } 
    else if ( doEscaped && str_inc == "n" && str == "\\" ) 
    {
      ss << "\n";
      i++;
    } 
    else if ( doEscaped && str_inc == "t" && str == "\\" ) 
    {
      ss << "\t";
      i++;
    } 
    else 
      ss << str.asChar();
  }
  // Moritz: now parse for MEL command sequences
  MString constructedString(  ss.str().c_str() );  
  constructedString = parseCommandString( constructedString );
  return constructedString;
}

// Moritz: added below code for simple MEL parameter expression scripting support
// syntax: `mel commands`
MString parseCommandString( const MString& inputString )
{
  MString constructedString;
  MString tokenString;
  unsigned sLength = inputString.length();
  for ( unsigned i = 0 ; i < sLength ; i++ ) 
  {
    MString str = inputString.substring(i, i);
    MString str_inc = inputString.substring(i+1, i+1);
    MString str_dec = inputString.substring(i-1, i-1);
    
    if ( str == "`" && str_dec != "\\" ) 
    {
      MString  melCmdString;
      i++;
      
      str = inputString.substring(i, i);
      str_inc = inputString.substring(i+1, i+1);
      str_dec = inputString.substring(i-1, i-1);
      
      // loop through the string looking for the closing %
      if ( i < sLength ) 
      {
        while ( i < sLength && inputString.substring(i, i) != "`" && inputString.substring(i - 1, i - 1) != "\\" ) 
        {
          melCmdString += inputString.substring(i, i);
          i++;
        }
        MCommandResult melCmdResult;
#ifdef DEBUG
        // Output command to Script window for debugging
        if ( MS::kSuccess == MGlobal::executeCommand( melCmdString, melCmdResult, true, false ) ) 
        {
#else
        if ( MS::kSuccess == MGlobal::executeCommand( melCmdString, melCmdResult, false, false ) ) 
        {
#endif
          // convert result to string
          MString tmpStr;
          switch ( melCmdResult.resultType() ) 
          {
            case MCommandResult::kInt: 
              {
                int tmpInt;
                melCmdResult.getResult( tmpInt );
                tmpStr = tmpInt;
              }
              break;
            case MCommandResult::kIntArray: 
              {
                MIntArray tmpIntArray;
                melCmdResult.getResult( tmpIntArray );
                for ( unsigned j = 0; j < tmpIntArray.length(); j++ ) 
                {
                  if ( j > 0 ) tmpStr += " ";
                  tmpStr += tmpIntArray[ j ];
                }
              }
              break;
            case MCommandResult::kDouble: 
              {
                double tmpDbl;
                melCmdResult.getResult( tmpDbl );
                tmpStr = tmpDbl;
              }
              break;
            case MCommandResult::kDoubleArray: 
              {
                MDoubleArray tmpDblArray;
                melCmdResult.getResult( tmpDblArray );
                for ( unsigned j = 0; j < tmpDblArray.length(); j++ ) 
                {
                  if ( j > 0 ) tmpStr += " ";
                  tmpStr += tmpDblArray[ j ];
                }
              }
              break;
            case MCommandResult::kString: 
              melCmdResult.getResult( tmpStr );
              break;
            case MCommandResult::kStringArray: 
              {
                MStringArray tmpStrArray;
                melCmdResult.getResult( tmpStrArray );
                for ( unsigned j = 0; j < tmpStrArray.length(); j++ ) 
                {
                  if ( j > 0 ) tmpStr += " ";
                  tmpStr += tmpStrArray[ j ];
                }
              }
              break;
            case MCommandResult::kVector: 
              {
                MVector tmpVec;
                melCmdResult.getResult( tmpVec );
                for ( int j = 0; j < tmpVec.length(); j++ ) 
                {
                  if ( i > 0 ) tmpStr += " ";
                  tmpStr += tmpVec[ i ];
                }
              }
              break;
            case MCommandResult::kVectorArray: 
              {
                MVectorArray tmpVecArray;
                melCmdResult.getResult( tmpVecArray );
                for ( unsigned j = 0; j < tmpVecArray.length(); j++ ) 
                {
                  if ( j > 0 ) tmpStr += " ";
                  for ( unsigned k = 0; tmpVecArray[ j ].length(); k++ ) 
                  {
                    if( k > 0 ) tmpStr += " ";
                    tmpStr += tmpVecArray[ j ] [ k ];
                  }
                }
              }
              break;
            case MCommandResult::kMatrix: 
              {
                MDoubleArray tmpMtx;
                int rows, cols;
                melCmdResult.getResult( tmpMtx, rows, cols );
                for ( int j = 0; j < rows * cols; j++ ) 
                {
                  if ( j > 0 ) tmpStr += " ";
                  tmpStr += tmpMtx[ j ];
                }
              }
              break;
            case MCommandResult::kInvalid:
            default: 
              {
              // do nothing
              // should we return some error string here?
              }
              break;
          }
          constructedString += tmpStr;
        }
        // else MEL command contained an error.. do nothing
      }
      // else early exit: ] was the last character in the string.. do nothing
    } 
    else if ( str_inc == "#" && str == "\\" ) 
    {
      // do nothing
    } 
    else if ( str_inc == "n" && str == "\\" ) 
    {
      constructedString += "\n";
      i++;
    } 
    else if ( str_inc == "t" && str == "\\" ) 
    {
      constructedString += "\t";
      i++;
    } 
    else 
      constructedString += str;
  }
  constructedString = removeEscapes( constructedString );
  return constructedString;
}

MString liquidTransGetSceneName()
{
  MString fullName;
  MGlobal::executeCommand( "file -q -sn -shn", fullName );
  fullName = ( fullName != "" )? fullName : "untitled.mb" ;

  // Move backwards across the string until we hit a dirctory / and
  // take the info from there on
  int i = fullName.rindex( '/' );
  int j = fullName.rindex( '.' );
  // From Maya 6, unsaved files have no extension anymore, we have
  // to account for this here as the ending delimiting '.' is missing
  if ( ( j < i + 2 ) ||( j == -1 ) ) j = fullName.length();
  return fullName.substring( i + 1, j - 1 );
}

MString liquidTransGetFullSceneName()
{
  MString fileName;
  MGlobal::executeCommand (  "file -q -sn", fileName );
  return fileName;
}

MString liquidResolveWinPaths( MString inPath )
{
  MString newName;
  for( unsigned int i = 0; i < inPath.length(); i++ ) 
  {
  }
  return newName;
}

/** Calculates a hashes from a string.
 */
liquidlong liquidHash(const char *str)
{
  LIQDEBUGPRINTF( "-> hashing string\n" );
  liquidlong hc = 0;

  while ( *str ) 
  {
    //hc = hc * 13 + *str * 27;   // old hash function
    hc = hc + *str;   // change this to a better hash func
    str++;
  }
  LIQDEBUGPRINTF( "-> done hashing string\n" );
  return (liquidlong)hc;
}

#ifdef _WIN32
char* basename( const char *filename ) {
//      returns the filename portion of a path
#ifdef MSVC6
  char *p = strrchr( filename, '/' );
#else
  char *p = const_cast< char* >( strrchr( filename, '/' ) );
#endif
  return p ? p + 1 :( char* ) filename;
}
#endif

/** Converts '\' into '/'
 */
MString liquidSanitizePath( const MString& inputString ) {
  const string str( inputString.asChar() );
  string constructedString, buffer;

  for ( unsigned i( 0 ); i < inputString.length(); i++ ) 
  {
    if ( '\\' == str[ i ] ) 
      constructedString += "/";
    else 
      constructedString += str.substr( i, 1 );
  }
  return MString( constructedString.c_str() );
}

/** Convert <driveletter>: into //<driveletter>
 */
MString liquidSanitizeSearchPath( const MString& inputString ) {
  MString constructedString( liquidSanitizePath( inputString ) );
#if defined( DELIGHT ) || defined( PRMAN ) || defined( GENERIC_RIBLIB )
  // Convert from "C:/path" into "//C/path"
  if ( inputString.substring( 1, 1 ) == ":" )
    constructedString = "//" + constructedString.substring( 0, 0 ) + constructedString.substring( 2, inputString.length() - 1 ); // .toLowerCase();
#endif // defined DELIGHT || PRMAN
  return constructedString;
}

string liquidSanitizePath( const string& inputString ) {
  string constructedString, buffer;

  for ( unsigned i( 0 ); i < inputString.length(); i++ ) 
  {
    if ( '\\' == inputString[ i ] ) 
    {
      if ( '\\' == inputString[ i + 1 ] ) 
      {
        ++i; // skip double slashes
        buffer = "\\";
      } 
      else 
        buffer = "/";
    } 
    else 
      buffer = inputString.substr( i, 1 );
    constructedString += buffer;
  }
  return constructedString;
}

string liquidSanitizeSearchPath( const string& inputString ) 
{
  string constructedString( liquidSanitizePath( inputString ) );

#if defined( DELIGHT ) || defined( PRMAN ) || defined( GENERIC_RIBLIB )
  // Convert from "C:/path" into "//C/path"
  if ( inputString[ 1 ] == ':' ) 
  {
    constructedString = "//" + constructedString.substr( 0, 1 ) + constructedString.substr( 2 ); // to_lower_copy( constructedString.substr( 2 ) )
  }
#endif // defined DELIGHT || PRMAN
  return constructedString;
}

/** Get absolute pathnames for creating RIBs,
 *  archives and the renderscript in case the user
 *  has choosen to have all paths to be relative
 */
string liquidGetRelativePath ( bool relative, const string& name, const string& dir ) 
{
  string ret = name;
  bool isNameFullPath = false;
  
  if ( ( '/' == name[ 0 ] ) || ( ':' == name[ 1 ] ) )
    isNameFullPath = true;  
  
  if ( relative )
  {
    if ( isNameFullPath ) 
    {
      if ( dir.length() != 0 && name.find ( dir ) == 0 ) // if name starts with dir  
      {
        ret = name.substr ( dir.length(), name.length() - dir.length() );
      }
    }
  }  
  else
  {  
    if ( !isNameFullPath )
      ret = dir + name; 
  }  
  return ret;

}

MString liquidGetRelativePath ( bool relative, const MString& name, const MString& dir ) 
{
  MString ret = name;
  bool isNameFullPath = false;
  
  if ( ( 0 == name.index('/') ) || ( name.substring( 1, 1 ) == ":" ) )
    isNameFullPath = true;  
  
  if ( relative )
  {
    if ( isNameFullPath ) 
    {
      if ( dir.length() != 0 && name.indexW ( dir ) == 0 ) // if name starts with dir  
      {
        ret = name.substring ( dir.length(), name.length() - 1 );
      }
    }
  }  
  else
  {  
    if ( !isNameFullPath )
      ret = dir + name; 
  }  
  return ret;
}

MString removeEscapes( const MString& inputString ) 
{
  MString constructedString;
  MString tokenString;
  int sLength = inputString.length();
  int i;

  for ( i = 0; i < sLength; i++ ) 
  {
    MString str = inputString.substring( i, i );
    MString str_inc = inputString.substring( i, i+1 );
    
    if ( str_inc == "\\@" ) 
    {
      constructedString += "@";
      i++;
    } 
    else if ( str_inc == "\\#" ) 
    {
      constructedString += "#";
      i++;
    } 
    else if ( str_inc == "\\[" ) 
    {
      constructedString += "[";
      i++;
    } 
    else if ( str_inc == "\\]" ) 
    {
      constructedString += "]";
      i++;
    } 
    else 
      constructedString += str;
  }
  return constructedString;
}

MObject getNodeByName( MString name, MStatus *returnStatus )
{
  MObject node;
  MSelectionList list;

  *returnStatus = MGlobal::getSelectionListByName( name, list );

  if ( MS::kSuccess != *returnStatus )
  {
    MGlobal::displayError( "Cound't get node '" + name + "'. There might be multiple nodes with this name" );
    return node;
  }
  *returnStatus=list.getDependNode( 0, node );
  if ( MS::kSuccess != *returnStatus ) 
  {
    MGlobal::displayError( "Cound't get node '"+ name + "'. There might be multiple nodes with this name" );
    return MObject::kNullObj;
  }
  return node;
}

string getEnvironment( const string& envVar )
{
  string ret;
  char* tmp( getenv( envVar.c_str() ) );
  if ( tmp ) ret = tmp;
  return ret;
}

vector< int > generateFrameNumbers( const string& seq ) 
{
  // We maintain a set to ensure we don't insert a frame into the list twice
  set< int > theSeq;
  vector< int > theFrames;
  typedef tokenizer< char_separator< char > > tokenizer;
  char_separator< char > comma( "," );
  tokenizer frames( seq, comma );

  for ( tokenizer::iterator it( frames.begin() ); it != frames.end(); it++ ) 
  {
    size_t pos( it->find( "-" ) );
    if ( string::npos == pos ) 
    {
      float f(( float )atof( it->c_str() ) );
      if ( theSeq.end() == theSeq.find(( int )f ) ) 
      {
        theSeq.insert(( int )f );
        theFrames.push_back(( int )f );
      }
    } 
    else 
    {
      float startFrame(( float )atof( it->substr( 0, pos ).c_str() ) );
      float endFrame, frameStep;
      size_t pos2( it->find( "@" ) );
      size_t pos3( it->find( "x" ) );
      if ( string::npos == pos2 ) 
        if ( string::npos != pos3 ) 
          pos2 = pos3;
      // Support both RSP- & Shake frame sequence syntax
      if ( string::npos == pos2 ) {
        endFrame =( float )atof( it->substr( pos + 1 ).c_str() );
        frameStep = 1;
      }
      else 
      {
        endFrame =( float )atof( it->substr( pos + 1, pos2 - pos ).c_str() );
        frameStep =( float )fabs( atof( it->substr( pos2 + 1 ).c_str() ) );
      }
      if ( startFrame < endFrame ) 
      {
        for ( float f( startFrame ); f <= endFrame; f += frameStep )
        { 
          if ( theSeq.end() == theSeq.find(( int )f ) ) 
          {
            theSeq.insert(( int )f );
            theFrames.push_back(( int )f );
          }
        }
        if ( theSeq.end() == theSeq.find(( int )endFrame ) )
          theFrames.push_back(( int )endFrame );
      }
      else 
      {
        for ( float f( startFrame ); f >= endFrame; f -= frameStep ) 
        {
          if ( theSeq.end() == theSeq.find(( int )f ) ) 
          {
            theSeq.insert(( int )f );
            theFrames.push_back(( int )f );
          }
        }
        if ( theSeq.end() == theSeq.find(( int )endFrame ) )
          theFrames.push_back(( int )endFrame );
      }
    }
  }
  return theFrames;
}
/**
 *  Create a full path
 */
bool makeFullPath( const string& name, int mode ) 
{
  struct stat stats;
  // Get some space to store out tokenized string
  scoped_array< char > tmp( new char[ name.length() + 1 ] );
  // Need to copy the input string since strtok changes its input
  strncpy( tmp.get(), name.c_str(), name.length() + 1 );
  // Tokenize
  char* token( strtok( tmp.get(), "/" ) );
  // Our path for mkdir()
  string path( token );
  
  if ( '/' == name[ 0 ] ) path = string( "/" ) + path;
#ifdef _WIN32
  if ( '/' == name[ 1 ] ) // probably windows network path here...
  {
    path = string( "/" ) + path;
    // try to find first avaliable share name
    while ( true )
    {
      if ( stat( path.c_str(), &stats ) < 0 )
      {
        token = strtok( NULL, "/" );
        if ( !token ) break;
        path += string( "/" ) + token;
      } 
      else
        break;
    }
  }
#endif

  while ( true ) 
  {
#ifdef _WIN32
    // skip drive letter part of path
    if ( !( ( 2 == path.length() ) &&( ':' == path[ 1 ] ) ) ) 
    {
#endif
      
      if ( stat( path.c_str(), &stats ) < 0 ) 
      {
        // The path element didn't exist
#ifdef _WIN32
        if ( _mkdir( path.c_str() ) )
#else
        if ( mkdir( path.c_str(), mode ) )
#endif
        {
          return false;
        }
      }
#ifdef _WIN32
      //WIN32_FIND_DATA FileData;
      //FindFirstFile( path.c_str(), &FileData );
      //if( !( FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
      if ( !(stats.st_mode & _S_IFDIR) )
#else
      if ( !S_ISDIR( stats.st_mode ) )
#endif
        return false;
#ifdef _WIN32
    }  // if( !( ( 2 == path.length() ) &&( ':' == path[ 1 ] ) ) )
#endif
    // Tokenize
    token = strtok( NULL, "/" );
    if ( !token ) break;
    path += string( "/" ) + token;
  }
  return true;
}

string sanitizeNodeName( const string& name ) 
{
  string newName( name );
//  replace_all( newName, "|", "_" );
//  replace_all( newName, ":", "_" );
// mesh: replaced this due strange exeption in boost function
// on OSX 10.6 Maya2011 x64...
  newName = replace_all_copy( newName, "|", "_" );
  newName = replace_all_copy( newName, ":", "_" );
  return newName;
}

MString sanitizeNodeName( const MString& name ) 
{
  string newName( name.asChar() );
  newName = replace_all_copy( newName, "|", "_" );
  newName = replace_all_copy( newName, ":", "_" );
  return MString( newName.c_str() );
}

RtString& getLiquidRibName( const string& name ) 
{
  static string ribName;
  static RtString tmp;
  ribName = sanitizeNodeName( name );
  tmp = const_cast< RtString >( ribName.c_str() );
  return tmp;
}

/** Standard function to send messages to either the
 *  maya console or the shell for user feedback.
 */
void liquidMessage( MString msg, liquidVerbosityType type ) 
{
  if ( liqglo_verbosity >= type || liquidBin ) 
  {
    if ( !liquidBin ) 
    {
      MString infoOutput( "[Liquid] " + msg );
      switch ( type ) 
      {
        case messageInfo:
          MGlobal::displayInfo( infoOutput );
          break;
        case messageWarning:
          MGlobal::displayWarning( infoOutput );
          break;
        case messageError:
          MGlobal::displayError( infoOutput );
      }
    } 
    else 
    {
      MString infoOutput;
      switch ( type ) 
      {
        case messageWarning:
          infoOutput = "Warning: ";
          break;
        case messageError:
          infoOutput = "Error: ";
          break;
        case messageInfo:
        default:
          infoOutput = "Info: ";  
      }

      cerr << "[Liquid]" << infoOutput << msg << endl << flush;
      //fprintf( STDERR, "[Liquid] %s %s\n", infoOutput.asChar(), msg.asChar() );
      //fflush( STDERR );
    }
  }
}

// get combined string from liquidRibRequest node - Alf
// another piece of easy to read and well documented code
MString parseLiquidRibRequest( MStringArray requestArray, MString attr )
{
	// get all plugin nodes
	MItDependencyNodes itDep( MFn::kPluginDependNode );

	MObjectArray oaLiquidRibRequest;
	MString combinedRequest;
	for ( ; !itDep.isDone(); itDep.next() )
	{
		// filter out liquidRibRequest nodes
		MFnDependencyNode fnDep( itDep.thisNode() );
		if ( fnDep.typeId() == liqRibRequestId )
			oaLiquidRibRequest.append( fnDep.object() );
	}
	
	if ( !oaLiquidRibRequest.length() ) return requestArray[0];

	// get all values and append them to string array
	for ( unsigned i( 0 ); i < oaLiquidRibRequest.length(); i++ )
	{
		MFnDependencyNode fnLiquidRibRequest( oaLiquidRibRequest[i] );
		MPlug pRequest = fnLiquidRibRequest.findPlug( attr );
		MString request;
		pRequest.getValue( request );
//		requestArray.append( request );
		combinedRequest += request;
	}
	
	// skipped this as it does not exist in linux - no duplicates are sorted so what?
	// use list so duplicates can be removed easy - Alf
/*	typedef list< MString > MStringList;
	MStringList preFrameList( requestArray.length() );

	unsigned i( 0 );
	for( MStringList::iterator itPreFrameList( preFrameList.begin() ); itPreFrameList != preFrameList.end(); itPreFrameList++, i++ )
	{
		*itPreFrameList = requestArray[i];
	}
	// remove duplicates
//	preFrameList.sort();
	preFrameList.unique();

	// construct combined string
	MString combinedRequest;
	for( MStringList::iterator itPreFrameList( preFrameList.begin() ); itPreFrameList != preFrameList.end(); itPreFrameList++, i++ )
	{
		combinedRequest += ( *itPreFrameList + "\n" );
	}
//	MGlobal::displayInfo( MString( "combinedRequest: " ) + combinedRequest );
*/	
  return combinedRequest;
}
// liquidGetPlugValue bool
MStatus liquidGetPlugValue( MFnDependencyNode &node, 
                            const char *name, bool &value, MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess ) plug.getValue( value );
  return status;
}
// liquidGetPlugValue int
MStatus liquidGetPlugValue( MFnDependencyNode &node, 
                            const char *name, int &value, MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess ) plug.getValue( value );
  return status;
}
// liquidGetPlugValue RtFloat
MStatus liquidGetPlugValue( MFnDependencyNode &node, 
                            const char *name, RtFloat &value, MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess ) plug.getValue( value );
  return status;
}
// liquidGetPlugValue double
MStatus liquidGetPlugValue( MFnDependencyNode &node, 
                            const char *name, double &value, MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess ) plug.getValue( value );
  return status;
}
// liquidGetPlugValue MString
MStatus liquidGetPlugValue( MFnDependencyNode &node, 
                            const char *name, MString &value, MStatus &status, bool parsed )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess )
  {  
    MString varVal;
    plug.getValue( varVal );
    value = ( parsed )? parseString( varVal ) : varVal;
  }
  return status;
}
// liquidGetPlugValue MVector
MStatus liquidGetPlugValue( MFnDependencyNode &node, 
                            const char *name, MVector &value, MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess ) 
  { 
    plug.child(0).getValue( value[0] );
	  plug.child(1).getValue( value[1] );
	  plug.child(2).getValue( value[2] );
  }
   return status;
}
// liquidGetPlugNumElements
unsigned int liquidGetPlugNumElements( MFnDependencyNode &node, const char *name, MStatus *status )
{
  unsigned int num = 0;
  status->clear();
  MPlug plug = node.findPlug( name, &status );
  if ( *status == MS::kSuccess )
  {
    status->clear();
    num = plug.numElements( status );
  }
  return num;
}
// liquidGetPlugElementValue MString
MStatus liquidGetPlugElementValue( MFnDependencyNode &node, 
                                   unsigned int ind, 
                                   const char *name, 
                                   MString &value, 
                                   MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess )
  {  
    status.clear();
    MPlug elementPlug = plug.elementByLogicalIndex( ind, &status );
    if ( status == MS::kSuccess ) elementPlug.getValue( value );
  }
  return status;
}
// liquidGetPlugElementValue int
MStatus liquidGetPlugElementValue( MFnDependencyNode &node, 
                                   unsigned int ind, 
                                   const char *name, 
                                   int &value, 
                                   MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess )
  {  
    status.clear();
    MPlug elementPlug = plug.elementByLogicalIndex( ind, &status );
    if ( status == MS::kSuccess ) elementPlug.getValue( value );
  }
  return status;
}
// liquidGetPlugElementValue bool
MStatus liquidGetPlugElementValue( MFnDependencyNode &node, 
                                   unsigned int ind, 
                                   const char *name, 
                                   bool &value, 
                                   MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess )
  {  
    status.clear();
    MPlug elementPlug = plug.elementByLogicalIndex( ind, &status );
    if ( status == MS::kSuccess ) elementPlug.getValue( value );
  }
  return status;
}
// liquidGetPlugElementValue float
MStatus liquidGetPlugElementValue( MFnDependencyNode &node, 
                                   unsigned int ind, 
                                   const char *name, 
                                   float &value, 
                                   MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess )
  {  
    status.clear();
    MPlug elementPlug = plug.elementByLogicalIndex( ind, &status );
    if ( status == MS::kSuccess ) elementPlug.getValue( value );
  }
  return status;
}
// liquidGetPlugElementValue MStringArray
MStatus liquidGetPlugElementValue( MFnDependencyNode &node, 
                                   unsigned int ind, 
                                   const char *name, 
                                   MStringArray &array, 
                                   MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess )
  {  
    status.clear();
    MPlug elementPlug = plug.elementByLogicalIndex( ind, &status );
    if ( status == MS::kSuccess )
    {
      MObject arrayDataObj;
      elementPlug.getValue( arrayDataObj );
      MFnStringArrayData arrayData( arrayDataObj, &status );
      arrayData.copyTo( array );
    }
  }
  return status;
}
// liquidGetPlugElementValue MIntArray
MStatus liquidGetPlugElementValue( MFnDependencyNode &node, 
                                   unsigned int ind, 
                                   const char *name, 
                                   MIntArray &array, 
                                   MStatus &status )
{
  status.clear();
  MPlug plug = node.findPlug( name, &status );
  if ( status == MS::kSuccess )
  {  
    status.clear();
    MPlug elementPlug = plug.elementByLogicalIndex( ind, &status );
    if ( status == MS::kSuccess )
    {
      MObject arrayDataObj;
      elementPlug.getValue( arrayDataObj );
      MFnIntArrayData arrayData( arrayDataObj, &status );
      arrayData.copyTo( array );
    }
  }
  return status;
}