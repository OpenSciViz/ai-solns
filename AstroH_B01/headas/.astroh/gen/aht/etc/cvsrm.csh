#!/bin/csh -f
set dirname = ./
if ( "$1" != "" ) set dirname = "$1"
#
# forgot where i found this:
#CVS doesn't really keep directories under version control. Instead, as a kind of cheap substitute, it offers certain odd behaviors that in most cases do the "right thing". One of these odd behaviors is that empty directories can be treated specially. If you want to remove a directory from a project, you first remove all the files in it and then run update in the directory above it with the -P flag: The -P option tells update to "prune" any empty directories - that is, to remove them from the working copy. Once that's done, the directory can be said to have been removed; all of its files are gone, and the directory itself is gone (from the working copy, at least, although there is actually still an empty directory in the repository).
#
#An interesting counterpart to this behavior is that when you run a plain update, CVS does not automatically bring new directories from the repository into your working copy. There are a couple of different justifications for this, none really worth going into here. The short answer is that from time to time you should run update with the -d flag, telling it to bring down any new directories from the repository. 
#
# need to provide ref. info...
#
pushd $dirname
#
# not sure if this is necessary, but should not hurt:
cvs update -d -P
# follow sym-links but only process directories:
# 
set alldirs = `find $dirname -follow -type d`
foreach d ($alldirs)
  if( $d:t != CVS ) then
    echo attempt cvs rm of files in directory $d
    pushd $d
    set files = `\ls -1` # 
    foreach f ($files)
      if ( -d $f ) echo ignore sub-directory $f
      if ( -f $f ) then
        echo \rm $f
        echo cvs rm $f
#       \rm $f 
#       cvs rm $f
      endif
    end
     echo cvs ci -m "removed all files in $d"
#    cvs ci -m "removed all files in $d"
  endif
# return home
  popd
end
#
pwd
# prune empty directories?
echo cvs update -P
#cvs update -P 

