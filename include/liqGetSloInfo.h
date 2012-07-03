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

#ifndef liqGetSloInfo_H
#define liqGetSloInfo_H

/* ______________________________________________________________________
**
** Liquid Get .slo Info Header File
** ______________________________________________________________________
*/

#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPxCommand.h>

#include <vector>
#include <map>



typedef enum {
    SHADER_TYPE_UNKNOWN,
    SHADER_TYPE_POINT,
    SHADER_TYPE_COLOR,
    SHADER_TYPE_SCALAR,
    SHADER_TYPE_STRING,
/* The following types are primarily used for shaders */
    SHADER_TYPE_SURFACE,
    SHADER_TYPE_LIGHT,
    SHADER_TYPE_DISPLACEMENT,
    SHADER_TYPE_VOLUME,           // 8
    SHADER_TYPE_TRANSFORMATION,
    SHADER_TYPE_IMAGER,
/* The following are variable types added since RISpec 3.1 */
    SHADER_TYPE_VECTOR,
    SHADER_TYPE_NORMAL,
    SHADER_TYPE_MATRIX,
/* The following is for co-shading */
    SHADER_TYPE_SHADER
} SHADER_TYPE;


typedef enum {
    SHADER_DETAIL_UNKNOWN,
    SHADER_DETAIL_VARYING,
    SHADER_DETAIL_UNIFORM
} SHADER_DETAIL;

typedef enum {
    VOLUME_TYPE_ATMOSPHERE,
    VOLUME_TYPE_INTERIOR,
    VOLUME_TYPE_EXTERIOR
} VOLUME_TYPE;


class liqGetSloInfo : public MPxCommand {
public:
  liqGetSloInfo();
  virtual       ~liqGetSloInfo();
  static void*  creator();
  int           setShader( MString shaderName );
  int           setShaderNode( MFnDependencyNode &shaderNode );
  void          resetIt();
  int           nargs();
  MString       getName();
  SHADER_TYPE   getType();
  int           getNumParam();
  MString       getTypeStr();
  static MString getTypeStr(SHADER_TYPE shaderType);
  MString       getArgName( int num );
  SHADER_TYPE   getArgType( int num );
  MString       getArgTypeStr( int num );
  SHADER_DETAIL getArgDetail( int num );
  MString       getArgDetailStr( int num );
  MString       getArgStringDefault( int num, int entry );
  float         getArgFloatDefault( int num, int entry );
  int           getArgArraySize( int num );
  int           isOutputParameter( unsigned int num );
  MString       getArgAccept( unsigned int num );

  // TODO :
  //int           getNumMethods( );
  //int           getMethodName( int num );

  struct mstrcomp
  {
    bool operator()(const MString s1, const MString s2) const
    {
      return strcmp( (char*)s1.asChar(), (char*)s2.asChar()) < 0;
    }
  };


	MStatus	    doIt(const MArgList& args );
private:
  unsigned numParam;
  SHADER_TYPE shaderType;
  MString shaderName;
  std::vector<MString> argName;
  std::vector<SHADER_TYPE> argType;
  std::vector<SHADER_DETAIL> argDetail;
  std::vector<int> argArraySize;
  std::vector<void*> argDefault;
  std::map<const MString, SHADER_TYPE, mstrcomp> shaderTypeMap;
  std::map<const MString, SHADER_DETAIL, mstrcomp> shaderDetailMap;
  std::vector<int> argIsOutput;
	std::vector<MString> argAccept;
};


#endif
