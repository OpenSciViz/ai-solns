#!/bin/csh -f
# attempt to recursively add directory tree to existing cvs repos.
# this assumes no CVS subdirectories are present -- i.e. this
# should behave somewhat like an import, except the result should
# yield freshly created and populated CVS sub-directories.
# but it does require a CVS directory at the top-level!
set dirname = ./
if ( "$1" != "" ) set dirname = "$1"
if ( ! -d $dirname ) then
  echo top level directory does not exist, abort!
  exit
endif
if ( ! -d $dirnamei/CVS ) then
  echo top level CVS directory does not exist, abort!
  exit
endif
# 
# while we want to make use of the existing top level $dirname/CVS directory,
# any newly added sub-directories should not contain an (old/duplicate?)
# CVS subdirectory? but on the other hand if the subdirectory already exits
# in CVS and we are simply adding new files, we don;t want to remove
# any existing CVS sub-directory -- a bit of a conundrum... 
#
# follow sym-links but only process directories:
#
pushd $dirname
#
# not sure if this is necessary, but should not hurt:
# cvs update -d -P
#
set alldirs = `find $dirname -follow -type d`
foreach d ($alldirs)
  if( $d:t != CVS ) then
    echo attempt cvs add of directory $d
    pushd $d/..
# evidently any attempt to add a directory that already exists fails with:
# "cvs [add aborted]: there is a version in <whatever> already"
    echo cvs add -m "initial add of directory $d:t"
#   cvs add -m "initial add of direcotory $d:t"
# evidently this is not necessary, but should not hurt:
#    echo cvs ci  -m "initial checkin of directory $d"
#   cvs ci -m "initial checkin of direcotory $d"
  endif
# return home
  popd
end
#
pwd
# not sure if this is necessary, but should not hurt:
# cvs update -d -P
#
foreach d ($alldirs)
  if( $d:t != CVS ) then
    echo attempt cvs add of all files in directory $d
    pushd $d
    set files = `\ls -1` # 
    foreach f ($files)
      if ( -d $f ) echo ignore sub-directory $f
      if ( -f $f ) then
        echo cvs add -m "adding new file" $f
#       cvs add -m "adding new file" $f
      endif
    end
# not sure if this is necessary:
    cd ..
    echo cvs ci -m "added all files in $d"
#    cvs ci -m "added all files in $d"
  endif
# return home
  popd
end
#
pwd
# prune empty directories?
echo cvs update -P
#cvs update -P 

