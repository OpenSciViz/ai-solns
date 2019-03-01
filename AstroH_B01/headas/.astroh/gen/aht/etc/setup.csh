#!/bin/csh -f
# presumably perl has been installed with this config:
# sh Configure -Dprefix=~/local -Dusethreads -Uuselargefiles -Duseshrplib -des
# this assumes headas6.11.x was made (via configured make)
# with --prefix=${HOME}/local on the indicated linux platform 
# with gcc ... etc. 
#
unlimit ; limit
cat /etc/redhat-release 
gcc -v
echo '__________________________'
#
if ( ! $?LD_LIBRARY_PATH ) then
  setenv LD_LIBRARY_PATH `pwd`/lib:${HOME}/local/lib64:${HOME}/local/lib
endif
echo "LD_LIBRARY_PATH == $LD_LIBRARY_PATH"
echo '__________________________'
#
if ( ! $?HEA ) then
#setenv HEA ~/local
#setenv HEA ~/local/heastro
setenv HEA ~/local/headev
endif
#
if ( ! $?CFITSIO ) then
  setenv CFITSIO $HEA
endif
#
if ( ! $?HEADAS ) then
# setenv HEADAS ${HEA}/x86_64-unknown-linux-gnu-libc2.5
  setenv HEADAS ${HEA}/x86_64-unknown-linux-gnu-libc2.12
endif
#
echo "HEADAS == ${HEADAS}"
echo '__________________________'
#
setenv ASTROH ${HEA}/astroh/x86_64-unknown-linux-gnu-libc2.12
#echo "ASTROH == ${ASTROH}"
setenv ASTROH_LIB ${ASTROH}/lib
setenv ASTROH_PERL5LIB ${ASTROH}/lib/perl
setenv ASTROH_BIN ${ASTROH}/bin
setenv ASTROH_ETC ${HEA}/astroh/etc
#
if ( -e ${HEADAS}/headas-init.csh ) then
  source ${HEADAS}/headas-init.csh >& /dev/null
  echo sourced ${HEADAS}/headas-init.csh
else
  echo cannot find ${HEADAS}/headas-init.csh
endif
echo '__________________________'
#
echo $PATH | grep ${ASTROH_BIN} >& /dev/null
if ( $? != 0 ) then
  setenv PATH ${ASTROH_BIN}:${PATH}
endif
echo $PATH | grep ${ASTROH_ETC} >& /dev/null
if ( $? != 0 ) then
  setenv PATH ${ASTROH_ETC}:${PATH}
endif
#
echo $PATH | grep ${USER}/local/bin >& /dev/null
if ( $? != 0 ) then
  setenv PATH ${USER}/local/bin:${PATH}
endif
echo ${PATH} | grep '\./bin' >& /dev/null 
if ( $? != 0 ) then
  setenv PATH ./bin:${PATH} 
endif
echo "PATH == $PATH"
echo '__________________________'
#
#
# in the event any of the above reset PATH:
rehash
# presumably the above has set PFILES, but aht also needs
# to work locally in the current working directory (cwd):
echo "reset PFILES == $PFILES"
#setenv PFILES './output;./input;'$PFILES
setenv PFILES './output;./input'
echo "new setting for PFILES == $PFILES"
echo '__________________________'
#
if ( ! $?CALDB ) then
  setenv CALDB ${HOME}/local/caldb
endif
echo "CALDB == $CALDB"
echo '__________________________'
#
which perl
#setenv PERL5 5.8.8
#setenv PERL5 5.10.1
#setenv PERL5 5.14.2
setenv PERL5 `perl -v | sed 's/ /\n/g' | grep v5 | sed 's/(//g' | sed 's/)//g' | sed 's/v//'`
echo "PERL5 == ${PERL5}"
echo '__________________________'
setenv PERL5LIB ''
#
#setenv PERL5LIB ~/local/lib64/perl5/vendor_perl/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5/vendor_perl/${PERL5}:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5/vendor_perl:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5/site_perl/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5/site_perl/${PERL5}:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5/site_perl:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5/${PERL5}:${PERL5LIB}
#setenv PERL5LIB ~/local/lib64/perl5:${PERL5LIB}
#setenv PERL5LIB ~/local/lib/perl5/vendor_perl/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
#setenv PERL5LIB ~/local/lib/perl5/vendor_perl/${PERL5}:${PERL5LIB}
#setenv PERL5LIB ~/local/lib/perl5/vendor_perl:${PERL5LIB}
setenv PERL5LIB ~/local/lib/perl5/site_perl/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
setenv PERL5LIB ~/local/lib/perl5/site_perl/${PERL5}:${PERL5LIB}
#setenv PERL5LIB ~/local/lib/perl5/site_perl:${PERL5LIB}
setenv PERL5LIB ~/local/lib/perl5/${PERL5}/x86_64-linux-thread-multi:${PERL5LIB}
setenv PERL5LIB ~/local/lib/perl5/${PERL5}:${PERL5LIB}
#setenv PERL5LIB ~/local/lib/perl5:${PERL5LIB}
setenv PERL5LIB ${ASTROH_PERL5LIB}:${PERL5LIB}
setenv PERL5LIB ./:${PERL5LIB}
#
perl -le "print 'perl @INC == '; print foreach @INC;"
#
#
# potential full deps:
#perl -le 'use Digest::MD5; use File::Basename; use File::Compare; use File::Copy; use File::Find; use File::Find::Rule; use File::Path; use File::Tee; use Getopt::Long; use Getopt::Std; use IO::Select; use IPC::Cmd; use IPC::Open3; use IPC::Run; use Linux::Inotify2; use Net::Domain; use Params::Validate; use XML::LibXML;'
#
# current deps.:
perl -le 'use Digest::MD5; use File::Basename; use File::Compare; use File::Copy; use File::Find; use File::Path; use Getopt::Long; use Getopt::Std; use IO::Select; use IPC::Open3; use Net::Domain;'
#
# these should be somewhere else?
alias gnuarch 'echo $HEADAS:t'
alias hcvs 'cvs -d:pserver:${USER}@daria.gsfc.nasa.gov:/headas \!*'
alias acvs 'cvs -d:pserver:${USER}@daria.gsfc.nasa.gov:/astroh \!*'
