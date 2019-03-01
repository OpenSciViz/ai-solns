#!/bin/csh -f
#
# prepare directory for recursive cvsadd (alternative to cvs import)
# by removing all sym-links and and old/stale CVS or .svn sudirectories
# and any other items to be excluded from cvs add...
#
set dirname = ./
if ( "$1" != "" ) set dirname = "$1"
#
pushd $dirname
#
# rm all CVS sub-directories and all sym-links 
#
# 
find ./ -type l -exec unlink {} \;
find ./ -type d -name .svn -exec rm -rf {} \;
find ./ -type d -name CVS -exec rm -rf {} \;
popd

