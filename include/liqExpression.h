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
#ifndef liqExpression_H
#define liqExpression_H
#include <string.h>

#include <maya/MString.h>
#include <maya/MStringArray.h>


#define IS_EXPRESSION( s ) ( strlen( s ) > 2 && s[0] == '[' && s[ strlen( s ) - 1 ] == ']' )? true : false

enum ExprType {
  exp_None,
  exp_CoordSys,
  exp_MakeTexture,
  exp_CubeEnvMap,
  exp_ReflectMap,
  exp_EnvMap,
  exp_Shadow,
  exp_PointShadow
};

class liqExpression {
  public:
  liqExpression( const string& str, const string& objName = string() );

  MString	CalcValue( void );

  MString	GetValue( void ){ return value; };
  MString	GetCmd( void );

  bool	destExists;
  bool	destIsNewer;;
  bool	isValid;
  ExprType	type;

  private:

  MStringArray tokens;
  MString object_name;
  MString	value;
  MString	options;
  MString source;
  MString dest;
};

#endif
