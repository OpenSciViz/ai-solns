#!/bin/bash
# presumably perl has been installed with this config:
# sh Configure -Dprefix=~/local -Dusethreads -Uuselargefiles -Duseshrplib -des
# this assumes headas6.11.x was made (via configured make)
# with --prefix=${HOME}/local on the indicated linux platform 
# with gcc ... etc. 
#
ulimit -acdflmnpstuv
cat /etc/redhat-release 
gcc -v
echo '__________________________'
#
if [ -z $LD_LIBRARY_PATH ] ; then
  export LD_LIBRARY_PATH=`pwd`/lib:${HOME}/local/lib64:${HOME}/local/lib
fi
echo "LD_LIBRARY_PATH == $LD_LIBRARY_PATH"
echo '__________________________'
#
if [ -z $HEA ] ; then
#export HEA=~/local
#export HEA=~/local/heastro
export HEA=~/local/headev
fi
#
if [ -z $CFITSIO ] ; then
  export CFITSIO=$HEA
fi
#
if [ -z $HEADAS ] ; then
# export HEADAS=${HEA}/x86_64-unknown-linux-gnu-libc2.5
  export HEADAS=${HEA}/x86_64-unknown-linux-gnu-libc2.12
fi
#
echo "HEADAS == ${HEADAS}"
echo '__________________________'
#
export ASTROH=${HEA}/astroh/x86_64-unknown-linux-gnu-libc2.12
#echo "ASTROH == ${ASTROH}"
export ASTROH_LIB=${ASTROH}/lib
export ASTROH_PERL5LIB=${ASTROH}/lib/perl
export ASTROH_BIN=${ASTROH}/bin
export ASTROH_ETC=${HEA}/astroh/etc
#
if [ -e ${HEADAS}/headas-init.sh ] ; then
  . ${HEADAS}/headas-init.sh >& /dev/null
  echo sourced ${HEADAS}/headas-init.sh
else
  echo cannot find ${HEADAS}/headas-init.sh
fi
echo '__________________________'
#
echo $PATH | grep ${ASTROH_BIN} >& /dev/null
if [ $? != 0 ] ; then
  export PATH=${ASTROH_BIN}:${PATH}
fi
echo $PATH | grep ${ASTROH_ETC} >& /dev/null
if [ $? != 0 ] ; then
  export PATH=${ASTROH_ETC}:${PATH}
fi
#
echo $PATH | grep ${USER}/local/bin >& /dev/null
if [ $? != 0 ] ; then
  export PATH=${USER}/local/bin:${PATH}
fi
echo ${PATH} | grep '\./bin' >& /dev/null 
if [ $? != 0 ] ; then
  export PATH=./bin:${PATH} 
fi
echo "PATH == $PATH"
echo '__________________________'
#
# presumably the above has set PFILES, but aht expects
# to work locally in the current working directory (cwd):
echo "reset PFILES == $PFILES" 
#export PFILES='./output;./input;'$PFILES
export PFILES='./output;./input'
echo "new setting for PFILES == $PFILES"
echo '__________________________'
#
if [ ! $?CALDB ] ; then
  export CALDB=${HOME}/local/caldb
fi
echo "CALDB == $CALDB"
echo '__________________________'
#
which perl
#export PERL5 5.8.8
#export PERL5 5.10.1
#export PERL5 5.14.2
export PERL5 `perl -v | sed 's/ /\n/g' | grep v5 | sed 's/(//g' | sed 's/)//g' | sed 's/v//'`
echo "PERL5 == ${PERL5}"
echo '__________________________'
export PERL5LIB=''
#
export PERL5LIB=~/local/lib/perl5/site_perl/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
export PERL5LIB=~/local/lib/perl5/site_perl/${PERL5}:${PERL5LIB}
export PERL5LIB=~/local/lib/perl5/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
export PERL5LIB=~/local/lib/perl5/${PERL5}:${PERL5LIB}
export PERL5LIB=${ASTROH_PERL5LIB}:${PERL5LIB}
export PERL5LIB=./:${PERL5LIB}
#
perl -le "print 'perl @INC == '; print foreach @INC;"
#
# potential full deps:
#perl -le 'use Digest::MD5; use File::Basename; use File::Compare; use File::Copy; use File::Find; use File::Find::Rule; use File::Path; use File::Tee; use Getopt::Long; use Getopt::Std; use IO::Select; use IPC::Cmd; use IPC::Open3; use IPC::Run; use Linux::Inotify2; use Net::Domain; use Params::Validate; use XML::LibXML;'
#
# current deps.:
perl -le 'use Digest::MD5; use File::Basename; use File::Compare; use File::Copy; use File::Find; use File::Path; use Getopt::Long; use Getopt::Std; use IO::Select; use IPC::Open3; use Net::Domain;'
#
# these should be somewhere else?
alias gnuarch='echo ${HEADAS##*/}'
# since bash alias does not allow args, must use bash funcs:
function hcvs {
  cvs -d:pserver:${USER}@daria.gsfc.nasa.gov:/headas $*
}
function acvs {
  cvs -d:pserver:${USER}@daria.gsfc.nasa.gov:/astrohi $*
}
