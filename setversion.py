# Python program to set the version.
##############################################

import re
import sys

def fileProcess( name, lineFunction ):
	filestream = open( name, 'r' )
	if filestream.closed:
		print( "file " + name + " not open." )
		return

	output = ""
	print( "--- Processing " + name + " ---------" )
	while 1:
		line = filestream.readline()
		if not line: break
		output += lineFunction( line )
	filestream.close()
	
	if not output: return			# basic error checking
	
	print( "Writing file " + name )
	filestream = open( name, "w" );
	filestream.write( output );
	filestream.close()
	
	
def echoInput( line ):
	return line

major = input( "Major: " )
minor = input( "Minor: " )
build = input( "Build: " )

print "Setting dox,tinyxml2.h"
print "Version: " + `major` + "." + `minor` + "." + `build`

#### Write the tinyxml.h ####

def engineRule( line ):

	matchMajor = "static const int TIXML2_MAJOR_VERSION"
	matchMinor = "static const int TIXML2_MINOR_VERSION"
	matchBuild = "static const int TIXML2_PATCH_VERSION"

	if line[0:len(matchMajor)] == matchMajor:
		print "1)tinyxml2.h Major found"
		return matchMajor + " = " + `major` + ";\n"

	elif line[0:len(matchMinor)] == matchMinor:
		print "2)tinyxml2.h Minor found"
		return matchMinor + " = " + `minor` + ";\n"

	elif line[0:len(matchBuild)] == matchBuild:
		print "3)tinyxml2.h Build found"
		return matchBuild + " = " + `build` + ";\n"

	else:
		return line;

fileProcess( "tinyxml2.h", engineRule )


#### Write the dox ####

def doxRule( line ):

	match = "PROJECT_NUMBER"

	if line[0:len( match )] == match:
		print "dox project found"
		return "PROJECT_NUMBER = " + `major` + "." + `minor` + "." + `build` + "\n"

	else:
		return line;

fileProcess( "dox", doxRule )


#### Write the CMakeLists.txt ####

def cmakeRule1( line ):

	matchVersion = "set(GENERIC_LIB_VERSION"

	if line[0:len(matchVersion)] == matchVersion:
		print "1)tinyxml2.h Major found"
		return matchVersion + " \"" + `major` + "." + `minor` + "." + `build` + "\")" + "\n"

	else:
		return line;

fileProcess( "CMakeLists.txt", cmakeRule1 )

def cmakeRule2( line ):

	matchSoversion = "set(GENERIC_LIB_SOVERSION"

	if line[0:len(matchSoversion)] == matchSoversion:
		print "1)tinyxml2.h Major found"
		return matchSoversion + " \"" + `major` + "\")" + "\n"

	else:
		return line;

fileProcess( "CMakeLists.txt", cmakeRule2 )
