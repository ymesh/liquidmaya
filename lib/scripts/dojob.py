#!/usr/bin/python
# #############################################################################
#
# d o j o b . p y
#
# Simple script to recursively parse a Liquid XML jobfile and execute all
# commands found in it.
# Supports SUN's Grid Engine for job distribution including holding/postponing
# of dependent tasks/subtasks.
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with the
# License. You may obtain a copy of the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
# the specific language governing rights and limitations under the License.
#
# The Original Code is the dojob.py script.
#
# The Initial Developer of the Original Code is Moritz Moeller. Portions
# created by Moritz Moeller are Copyright (C) 2004. All Rights Reserved.
#
# Todo: make this a 'true' OO program, aka: create a 'jobChef' class that cooks
#       commands. Maintain global parameters in another class that can be
#       questioned about their values to avoid the use of global variables.
#
# #############################################################################


import getopt, sys, os, string, re, random
from xml.dom import minidom

def usage():
	print """Version: doJob 0.9.0

Usage:   dojob.py [options] jobfile.xml

Options:
  --start | -s <frame>      frame to start at executing tasks
  --end | -e <frame>        frame to stop at executing tasks

  --sge                     use Grid Engine

  -h | --help               print this usage information
"""


class indentHelper:
	def __init__( self ):
		self.tabSize = 2
		self.execDepth = 0

	def incrIndent( self ):
		self.execDepth += self.tabSize

	def decrIndent( self ):
		self.execDepth -= self.tabSize

	def getIndent( self ):
		str = ""
		for i in range( 0, self.execDepth ):
			str += " "
		return str


globalIndent = indentHelper()
globalSGE = 0


def doSubTasks( job, startFrame, endFrame ):

	global execDepth;

	jobList = []
	holdJobList = []

	if job.hasChildNodes():
		for element in job.childNodes:
			if element.nodeName == "task":
				# a task can have subtasks
				taskName = element.getElementsByTagName( "title" )[ 0 ].firstChild.data

				executeTask = 1
				if ( startFrame != -1 ) or ( endFrame != -1 ):
					result = re.compile( "Frame[0-9]+$" ).search( taskName )
					if result:
						if startFrame != -1:
							executeTask &= int( taskName[ result.start() + 5: ] ) >= startFrame
						if endFrame != -1:
							executeTask &= int( taskName[ result.start() + 5: ] ) <= endFrame

				if 1 == executeTask:
					print globalIndent.getIndent() + "Executing Task '" + taskName + "':"

					for subTask in element.childNodes:
						if subTask.nodeName == "subtasks":
							globalIndent.incrIndent()
							print globalIndent.getIndent() + "Processing Sub Tasks..."
							globalIndent.incrIndent()
							# Set startframe and endframe to -1 ->
							# do sub tasks for *all* frames, frame range is
							# only checked for the first hierarchy depth
							# Change below line to
							#
							#  doSubTasks( subTask, startFrame, endFrame )
							#
							# to inherit the frame sequence limit to subtasks
							holdJobList = doSubTasks( subTask, -1, -1 )
							#print holdJobList
							globalIndent.decrIndent()
							globalIndent.decrIndent()

					for subelement in element.childNodes:

						if subelement.nodeName == "commands":

							jobId = taskName + "_" + "%d" % random.randint( 0, 99999 )
							jobList += [ jobId ]

							globalIndent.incrIndent()
							print globalIndent.getIndent() + "Running Commands for '" + taskName + "'..."
							for command in subelement.childNodes:
								if command.nodeName == "command":
									globalIndent.incrIndent()
									# strip string of leading and trailing whitespace
									c = string.strip( command.firstChild.data )
									print globalIndent.getIndent() + "Cooking '" + c + "'..."
									print "_______________________________________________________________________________"
									print

									cmdarray = string.split( c, " ", -1 );

									if globalSGE:
										cmd = "sub"
										cmd += " -rman"
										cmd += " -N " + jobId
									else:
										cmd = cmdarray[ 0 ]
									
									# Convert -Progress param to -progress
									for a in cmdarray[ 1: ]:
										if a != "-Progress":
											cmd += " " + a
										else:
											cmd += " -progress"

									if holdJobList != [] and globalSGE:
										cmd += " -hold "
										for holdJob in holdJobList:
											cmd += holdJob + ","
										cmd = cmd[ :len( cmd ) - 1 ]
									
									# On UNIX the job should be launched by os.popen or the
									# like and the progress being captured and printed
									os.system( cmd )

									print "_______________________________________________________________________________"
									print "\n"
									globalIndent.decrIndent()
							globalIndent.decrIndent()
	return jobList


try:
	filename = ""
	startFrame = -1
	endFrame = -1

	try:
		optlist, args = getopt.getopt( sys.argv[ 1: ], "s:e:h", [ "start=", "end=", "startframe=", "endframe=", "help", "sge" ] )
	except:
		usage()
		sys.exit( 2 )

	for f in sys.argv[ 1: ]:
		if f[ 0 ] != '-':
			jobfile = f

	for o, a in optlist:
		if o in ( "-s", "--start" "--startframe" ):
			startFrame = int( a )
		elif o in ( "-e", "--end" "--endframe" ):
			endFrame = int( a )
		elif o == "--sge":
			globalSGE = 1
		elif o in ( "-h", "--help" ):
			sys.exit( 0 )

	try:
		scriptJob = minidom.parse( jobfile );
		print "Running commands for '" + scriptJob.getElementsByTagName( "title" )[ 0 ].firstChild.data + "':"

		doSubTasks( scriptJob.getElementsByTagName( "subtasks" )[ 0 ], startFrame, endFrame )

	except:
		print "Error parsing '" + jobfile + "'."

except:
	usage()
	sys.exit( 1 )
