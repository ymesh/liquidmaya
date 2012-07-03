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

#ifndef liquidRibGenData_H
#define liquidRibGenData_H

/* ______________________________________________________________________
**
** Liquid Rib Generator Data Header File
** ______________________________________________________________________
*/

#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>

#include <liqRibData.h>
#include <liqRibStatus.h>


class liqRibGenStatus: public liqRibStatus {
  private:
    MCommandResult *CmdResult;
  public:
    liqRibGenStatus();
    virtual ~liqRibGenStatus();

    virtual void  ReportError( RenderingError e, const char *fmt, ... );
    virtual MCommandResult * ExecuteHostCmd( const char *cmd, std::string &errstr );
    //virtual MCommandResult * ExecuteHostCmd( const char *cmd, char** errstr );
    virtual RtVoid Comment( RtToken name );
    virtual RtVoid AttributeBegin();
    virtual RtVoid AttributeEnd();
  
};

class liqRibGenData : public liqRibData {
public: // Methods
    liqRibGenData( MObject obj, MDagPath path );

    virtual void       write();
    virtual bool       compare( const liqRibData & other ) const;
    virtual ObjectType type() const;

private: // Data
    MString 	      ribGenSoName;
    liqRibGenStatus	ribStatus;
};



#endif
