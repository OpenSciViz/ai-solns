#!/usr/bin/env perl
# some utilities like signal handlers, logging, file-sys, print help,
# formatted date-time text string, etc. 
#
package AH::UTUtil;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  require Exporter;
  @ISA = qw(Exporter);
  # symbols to export on request
  @EXPORT_OK = qw($SigRecvd $ChildExitVal $ChildDumpedCore $AbortedTest setSigHandler cpParfile initFileSys exitStatCheck trim lnkRuntimePath dataDirListTxt mkDirDateTime logStdErr logStdOut logDateTime);
}

use Config;
defined $Config{sig_name} || die __FILE__, "No sigs? \n";

use File::Basename;
use File::Copy;
use File::Path;
use Symbol; # for gensym
use Net::Domain qw(hostname hostfqdn hostdomain domainname);

use POSIX qw(:signal_h :errno_h :sys_wait_h);

use Getopt::Std;
use Getopt::Long;

use AH::UTEnv qw(initSetup setupEnv);
use AH::UTExec qw(validChild);
use AH::UTInfo qw(briefHelp verboseHelp printConOps printDesign printRequirements printUseCases);
use AH::UTInvoke qw($NoOp $Verbose $VeryVerbose);
use AH::UTManifest qw(initManifest loadManifest loadAhtOpts evalManifest evalAhtOpts setDefault updateManifest);
use AH::UTRCopy qw(rcopy);

use vars qw(%runtime_manifest %aht_cmdopts); # the two essential hashes provided by evals of the manifest

# export these:
our $SigRecvd = 0;
our $ChildExitVal = 0;
our $ChildDumpedCore = 0;
our $AbortedTest = 0;

my $rcsId = '$Name: AstroH_B01 $ $Id: UTUtil.pm,v 1.21 2012/02/03 19:09:57 dhon Exp $';

sub sighandler {
  # general purpose sighandler -- for all signals except child 
  $SigRecvd = shift;
  if( $VeryVerbose ) { logStdOut(__LINE__, "recv'd sig: $SigRecvd"); }
  if( $SigRecvd eq "INT" ) { 
    my $u = "n";
    print "abort (y/n)? [n]: ";
    # note that getc returns "\n" if one simply hits enter key
    # but if one hits any other key followed by enter, getc return the single key char
    $u = getc;
    if( $VeryVerbose ) { logStdOut(__LINE__, "you entered: \"$u\""); }
    if( $u ne "n" && $u ne "\n" ) { 
      $AbortedTest = 1;
      if( $VeryVerbose ) { logStdOut(__LINE__, "ok aborting... send SIGTERM to child(ren) ..."); }
      my $childpid = AH::UTExec::validChild();
      if( $childpid ) {
        kill('TERM', $childpid);
        if( $NoOp || $Verbose ) { logStdOut(__LINE__, "sent terminate signal to child: $childpid"); }
      }
      elsif( $NoOp || $Verbose ) { 
        logStdOut(__LINE__, "no valid children left to signal terminate?");
      }
    }
  }
}

sub sigChild {
  $SigRecvd = shift;
  if( $Verbose ) { 
    logStdOut(__LINE__, "recvd sig: $SigRecvd \n");
    logStdOut(__LINE__, "note that child coredumps set exit status to 0, but if coredumpsize limit is set to 0, \n");
    logStdOut(__LINE__, "it is not possible to determine if child exited normally -- with satus 0 -- or dumped core! \n");
  }

  # attribution: http://docstore.mik.ua/orelly/perl/cookbook/ch16_20.htm
  my $childpid = waitpid(-1, &WNOHANG); # returns childpid and sets $?
  my $childexited = WIFEXITED($?);
  my $signum  = $? & 127;
  $ChildExitVal = $? >> 8;
  $ChildDumpedCore = $? & 128;
  if( $VeryVerbose ) { logStdOut(__LINE__, "waitpid returned: $childpid, ChildExitVal == $ChildExitVal"); }

  if( $ChildDumpedCore ) {
    logStdErr(__LINE__, "child coredumped?");
  }
  elsif( $childexited ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "child process pid == $childpid exit val: $ChildExitVal"); }
  }
  # in case of unreliable signals
  $SIG{CHLD} = $SIG{CLD} = \&sigChild;
}


sub setSigHandler {
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "set sighandler for all sigs..."); }
  # for all other signals:
  my $has_nonblocking = $Config{d_waitpid} eq "define" || $Config{d_wait4} eq "define";
  if( $VeryVerbose ) { logStdOut(__LINE__, "has_nonblocking: $has_nonblocking"); }
  if( $VeryVerbose ) { logStdOut(__LINE__, "set special sighandler for child sig. ... "); }
  $SIG{CHLD} = $SIG{CLD} = \&sigChild;
  foreach my $name (split(' ', $Config{sig_name})) {
    if( $name ne 'CLD' && $name ne 'CHLD' && $name ne 'ZERO' ) {
      if( $VeryVerbose ) { logStdOut(__LINE__, "set sighandler for sig: $name"); }
      $SIG{$name} = \&sighandler;
    }
  }
}

sub exitStatCheck {
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "..."); }
  my (%runtime_manifest) = @_;
  # compare $ChildExitVal to manifest expectation:
  my $toolexit = $runtime_manifest{exitstatus}{default}; 
  logStdOut(__LINE__, "tool exit status val: $ChildExitVal ... expected: $toolexit"); 
  if( $toolexit == $ChildExitVal ) { return 0; }
  if( $toolexit eq $ChildExitVal ) { return 0; }
  return "oops!";
}

sub printHashes {
  my $ref_manifest = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$ref_manifest"); }
  my %aht_cmdopts = evalAhtOpts($ref_manifest);
  printAhtOpts(%aht_cmdopts);
  my %runtime_manifest = evalManifest($ref_manifest);
  printManifest(%runtime_manifest);
}

sub lnkRuntimePath {
  # setup sym-link to runtime ouput path/dir:
  my $runpath = shift;
  my $datedir = mkDirDateTime();
  my $output = $datedir . "/output";
  # tool runtime outputs here should be compared with expected_ouput 
  mkpath($output);
  if( -d $runpath ) { # if runpath directory already present 
    rcopy("$runpath/*", $output);
    rmtree($runpath);
  }
  my $lnk = $runpath;
  logStdOut(__LINE__, "create sym. link $lnk to: $output");
  unlink($lnk);
  my $slnk = eval { symlink($output, $lnk); 1 };
  if( ! $slnk ) { logStdErr(__LINE__, "Failed create of $output and/or sym-link to it!"); }
  return $datedir;
}

sub cpParfile {
  my $toolname = shift;
  my $cp = 0;
  if( ! $toolname || length($toolname) <= 0 ) {
    logStdErr(__LINE__, "empty or undefined toolname...");
    return $cp;
  }
  # strip any path prefix:
  if( index($toolname, '/') >= 0 ) { $toolname = basename($toolname); }
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "replicate pfile for tool: $toolname"); }
  if( length( $toolname) <= 0 ) {
    logStdErr(__LINE__, "empty or unknown toolname: $toolname");
    return $cp;
  }
  my $pfcp = './input/' . $toolname . '.par';
  my $pfile = './output/' . $toolname . '.par';
  $cp = copy($pfile, $pfcp);
  if( $cp > 0 ) {
    logStdOut(__LINE__, "replicated pfile: $pfile to: $pfcp");
    return $cp;
  }
  logStdErr(__LINE__, "failed replication of local pfile -- file not found: $pfile");
  my $hmpfile = $ENV{HOME} . '/pfiles/' . $toolname . '.par';
  $cp = copy($hmpfile, $pfcp);
  if( $cp > 0 ) {
    logStdOut(__LINE__, "replicated pfile: $hmpfile to: $pfcp");
    return $cp;
  }
  logStdErr(__LINE__, "failed replication of home pfile -- file not found: $hmpfile");
  my $syspfile = $ENV{HEADAS} . '/syspfiles/' . $toolname . '.par';
  $cp = copy($syspfile, $pfcp);
  if( $cp > 0 ) {
    logStdOut(__LINE__, "replicated system pfile: $syspfile to: $pfcp");
    return $cp;
  }
  logStdErr(__LINE__, "failed replication of system pfile -- file not found: $syspfile");
  return $cp;
}

sub initFileSys {
  my $ref_manifest = shift; # "./aht_manifest.pl";
  my $hostnm = hostfqdn();
  my %aht_cmdopts = evalAhtOpts($ref_manifest);
  my @folders = @{$aht_cmdopts{filesys}{valvec}};
  foreach my $dir ( @folders ) {
    if( $dir eq "output" || $dir eq "./output" ) {
      # output folders should be organized by hostname & date, but use sym-lnk to last/latest:
      lnkRuntimePath($dir);
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$dir present..."); } 
      next;
    }
    if( -e $dir ) { # preserve any other subdirs already present 
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$dir present..."); } 
      next;
    }
    else { # ok init an empty subdir
      mkpath($dir);
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$dir"); } 
    }
  }
  # init sample shell setup scripts in ./etc or wherev. manifest indicates:
  my %runtime_manifest = evalManifest($ref_manifest);
  my $setup = $runtime_manifest{setup}{default};
  my $filepath = dirname($setup);
  initSetup($filepath);
  #
  # init a local copy of tool pfile?
  # my $toolname = $runtime_manifest{toolname}{default}; 
  # cpParfile($toolname);
}

sub trim {
  # remove leading whitespaces
	my $string = shift;
  #if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$string"); }
	$string =~ s/^\s+//;
  # and trailing whitespaces
  $string =~ s/\s+$//;
  # also check for too many quotes:
  $string =~ s/\"\"/\"/g;
  #if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$string"); }
	return $string;
}

sub logDateTime {
  my ( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time); 
  $year += 1900;
  if ( $sec < 10 ) { $sec = "0$sec"; }
  if ( $min < 10 ) { $min = "0$min"; }
  if ( $hour < 10 ) { $hour = "0$hour"; }
  if ( $mday < 10 ) { $mday = "0$mday"; }
  my @months = ('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec');
  $mon = $months[$mon]; 
  my $log = "$year$mon$mday\@$hour\.$min\.$sec";
  return $log;
}

sub logStdErr {
  my $invoker = (caller(1))[3]; # function call stack 1 level above this sub
  my $lineno = shift;
  my $text = shift;
  if( $Verbose ) {
    print STDERR "$invoker\@$lineno> $text\n";
  }
  else {
    print STDERR "aht> $text\n";
  }
}

sub logStdOut {
  my $invoker = (caller(1))[3]; # function call stack 1 level above this sub
  my $lineno = shift;
  my $text = shift;
  if( $Verbose ) {
    print STDOUT "$invoker\@$lineno> $text\n";
  }
  else {
    print STDOUT "aht> $text\n";
  }
}

sub dataDirListTxt {
  # try return list of 'data' files found in this directory
  # fits or (ancil) txt or ? as a single comma separated string
  my $dirpath = shift;
  opendir(IND, $dirpath) || die "Cannot open directory $dirpath";
  my @files = sort( readdir(IND) );
  closedir(IND);
  my $valvectxt = "";
  foreach my $f ( @files ) {
    unless ( ($f eq "CVS") || ($f eq ".") || ($f eq "..") || 
             (index($f, '.par') >= 0) || (index($f, 'stderr') >= 0) || (index($f, 'stdout') >= 0) ) {
      $valvectxt = $valvectxt . '"' . $f . '", ';
    }
  }
  chop($valvectxt); chop($valvectxt); # elim trailing ', '
  trim($valvectxt); # and elim any other leading or trailing whitespaces and "" -> " 
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "text of valvec: \'$valvectxt\'"); }
  return $valvectxt;
} 

sub mkDirDateTime {
  # create directory for this host and current date-time, and return string
  my ( $sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time); 
  $year += 1900;
  if ( $sec < 10 ) { $sec = "0$sec"; }
  if ( $min < 10 ) { $min = "0$min"; }
  if ( $hour < 10 ) { $hour = "0$hour"; }
  if ( $mday < 10 ) { $mday = "0$mday"; }
  my @months = ('Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec');
  $mon = $months[$mon];
  my $hostnm = hostfqdn();
  my $datedir = "./$hostnm/$year$mon$mday/$hour\.$min\.$sec"; 
  mkpath($datedir); # tool runtime outputs here should be diffed with expected 
  return $datedir; # keep track of the last/latest outputs for potential updates
}

1;

