#!/usr/bin/python
# #############################################################################
#
# xml2afanasy.py
#
# Based on Moritz Moeller dojob.py script.
#
#
# #############################################################################


import sys
import os
import string

#import getopt 
import re

from xml.dom import minidom

import af
  
def main(): 
  if ( len( sys.argv ) > 1 ):
    jobfile = sys.argv[1]
    
    
    workDir = os.getcwd()
    # assume that sys.argv[2] is working directory
    if ( len( sys.argv ) > 2 ):
      workDir = sys.argv[2]
    
    print( "input script: %s (%d arguments)" % (jobfile, len( sys.argv ) ) )
    print( "work dir: %s" % workDir )  
    
    try:
      scriptJob = minidom.parse( jobfile )
      jobTitle = scriptJob.getElementsByTagName( "title" )[ 0 ].firstChild.data
      
      
      minservers = scriptJob.getElementsByTagName( "minservers" )[ 0 ].firstChild.data
      maxservers = scriptJob.getElementsByTagName( "maxservers" )[ 0 ].firstChild.data
      
      subtasks = scriptJob.getElementsByTagName( "subtasks" )[ 0 ]
      
      #dirmaps = scriptJob.getElementsByTagName( "dirmaps" )[ 0 ].firstChild.data
            
      print( "Job: " + jobTitle )
      print( "minservers = " + minservers )
      print( "maxservers = " + maxservers )
      
      job = af.Job( jobTitle )
      
      if subtasks.hasChildNodes():
        for element in subtasks.childNodes:
          if element.nodeName == "task":
            # a task can have subtasks
            taskName = element.getElementsByTagName( "title" )[ 0 ].firstChild.data
            block = af.Block( taskName, 'prman')
            block.setWorkingDirectory( workDir )
            
            for subelement in element.childNodes:
              if subelement.nodeName == "commands":  
                for command in subelement.childNodes:
                  if command.nodeName == "command":  
                    # strip string of leading and trailing whitespace
                   
                    cmd_str = string.strip( command.firstChild.data )
                    cmd_lst = string.split( cmd_str, " ", -1 )
                    
                    cmd = cmd_lst[0]
                    for ss in cmd_lst[ 1: ]:
                      if ss == "-Progress":
                        cmd += " -progress"  
                      else:
                        # if matched dirmaps
                        m = re.match( "%D", ss )
                        if m :
                          ss = ss[3:-1]
                          # print( ">> matched dirmaps " + ss ) 
                        cmd += " " + ss
                        
                      #  m = p.match( ss )
                      #  print( ">> " + m )  
                      
                    task = af.Task( taskName )
                    task.setCommand( cmd )
                    block.tasks.append( task )
                    
              if subelement.nodeName == "chaser": 
                
                img_str = string.strip( subelement.firstChild.data )
                print( ">>chaser img_str: " + img_str ) 
                img_lst = string.split( img_str, " ", -1 )
                img = img_lst[1].strip('\"')
                print( ">>chaser img: " + img ) 
                # use only image name without "sho"
                block.setFiles( img )
                
            job.blocks.append( block )        
            
      print
      job.output( True )
      print

      job.send()
      
    except:
      print "Error parsing '" + jobfile + "'."
    
  sys.exit( 0 )
  
  
if __name__ == "__main__":  
  main()
