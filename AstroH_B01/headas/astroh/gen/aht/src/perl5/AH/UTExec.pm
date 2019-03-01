#!/usr/bin/env perl
# conduct tool test via IPC::Open3
#
package AH::UTExec;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  require Exporter;
  @ISA = qw(Exporter);
  # symbols to export on request
  @EXPORT_OK = qw($ChildPID $TimeOut $ToolRunTimeDir $ToolOutlog $ToolErrlog conductTest runTool execChild validChild watchFolders watchFiles);
}

use Config;
defined $Config{sig_name} || die __FILE__, "No sigs?";

use POSIX qw(:errno_h);

#use IPC::Cmd qw[can_run run];
# invoke open3 directly:
use IPC::Open3;

use IO::Handle;
use IO::Select;
use File::Basename;
use File::Copy;
use File::Path;
use Symbol; # for gensym
use Net::Domain qw(hostname hostfqdn hostdomain domainname);

use AH::UTRCopy qw(dircopy fcopy rcopy);
use AH::UTEnv qw(setupEnv);
use AH::UTInvoke qw($VeryVerbose $NoOp $Verbose envInvoked noOpInvoked);
use AH::UTUtil qw($ChildExitVal $ChildDumpedCore cpParfile lnkRuntimePath logStdErr logStdOut logDateTime setSigHandler);

# the two essential hashes provided by evals of the manifest:
use vars qw(%runtime_manifest %aht_cmdopts); 

# export this:
our $TimeOut = 0;
our $ChildPID = -1;
our $ToolRunTimeDir = undef;
our $ToolOutlog = undef;
our $ToolErrlog = undef;

my $rcsId = '$Name: AstroH_B01 $ $Id: UTExec.pm,v 1.16 2012/02/01 18:21:56 dhon Exp $';

sub watchFolders {
  my @folders = shift;
}

sub watchFiles {
  my @files = shift;
}

sub whereisProg {
  my $prog = shift;
  if( length($prog) <= 0 ) {
    logStdErr(__LINE__, "\'$prog\' not found in PATH?");
    return "0";
  }
# try IPC::Cmd::can_run?
# my $full_path = can_run($prog) or warn "$prog is not installed!";
  
  my $pathprog = undef;
  foreach my $dir (split /:/, $ENV{PATH}) {
    $pathprog = "$dir/$prog";
    if( $VeryVerbose ) { logStdOut(__LINE__, "check: $dir/$prog"); }
    if( (-x $pathprog) && !(-d $pathprog) ) {
      # presumably found executable binary file
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "found: $pathprog"); }
      return $pathprog;
    }
  }
  logStdErr(__LINE__, "\'$prog\' not found in PATH?");
  return "0";
}

sub validChild {
  my $childpid = $ChildPID;
  my $k = kill(0, $childpid);
  if( $k ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "child $childpid seems to be alive ..."); }
    return $childpid;
  }
  elsif( $k == EPERM ) { # changed uid?
    logStdErr(__LINE__, "child $childpid has escaped my control?");
  }
  elsif( $k == ESRCH ) {
    logStdErr(__LINE__, "child $childpid deceased or zombied?");
  }
  #else {
  #  # logStdErr(__LINE__, "unable check on the status of child $childpid $!\n";
  #}

  logStdOut(__LINE__, "child $childpid no longer exist: exited or died or terminated/aborted...");
  return 0;
}

sub execChild {
  # attribution:  http://www.perlmonks.org/?node_id=150748
  #  my ($exe, $exelogout, $exelogerr, @usr_input) = @_;
  my ($exe, $exelogout, $exelogerr) = @_;
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "exe: $exe \n  outlog: $exelogout \n errlog: $exelogerr"); }
  
  open(ERRLOG, ">$exelogerr") or die "Cannot open error log! $!";
  ERRLOG->autoflush(1); # from IO::Handle

  open(OUTLOG, ">$exelogout") or die "Cannot open output log! $!";
  OUTLOG->autoflush(1); # from IO::Handle
 
  my ($infh, $outfh, $errfh); # these are the FHs for our child 
  $errfh = gensym(); # create a symbol for the errfh -- open3 will not do so 

  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "stderr logging to: $exelogerr"); }
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "stdout logging to: $exelogout"); }

  my $noop = noOpInvoked();
  # my $noop = 1;
  if( $noop ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "Ok, noOp indicated, will not exec. tool test..."); }
    return 0;
  }

  eval{ $ChildPID = open3($infh, $outfh, $errfh, $exe); }; die "open3: $@\n" if $@;

  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$exe child proc. pid: $ChildPID"); }

  # if there is user input required for the tool / child stdin,
  # send it now:
  #if ( @usr_input ) {
  #  foreach my $usri (@usr_input) {
  #    print $infh $usri;
  #  }
  #  close($infh);
  #}

  my $timeout = $TimeOut;
  my $sel = new IO::Select; # create a select object to notify us on reads on our FHs
  $sel->add($outfh, $errfh); # add the FHs we're interested in

  my $tdelta = 1;
  my $eventloop = 1;
  my $line = undef;
  while( $eventloop ) {
    if( $VeryVerbose ) { logStdOut(__LINE__, "child: $ChildPID ... countdown: $timeout"); }
    # yield cpu(s) to child for a sec., allowing it to get some cputime and write ouput 
    sleep $tdelta; # this should also allow a usr interrupt (control-c) to be caught?
    # list of read ready file handles 
    my @ready = $sel->can_read; # this blocks until some i/o is available to read?
    foreach my $fh ( @ready ) {
      if( $fh == $outfh ) { # if we read from the outfh
        while( $line = <$fh> ) { # read one line from this fh
          print OUTLOG $line; # print it to output file handle
          if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$line."); }
        } 
        $sel->remove($fh); # remove it from the list and go handle the next file handle
      }
      elsif( $fh == $errfh ) { # do the same for errfh  
        while( $line = <$fh> ) { # read one line from this fh
          print ERRLOG $line;
          if( $NoOp || $Verbose ) { logStdErr(__LINE__, "$line"); }
        } 
        $sel->remove($fh); # remove it from the list and go handle the next file handle
      }
      else { # we read from something else? Should not be here?;
        logStdErr(__LINE__, "read from unkown source: $line");
      }
    }
    $eventloop = validChild();
    $timeout = $timeout - $tdelta;
    if( $timeout <= 0 ) { $eventloop = 0; }
  }
  
  close(ERRLOG) or die "Cannot close errlog! $!";
  close(OUTLOG) or die "Cannot close stdoutlog! $!";

  if( $timeout <= 0 ) {
    logStdErr(__LINE__, "child timed-out ... closed stdout/err ...");
  }
  else {
    logStdOut(__LINE__, "child exited or aborted ... closed stdout/err ...");
  }
  return $ChildPID;
}

sub runTool {
  my (%runtime_manifest) = @_;
#
  if( $NoOp || $Verbose ) {
    my $thdesc = $runtime_manifest{testheader}{describe};
    my $thdeflt = $runtime_manifest{testheader}{default};
    logStdOut(__LINE__, "$thdesc \n $thdeflt"); 
    my @testheader_valvec = @{$runtime_manifest{testheader}{valvec}};
    foreach my $val ( @testheader_valvec ) {
      logStdOut(__LINE__, "testheader: $val");
    }
  }

# the timeout value is exported:
  $TimeOut = $runtime_manifest{tooltimeout}{default};

  my $exename = $runtime_manifest{toolname}{default};
  my $exeopt = ' ' . $runtime_manifest{toolcmdopts}{default}; # or {valvec};
  my $binidx = rindex($exename, "/");
  if( $binidx >= 1 ) { # ok path present, redo and reset exename without path
    $exename = substr($exename, 1+$binidx); # exename will be used in log file names
  }
  # should be in path env:
  my $exe = whereisProg($exename) . $exeopt; # full invocation
  # presumably setupEnv has been invoked and any child proc sub-shell will enherit appropriately...
  # $exe = '. ./etc/setup.sh ; ' . $exe; # this is overkill and in fact confuses open3 i/o redirect
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "exe: $exe"); }

  # be sure all requried i/o directories exist:
  my $runtimedir = lnkRuntimePath("./output"); # set as symlnk to runtimedir/output
  my $expectout = "./expected_output"; # update copies latest output here
  my $exeinput = $runtimedir . "/input"; # copy inputs to here to preserve orig.
  my $exeoutput = $runtimedir . "/output"; #latest output ...
  my $exelogout = $exeoutput . "/stdout";
  my $exelogerr = $exeoutput . "/stderr";
  mkpath($exeinput); mkpath($expectout); mkpath($exeoutput);  mkpath($exelogout); mkpath($exelogerr);
  
  # consider new sub 'deepCopy' for this:
  # preserve symlink behavior
  my $origslnk_bahavior = $AH::UTRCopy::CopyLink;
  $AH::UTRCopy::CopyLink = 0; # need to follow symlinks and copy actual file...
  my $cpinputs = rcopy('./input/*', $exeinput); 
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "preserved orig. inputs: $cpinputs"); }
  $AH::UTRCopy::CopyLink = $origslnk_bahavior; # restore default symlink behavior

  # preserve the current log path/file names for potential keyword searches: 
  $ToolRunTimeDir = $runtimedir;
  $ToolOutlog = $exelogout = $exelogout . '/' . $exename;
  $ToolErrlog = $exelogerr = $exelogerr . '/' . $exename;

  # should be all set to exec child proc now...
  # but is this is first time ever exec of ftool, need to ensure pfile setup...
  # xis* binaries prompt with defaults ... force default prompt replies via: ???
  # $exe = "echo '\n\n\n\n\n\n\n\n\n\n\n\n' | " . $exe;
  # or:
  # my @usr_input = ['mode=h'];
  # execChild/Open3 (or IPC::Cmd) allows us to submit user input the the child procs' stdin...
  # my $pid = execChild($exe, $exelogout, $exelogerr, @usr_input);

  my $pid = execChild($exe, $exelogout, $exelogerr);
  return $pid;
}

sub conductTest {
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "using current test manifest..."); }
  my (%runtime_manifest) = @_;
  my @testcases = @{$runtime_manifest{toolname}{valvec}};
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "testcases found in manifest: \n @testcases \n"); }
  my $toolname = $runtime_manifest{toolname}{default}; # $testcases[0]?
  my $toolcmdopts =  $runtime_manifest{toolcmdopts}{default};
  logStdOut(__LINE__, "toolname == $toolname, cmdopts == $toolcmdopts");
  # my $setup = "./etc/setup.sh"; # = $runtime_manifest{setup}{default};
  my $setup = $runtime_manifest{setup}{default};
  # parse all "setenv" lines into ENV:
  setupEnv($setup);
  my $chldpid = 0; 
  my $exename = $toolname;
  
  # make sure binary exec exists (using current env)
  $exename = whereisProg($exename);
  if( ! $exename ) {
    logStdErr(__LINE__, "cannot find \'$exename\' in path");
    return $chldpid;
  }
  # if tool cmd-line indicates 'mode=h', make sure a par file is present for the tool
  # assume $home/pfiles/toolname.par is available
  if( index($toolcmdopts, 'mode=h') >= 0 ) {
    if( index($exename, '/') >= 0 ) { $exename = basename($exename); }
    my $pfcp = "./input/" . $exename. ".par";
    if( -e $pfcp ) {
      logStdOut(__LINE__, "using $pfcp to support $toolcmdopts");
    }
    else {
      logStdErr(__LINE__, "no parfile: $pfcp, try replicate to support $toolcmdopts");
      cpParfile($exename);
    }
  }
  # set separate signal handlers -- one for CLD and CHLD and on for all other sigs.
  setSigHandler();
  # and proceed
  $chldpid = runTool(%runtime_manifest);
  return $chldpid;
}

1;
