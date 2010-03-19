#!/bin/bash

# -------------------------------------------------------------------------- #
# Copyright 2002-2010, OpenNebula Project Leads (OpenNebula.org)             #
#                                                                            #
# Licensed under the Apache License, Version 2.0 (the "License"); you may    #
# not use this file except in compliance with the License. You may obtain    #
# a copy of the License at                                                   #
#                                                                            #
# http://www.apache.org/licenses/LICENSE-2.0                                 #
#                                                                            #
# Unless required by applicable law or agreed to in writing, software        #
# distributed under the License is distributed on an "AS IS" BASIS,          #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
# See the License for the specific language governing permissions and        #
# limitations under the License.                                             #
#--------------------------------------------------------------------------- #

#-------------------------------------------------------------------------------
# DIR DEFINITIONS
#-------------------------------------------------------------------------------

DOC_DIR="./doc"
BIN_DIR="./bin"
JAR_DIR="./jar/org.opennebula.client.jar"
LIB_DIR="./lib"
#-------------------------------------------------------------------------------
# COMMAND LINE PARSING
#-------------------------------------------------------------------------------
usage() {
 echo
 echo "Usage: build.sh [-d ] [-h] [-s]"
 echo
 echo "-d: build the documentation"
 echo "-s: compile the examples"
 echo "-h: prints this help"
}
#-------------------------------------------------------------------------------

TEMP_OPT=`getopt -o hsd -n 'build.sh' -- "$@"`

eval set -- "$TEMP_OPT"

DO_DOC="no"
DO_EXA="no"

while true ; do
    case "$1" in
        -h) usage; exit 0;;
        -d) DO_DOC="yes"; shift ;;
        -s) DO_EXA="yes"; shift ;;
        --) shift ; break ;;
        *)  usage; exit 1 ;;
    esac
done

#-------------------------------------------------------------------------------
# BUILD FUNCTIONS
#-------------------------------------------------------------------------------

do_documentation()
{
    echo "Generating javadocs..."

    rm -rf $DOC_DIR > /dev/null 2>&1 
    mkdir -p $DOC_DIR
    javadoc -quiet -classpath $LIB_DIR"/*" -d $DOC_DIR \
      -sourcepath ./src/ \
      -subpackages org.opennebula \
      -windowtitle 'OpenNebula Cloud API' \
      -doctitle 'OpenNebula Cloud API Specification' \
      -header '<b>OpenNebula</b><br><font size="-1">Java API</font>' \
      -bottom 'Visit <a
href="http://opennebula.org/">OpenNebula.org</a><br>Copyright 2002-2010 &copy;
OpenNebula Project Leads (OpenNebula.org).'
}

do_jar()
{
    rm -rf $BIN_DIR > /dev/null 2>&1
    mkdir -p $BIN_DIR

    echo "Compiling java files into class files..."
    javac -d $BIN_DIR -cp $LIB_DIR"/*" `find src -name *.java`

    if [ $? -eq 0 ]; then
        echo "Packaging class files in a jar..."
        jar cf $JAR_DIR -C $BIN_DIR org
    fi
}

do_examples()
{
    echo "Compiling OpenNebula Cloud API Examples..."
}

do_jar

if [ "$DO_DOC" = "yes" ] ; then
    do_documentation
fi

if [ "$DO_EXA" = "yes" ] ; then
    do_examples
fi
