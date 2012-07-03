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

#ifndef liqRenderScript_H
#define liqRenderScript_H

#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <liqIOStream.h>

#if defined(_WIN32) && !defined(DEFINED_LIQUIDVERSION)
extern const char * LIQUIDVERSION ;
// unix build gets this from the Makefile
#define DEFINED_LIQUIDVERSION
#endif

using namespace std;

/* ______________________________________________________________________
**
** Liquid Job Script Header File
** ______________________________________________________________________
*/


// Class to encapsulate the processes to run after liquid has completed
// generating RIB files
// 
// Initially this supports Pixar's Alfred and a basic XML format. Other formats
// can be added onto this class, or it can be extracted into a class hierarchy
// where new output formats can be added on through subclasses and an API mechanism


class liqRenderScript
{
public:
  // a Cmd object represents a single command line to be run as part of a job
  // needs to be a class rather than just a string, to carry along extra info about running the
  // command to be run (e.g. local or remote, alfred tags, etc)
  class Cmd
  {
  public:
    Cmd() : command(""), remote( true ), alfredExpand( false ), alfredTags( "" ), alfredServices( "" ) {}
    Cmd( const string &c, bool r ) : command(c), remote(r), alfredExpand(false), alfredTags(""), alfredServices("") {}

    string command;

    bool remote;
    bool alfredExpand;
    string alfredTags;
    string alfredServices;
    
    string getALF() const
    {
      stringstream ss;
      
      if ( remote ) ss << "RemoteCmd";
      else          ss << "Cmd";
      ss << " {" << command << "}";
      if ( alfredExpand ) ss << " -expand 1";
      if ( alfredServices != "" ) ss << " -service {" << alfredServices << "}";
      if ( alfredTags != "" ) ss << " -tags {" << alfredTags << "}";

      return ss.str();
    }
    
    string getXML() const
    {
      stringstream ss;
      
      ss << "<command remote=\"" << remote << "\"";
      if ( alfredServices != "" ) ss << " alfredservices=\"" << alfredServices << "\"";
      if ( alfredTags != "" ) ss << " alfredtags=\"" << alfredTags << "\"";
      ss << ">" << command << "</command>";
      
      return ss.str();
    }
  };

  // a Job object encapsulates a single logical task to be run, with additional
  // cleanup and 'chaser' commands (optional GUI driven tasks, e.g. display an image)
  // jobs can be nested to form hierarchical dependencies
  class Job
  {
  public:
    Job() : isInstance(false) {}
    
    string title;
    vector<Cmd> commands;
    vector<Cmd> cleanupCommands;
    string chaserCommand;

    vector<Job> childJobs;
    
    bool isInstance;
    
    string getALF( unsigned int indentLevel=0 ) const
    {
      stringstream ss;
      
      // setup the correct indentation amount to prepend onto each line
      string indent;
      for ( unsigned i(0); i < indentLevel ; ++i ) indent += "  ";
      
      // if we're an instance task, then the title is the name of the task to instance
      // and nothing more gets done in this job
      if ( isInstance ) 
      {
        ss << indent << "Instance {" << title << "}";
        return ss.str();
      }
      ss << indent << "Task -title {" << title << "}";
       
      if ( !childJobs.empty() ) 
      { 
        ss << " -subtasks {" << endl;
        for ( vector<Job>::const_iterator child=childJobs.begin(); child!=childJobs.end(); ++child ) 
          ss << child->getALF(indentLevel+1) << endl;
        ss << indent << "}";
      }
      
      if ( !commands.empty() ) 
      {
        ss << " -cmds {" << endl;
        for ( vector<Cmd>::const_iterator command=commands.begin(); command!=commands.end(); ++command ) 
          ss << indent << "  " << command->getALF() << endl;
        ss << indent << "}";
      }
      
      if ( !cleanupCommands.empty() ) 
      {
        ss << " -cleanup {" << endl;
        for ( vector<Cmd>::const_iterator cleanup=cleanupCommands.begin(); cleanup!=cleanupCommands.end(); ++cleanup ) 
          ss << indent << "  " << cleanup->getALF() << std::endl;
        ss << indent << "}";
      }
      
      if ( chaserCommand != "" ) 
      {
        ss << " -chaser {" << endl;
        ss << indent << "  " << chaserCommand << endl;
        ss << indent << "}";
      }
      return ss.str();
    }
    
    string getXML( unsigned int indentLevel = 0 ) const
    {
      stringstream ss;
      string indent;

      for ( unsigned i=0 ; i < indentLevel ; ++i ) indent += "  ";
       
      if (isInstance) 
      {
        ss << indent << "<instance>" << title << "</instance>" << endl;
        return ss.str();
      }
      
      ss << indent << "<task>" << endl;
      ss << indent << "<title>" << title << "</title>" <<endl;
      if ( !childJobs.empty() ) 
      {
        ss << indent << "<subtasks>" << endl;
        for ( std::vector<Job>::const_iterator child=childJobs.begin(); child!=childJobs.end(); ++child ) 
          ss << child->getXML(indentLevel+1);
        ss << indent << "</subtasks>" << endl;
      }
            
      if ( !commands.empty() ) 
      {
        ss << indent << "<commands>" << endl;
        for ( std::vector<Cmd>::const_iterator command=commands.begin(); command!=commands.end(); ++command ) 
          ss << indent << "  " << command->getXML() << endl;
        
        ss << indent << "</commands>" << endl;
      }
            
      if ( !cleanupCommands.empty() ) 
      {
        ss << indent << "<cleanup>" << endl;
        for ( std::vector<Cmd>::const_iterator cleanup=cleanupCommands.begin(); cleanup!=cleanupCommands.end(); ++cleanup ) 
          ss << indent << "  " << cleanup->getXML() << endl;
        
        ss << indent << "</cleanup>" << endl;
      }       
      
      if ( chaserCommand != "" ) 
        ss << indent << "<chaser>" << chaserCommand << "</chaser>" << endl;
      ss << indent << "</task>" << endl;
            
      return ss.str();
    }
    
    // adds a job as a dependency of any 'leaf' jobs anywhere under the hierarchy
    // of this job. i.e. any job with no children will get this as a child
    // good for tacking instances onto the end of job trees
    void addLeafDependency( const Job &job )
    {
      bool hasChildren = false;
      for ( std::vector<Job>::iterator child=childJobs.begin(); child!=childJobs.end(); ++child ) 
      {
        if ( !child->isInstance ) 
        {
          hasChildren = true;
          child->addLeafDependency( job );
        }
      }
      
      if ( !hasChildren ) childJobs.push_back( job );
    }
  };

public:
  liqRenderScript()
  {
    clear();
  }

  // add a job to the list
  // if a job is a child you need to specify the job ID of the parent
  // the first (main) job will always have an ID of '1'
  // for the main job, or any other job not dependent on anything else
  // you should specify a parent ID of '0' (no parent)
  int addJob( const Job &job, unsigned int parentJobID = 0 )
  {
    int jobId = popNextJobID();
    assert(jobId > 0);
    
    if ( parentJobID == 0 ) 
    {
      jobs[jobId] = job;
    } 
    else 
    {
      if (jobs.find(parentJobID) == jobs.end()) 
      {
        throw MString( "Error liqRenderScript::addJob() called with non-existant parent job ID" );
      }
      jobs[parentJobID].childJobs.push_back( job );
    }
    
    return jobId;
  }

  void clear()
  {
    title = "Liquid Job";
    jobs.clear();
    cleanupCommands.clear();
    nextJobID     = 1;
    minServers    = 1;
    maxServers    = 1;
  }
  
  string getALF() const
  {
    stringstream ss;
    
    ss << "##AlfredToDo 3.0" << endl;
    ss << "Job -title {" << title << "(liquid job)}";
    ss << " -comment {#Created By Liquid " << LIQUIDVERSION << "}";
    
    if ( dirmaps.size() ) ss << " -dirmaps {" << dirmaps << "}";  
    ss << " -atleast " << minServers << " " << "-atmost " << maxServers;
     
    if ( !jobs.empty() ) 
    {
      ss << " -subtasks { ";
      for ( std::map<int, Job>::const_iterator job=jobs.begin(); job!=jobs.end(); ++job ) 
        ss << endl << job->second.getALF(1);    
      ss << endl << "}";
    }
    
    if ( !cleanupCommands.empty() ) 
    {
      ss << " -cleanup { " << endl;
      for ( std::vector<Cmd>::const_iterator cmd=cleanupCommands.begin(); cmd!=cleanupCommands.end(); ++cmd ) 
        ss << "  " << cmd->getALF() << endl;
      ss << "}" << endl;
    }
    
    return ss.str();
  }
  
  string getXML() const
  {
    stringstream ss;
    
    ss << "<?xml version=\"1.0\"?>" << endl;
    ss << "<renderscript>" << endl;
    ss << "  <title>" << title << "</title>" << endl;
    ss << "  <minservers>" << minServers << "</minservers>" << endl;
    ss << "  <maxservers>" << maxServers << "</maxservers>" << endl;
    
    if ( !jobs.empty() ) 
    {
      ss << "  <subtasks>" << endl;
      for ( std::map<int, Job>::const_iterator job=jobs.begin(); job!=jobs.end(); ++job ) 
        ss << job->second.getXML(1);    
      ss << "  </subtasks>" << endl;
    }
    
    if ( !cleanupCommands.empty() ) 
    {
      ss << "  <cleanup>" << endl;
      for ( std::vector<Cmd>::const_iterator cmd=cleanupCommands.begin(); cmd!=cleanupCommands.end(); ++cmd ) 
        ss << "    " << cmd->getXML() << endl;
      
      ss << "  </cleanup>" << endl;
    }
    ss << "</renderscript>" << endl;
    
    return ss.str();
  }
  
  bool writeALF( const string &filename ) const
  {
    ofstream outFile( filename.c_str() );
    
    if ( !outFile ) return false;
    
    outFile << getALF().c_str();
    outFile.close();
    return true;
  }
  
  bool writeXML( const string &filename ) const
  {
    ofstream outFile( filename.c_str() );
	  LIQDEBUGPRINTF( "Writing XML Render Script to: %s!\n", filename.c_str() );	  
    
    if ( !outFile ) return false;
    
    outFile << getXML().c_str();
    outFile.close();
	  LIQDEBUGPRINTF( "Finished writing XML Render Script.\n" );
    return true;
  }
  
  void addLeafDependency( const Job &job )
  {
    for ( map<int, Job>::iterator j=jobs.begin(); j!=jobs.end(); ++j ) 
      if ( (!j->second.isInstance) && (j->second.title != "liquid pre-job") ) 
        j->second.addLeafDependency(job);
  }

public:
  string title;
  vector<Cmd> cleanupCommands;
  
  unsigned int minServers;
  unsigned int maxServers;
  string dirmaps;
  
private:
  int popNextJobID()
  {
    return nextJobID++;
  }
    
private:
  map<int, Job> jobs;
  int nextJobID;
};


#endif // liqRenderScript_H
