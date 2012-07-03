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
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

#ifndef liqProcessLauncher_H_
#define liqProcessLauncher_H_


class MString;


// class for spawning new processes from within Liquid (e.g. start the render 
// process after making RIB files). The code is platform-dependant so this 
// class encapsulates all the details
class liqProcessLauncher
{
public:
  static bool execute(const MString &command, const MString &arguments, const MString &path, const bool wait );
};


#endif // liqProcessLauncher_H_
