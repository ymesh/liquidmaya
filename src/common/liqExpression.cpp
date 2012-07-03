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

#include <liqIOStream.h>
#include <liqGlobalHelpers.h>
#include <liqExpression.h>

extern int debugMode;

extern long liqglo_lframe;
extern MString liqglo_sceneName;
extern MString liqglo_textureDir;
extern MString liqglo_projectDir;
extern bool liqglo_isShadowPass;
extern bool liqglo_expandShaderArrays;
extern bool liqglo_doShadows;
extern bool liqglo_shortShaderNames;
extern MString liqglo_DDimageName;

liqExpression::liqExpression( const string& str1, const string& objName ) :
// liqExpression::liqExpression( char * str, char *objName ) :
  destExists( false ),
  destIsNewer( false ),
  isValid( true ),
  type( exp_None ),
  object_name("")
{
  const char* str( str1.c_str() );
  if ( IS_EXPRESSION( str ) ) 
  {
    MStatus	stat;
    MString strValue( str );
    MString strExpr = strValue.substring( 1, strValue.length() - 2 );
    stat = strExpr.split( ' ', tokens );
    
    if ( objName.length() )
      object_name.set( objName.c_str() );
    //if ( objName != NULL)
    //  object_name.set( objName );

    if ( tokens[0] == "CoordSys" ) 
    {
      type = exp_CoordSys;
      if ( tokens.length() != 2 )
        isValid = false;
    } 
    else if ( tokens[0] == "MakeTexture" ) 
    {
      type = exp_MakeTexture;
      if ( tokens.length() < 2 )
        isValid = false;
    } else if ( tokens[0] == "CubeEnvMap" ) 
    {
      type = exp_CubeEnvMap;
      if ( tokens.length() < 7 )
        isValid = false;
    } else if ( tokens[0] == "ReflectMap" ) 
    {
      type = exp_ReflectMap;
      if ( tokens.length() != 2 )
        isValid = false;
    } else if ( tokens[0] == "EnvMap" ) 
    {
      type = exp_EnvMap;
    } else if ( tokens[0] == "Shadow" || tokens[0] == "PointShadow" ) 
    {
      type = (tokens[0] == "Shadow")? exp_Shadow : exp_PointShadow ;
    }
  }
}

MString	liqExpression::CalcValue( )
{
  int i;

  switch( type ) 
  {
    case exp_CoordSys:
      value = tokens[1];
      break;

    case exp_MakeTexture:

      source = getFullPathFromRelative( tokens[1] );
      value = getFileName( source );
      value += ".tex";

      for ( i = 2; i < tokens.length() ; i++ ) 
      {
        options += tokens[i];
        options += " ";
      }

      dest = liqglo_textureDir + value;
      destExists = fileExists( dest );
      destIsNewer = fileIsNewer( dest, source );
      break;

    case exp_ReflectMap:
      object_name = tokens[1];
      if ( object_name.length() ) 
      {
        if ( ( liqglo_DDimageName == "" ) ) 
        {
          value += liqglo_sceneName;
        } 
        else 
        {
          int pointIndex = liqglo_DDimageName.index( '.' );
          value += liqglo_DDimageName.substring(0, pointIndex-1).asChar();
        }
        value += "_";
        value += object_name;
        value += "REF";
        value += ".";
        value += (int)liqglo_lframe;
        value += ".tex";
      }
      break;

    case exp_Shadow:
    case exp_PointShadow:
      if ( object_name.length() ) 
      {
        if ( ( liqglo_DDimageName == "" ) ) 
        {
          value += liqglo_sceneName;
        } 
        else 
        {
          int pointIndex = liqglo_DDimageName.index( '.' );
          value += liqglo_DDimageName.substring(0, pointIndex-1).asChar();
        }
        value += "_";
        value += object_name;
        value += "SHD";
        if ( type == exp_PointShadow ) 
        {
          value += "_";
          value += tokens[1]; // assume that it must be PX|PY|PZ|NX|NY|NZ
        }
        value += ".";
        value += (int)liqglo_lframe;
        value += ".tex";
      }
      break;

    case exp_EnvMap:
      cout <<"Liquid -> EnvMap is not implemented yet"<<endl;
      break;
    case exp_CubeEnvMap:
      cout <<"Liquid -> CubeEnvMap is not implemented yet"<<endl;
      break;
    case exp_None:
    default:
      cout <<"Liquid -> non valid expression token"<<endl;
      break;
  }
  return value;
}

MString	liqExpression::GetCmd( void )
{
  MString cmd;

  switch ( type ) 
  {
    case exp_MakeTexture:
      cmd = options + " " + source + " " + dest;
      break;

    case exp_CoordSys:
    case exp_ReflectMap:
    case exp_Shadow:
    case exp_PointShadow:
    case exp_EnvMap:
    case exp_CubeEnvMap:
    case exp_None:
    default:
      break;
  }
  return cmd;
}
