#!/usr/bin/env perl
# provide default / example environment setup scripts in (t)csh and (ba)sh
#
package AH::UTEnv;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  require Exporter;
  @ISA = qw(Exporter);
  @EXPORT_OK = qw($SysPFILES initSetup cshSetup shSetup setupEnv); # symbols to export on request
}

use File::Path;

use AH::UTInvoke qw($NoOp $Verbose $VeryVerbose envInvoked noOpInvoked);
use AH::UTManifest qw(initManifest loadManifest evalManifest loadAhtOpts evalAhtOpts);
use AH::UTUtil qw(initFileSys logStdErr logStdOut logDateTime);

# the two essential hashes provided by evals of the manifest:
use vars qw(%runtime_manifest %aht_cmdopts); 

# globals
our $SysPFILES = undef;

my $rcsId = '$Name: AstroH_B01 $ $Id: UTEnv.pm,v 1.12 2012/02/07 15:40:39 dhon Exp $';

sub setupEnv {
  my $sh_setup = shift;
  logStdOut(__LINE__, "setup env from: $sh_setup"); 
  # $SysPFILES = `. \${HEADAS}/headas-init.sh ; echo \$PFILES | cut -d\; -f2`;
  $SysPFILES = `. \${HEADAS}/headas-init.sh ; echo \$PFILES`;
  my $semcoln = index($SysPFILES, ';');
  $SysPFILES = substr($SysPFILES, $semcoln);
  if( $Verbose ) { logStdOut(__LINE__, "preserve original PFILES system setting: $SysPFILES"); }
  my $printenv = envInvoked();
  # my $printenv = 1;
  my $envsrc = undef;
  if($sh_setup =~ ".csh") {
    $envsrc = `source $sh_setup >& /dev/null; env | sort -u | grep -v env`;
  }
  else {
    $envsrc = `. $sh_setup >& /dev/null; env | sort -u | grep -v env`;
  }
  my @env = split(/\n/, $envsrc);
  foreach ( @env ) {
    chomp;
    next unless /=/;
    my ($var, $value) = split(/=/, $_);
    $ENV{$var} = $value;
    if( $printenv ) {
      if( $Verbose ) { logStdOut(__LINE__, "$var == $value"); }
      elsif( $var =~ 'CAL' ) { logStdOut(__LINE__, "$var == $value"); }
      elsif( $var =~ 'HEA' ) { logStdOut(__LINE__, "$var == $value"); }
      elsif( $var =~ 'PATH' ) { logStdOut(__LINE__, "$var == $value"); }
      elsif( $var =~ 'PF' ) { logStdOut(__LINE__, "$var == $value"); }
    }
  }
}
 
sub initSetup {
  my $path = shift;
  if( ! $path || length($path) <= 0 ) { $path = './etc'; }
  my $sh = $path . '/setup.sh';
  if( !(-e $sh) ) {
    logStdOut(__LINE__, "no pre-existing $sh -- creating one!");
    shSetup();
  }
  #elsif( $Verbose) {
  else {
    logStdOut(__LINE__, "found pre-existing $sh -- so NOT overwriting it!");
  }
  
  my $csh = $path . '/setup.csh';
  if( !(-e $csh) ) { 
    logStdOut(__LINE__, "no pre-existing $csh -- creating one!");
    cshSetup(); 
  }
  #elsif( $Verbose) {
  else {
    logStdOut(__LINE__, "found pre-existing $csh -- so NOT overwriting it!");
  }
}

sub shSetup {
  # be sure ./etc exist:
  my $etc = "./etc";
  mkpath($etc);
  # and use here content to create initial version of (ba)sh setup script
  my $refscript = $etc . "/setup.sh" ;
  open(OUTSCRIPT, ">$refscript") or die __LINE__, " failed to init. (ba)sh setup script: $refscript $!";
# note two space prefix '  ENDHERE' ... please do not break this...
  print OUTSCRIPT <<'  ENDHERE';
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
  ENDHERE
# note two space prefix '  ENDHERE' as indicated at start of HERE above...
# please do not break this...
  close(OUTSCRIPT) or die "Cannot close outlog! $!";
} # end of sub shSetup

sub cshSetup {
  # be sure ./etc exist:
  my $etc = "./etc";
  mkpath($etc);
  # and use here content to create initial version of (ba)sh setup script
  my $refscript = $etc . "/setup.csh" ;
  open(OUTSCRIPT, ">$refscript") or die __LINE__, "failed to init. (ba)sh setup script: $refscript! $!";
# note two space prefix '  ENDHERE' ... please do not break this...
  print OUTSCRIPT <<'  ENDHERE';
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
  ENDHERE
# note two space prefix '  ENDHERE' as indicated at start of HERE above...
# please do not break this...
  close(OUTSCRIPT) or die "Cannot close outlog! $!";
}
    
1;
