#!/usr/bin/env perl
# utest main and cmd-line options ...
#
package AH::UTInvoke;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  require Exporter;
  @ISA = qw(Exporter);
  @EXPORT_OK = qw($NoOp $Verbose $VeryVerbose utmain envInvoked noOpInvoked); # symbols to export on request
}

use Config;
defined $Config{sig_name} || die __FILE__, "No sigs? \n";

use File::Basename;
use File::Path;
use Getopt::Std;
use Getopt::Long;
use IO::Handle;

use Net::Domain qw(hostname hostfqdn hostdomain domainname);
use POSIX qw(:signal_h :errno_h :sys_wait_h);

use AH::UTEnv qw(setupEnv shSetup cshSetup);
use AH::UTExec qw($ToolOutlog $ToolErrlog conductTest);
use AH::UTInfo qw(briefHelp verboseHelp veryverboseHelp printConOps printDesign printRequirements printSynopsis printUseCases);
use AH::UTManifest qw(initManifest loadManifest loadAhtOpts evalManifest evalAhtOpts setDefault setDescribe setValVec updateManifest);
use AH::UTRCopy qw(rcopy);
use AH::UTTee qw(tee);
use AH::UTUtil qw($ChildExitVal $ChildDumpedCore $AbortedTest cpParfile initFileSys setSigHandler logStdErr logStdOut logDateTime exitStatCheck);
use AH::UTValidate qw(searchKeywords validate);

# the two essential hashes provided by evals of the manifest:
use vars qw(%runtime_manifest %aht_cmdopts);

# exported globals
our $NoOp = 0;
our $Verbose = 0;
our $VeryVerbose = 0;

my $rcsId = '$Name: AstroH_B01 $ $Id: b00UTInvoke.pm,v 1.1 2012/03/05 19:57:41 dhon Exp $';

sub environ {
  my $manifestfile = shift; # "./aht_manifest.pl";
  $NoOp = 1;
  if( $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  my %runtime_manifest = evalManifest($manifestfile);
  my $setup = $runtime_manifest{setup}{default};
  setupEnv($setup);
  return %runtime_manifest;
}

sub noop {
  my $manifestfile = shift; # "./aht_manifest.pl";
  $NoOp = 1;
  if( $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  my %runtime_manifest = environ($manifestfile);
  conductTest(%runtime_manifest);
}

sub debug {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if ( ! -e $manifestfile ) { initManifest($manifestfile); }
  noop($manifestfile);
  if( $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  my %runtime_manifest = evalManifest($manifestfile);
  my $toolname = $runtime_manifest{toolname}{default};
  my $execmd = "gdb $toolname";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "exec $execmd ..."); }
  exec($execmd) or logStdErr(__LINE__, "could not exec $execmd: $!");
}

sub param {
  my $manifestfile = shift; # "./aht_manifest.pl";
  $NoOp = 1;
  if( $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  my %runtime_manifest = evalManifest($manifestfile);
  my $toolname = $runtime_manifest{toolname}{default};
  if( $Verbose ) { logStdOut(__LINE__, "running plist on toolname indicated in manifest: $toolname"); }
  my $parinfo = `plist $toolname`;
  logStdOut(__LINE__, $parinfo); 
}

sub exitstatus {
  my $manifestfile = shift; # "./aht_manifest.pl";
  my $estat = shift;
  # make sure estat is non-null/empty string
  if( !$estat ) { 
    $estat = "0";
  }
  else {
    $estat = "$estat";
  }
  logStdOut(__LINE__, "set expected exit status to: $estat");
  if( $Verbose ) {
    logStdOut(__LINE__, "using tool manifest file: $manifestfile");
  }
  my $evec = $estat;
  my $vidx = index($estat, '[');
  if( $vidx < 0 ) {
    setDefault($manifestfile, "exitstatus", $estat);
    return;
  }
  # assume syntax '[0, 1, ...]' and find first comma
  my $comma = index($estat, ',');
  if( $comma > $vidx ) {
    $estat = substr($estat, 1+$vidx, $comma - $vidx - 1);
    setDefault($manifestfile, "exitstatus", $estat);
    setValVec($manifestfile, "exitstatus", $evec);
  }
}

sub update {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  my %runtime_manifest = evalManifest($manifestfile);
  my $toolname = $runtime_manifest{toolname}{default}; 
  # make sure some version of pfile is present (if none is currently in ./output)
  my $cp = cpParfile($toolname);

  # copy all currently available outputs to expected outputs
  my $expdir = './expected_output';  mkpath($expdir); # make sure subdir exists for cp/sync
  my $outdir = './output/'; # should already exist!
  my $origslnk_bahavior = $AH::UTRCopy::CopyLink;
  $AH::UTRCopy::CopyLink = 0; # need to follow symlinks and copy actual file...
  $cp = rcopy($outdir, $expdir);
  $AH::UTRCopy::CopyLink = $origslnk_bahavior; # restore default symlink behavior
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "syncd ./expected_output with current ./output content, files: $cp"); }

  # set new manifest file:
  updateManifest($manifestfile);
}

sub init {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  initManifest($manifestfile);
  # make sure input & output & etc directories exist
  initFileSys($manifestfile);
  my $ui = shift;
  if( $ui ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "set the manifest testheader: $ui"); }
    setDescribe($manifestfile, "testheader", $ui);
  }
  # if ./input iand ./output present and not-empty, update should
  # finish the (re)init via update:
  update($manifestfile);
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "tool manifest initiliazed ..."); }
}

sub toolname {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  if ( ! -e $manifestfile ) { 
    if( $Verbose ) { logStdOut(__LINE__, "no manifest found, initing tool manifest from internal reference..."); }
    initManifest($manifestfile);
    # initFileSys($manifestfile); # and might as well be sure the directory structure is ready ?
  }
  my %runtime_manifest = evalManifest($manifestfile);
  my $toolname = $runtime_manifest{toolname}{default};
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "current toolname in manifest: $toolname"); }
  my $ui = shift;
  if( $ui ) {
    my $newtoolname = $ui;
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "this may update the manifest default toolname: $newtoolname"); }
    if( index($toolname, $newtoolname) < 0 ) { # only (re)set default if needed 
      setDefault($manifestfile, "toolname", $newtoolname);
      %runtime_manifest = evalManifest($manifestfile);
    }
  }
  return %runtime_manifest;
}

sub dashdash {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  if ( ! -e $manifestfile ) { 
    initManifest($manifestfile);
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "tool manifest initiliazed ..."); }
  }
  my @ui = @_;
  my $cmdln = join(' ', @ui);
  # extract all text to the right of the double-dash:
  $cmdln = substr($cmdln, 2+index($cmdln, '--'));
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "set the manifest tool cmd-line: $cmdln"); }
  setDefault($manifestfile, "toolcmdopts", $cmdln);
}


sub handleShortOpts {
  my $manifestfile = shift; # "./aht_manifest.pl";
  logStdOut(__LINE__, "parsing $manifestfile");
  my @args = @ARGV; # preserve @ARGV
  my $opt = undef;
  my @optlist = ();
  my %opthash = ();
  my $optshort = "";
  my $optstd = "";
  my $optlong = undef;
  my $ahtkey = undef;
  my %aht_cmdopts = evalAhtOpts($manifestfile);
  foreach $ahtkey ( keys %aht_cmdopts ) {
    $optshort = substr($ahtkey, 0, 1);
    push @optlist, "$optshort";
    # $optstd =  $optstd . $optshort;
    $optstd = $optstd . $optshort . ':'; # indicate val is expected, i.e. -o val?
    logStdOut(__LINE__, "check option: $optshort");
  }
  # check short / std options first:
  logStdOut(__LINE__, "getopts for allowed short options: $optstd");
  getopts($optstd, \%opthash);
  @ARGV = @args; # restore depleted argv
  # now process contents of %opthash
  foreach $opt ( keys %opthash ) {
    if( defined $opthash{$opt} ) {
      logStdOut(__LINE__, "%opthash{$opt} = $opthash{$opt}");
    }
    else {
      logStdOut(__LINE__, "%opthash{$opt} indicated as flag, i.e. without value...");
    }
  }
}

sub handleLongOpts {
  my $manifestfile = shift; # "./aht_manifest.pl";
  # use Getopt::Long and Getopt::Std to handle cmd-line in standard fashion?
  #Getopt::Long::Configure ("bundling"); # allows '-in' in addition to '--init --noop'?
  Getopt::Long::Configure ("bundling_override", "no_ignore_case"); # -i -init --init 
  logStdOut(__LINE__, "bundling turned-on, parsing $manifestfile");
  my @args = @ARGV; # preserve @ARGV
  my $arg = undef;
  my $opt = undef;
  my @optlist = ();
  my %opthash = ();
  my $optlong = undef;
  my $optshort = undef;
  my $ahtkey = undef;
  my %aht_cmdopts = evalAhtOpts($manifestfile);
  foreach $ahtkey ( keys %aht_cmdopts ) {
    $optshort = substr($ahtkey, 0, 1);
    $optlong = $ahtkey . '|' . $optshort;  # treat all --opt=val vals as strings
    push @optlist, "$optlong:s";
    logStdOut(__LINE__, "check option: $optlong");
  }
  # check long (with bundling?)
  logStdOut(__LINE__, "getopts for allowed long options:");
  # my $ret = GetOptionsFromArray(\@args, \%opthash, @optlist ); # no such func?
  GetOptions(\%opthash, @optlist);
  @ARGV = @args; # restore depleted argv
  # now process contents of %opthash
  foreach $opt ( keys %opthash ) {
    if( defined $opthash{$opt} ) {
      logStdOut(__LINE__, "%opthash{$opt} = $opthash{$opt}");
    }
    else {
      logStdOut(__LINE__, "%opthash{$opt} indicated as flag, i.e. without value...");
    }
  }
}

sub handleCmdOpts {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$manifestfile"); }
  if( @ARGV eq 0 ) {
    logStdOut(__LINE__, "ARGV == " . @ARGV . " ... please provide at least one cmd-line option -h or -i or -t etc.");
    briefHelp($manifestfile); exit;
  }
  if( !(-e $manifestfile) ) { init($manifestfile); } 
  # preserve orig. argv  
  my @args = @ARGV;
  my $arg = undef;
  # set verbosity and save any cmdline text / args meant for tool:
  foreach $arg ( @args ) {
    if( $arg eq '-v' ) { $Verbose = 1; }
    if( $arg eq '-vv' ) { $Verbose = 1; $VeryVerbose = 1; }
    if( $arg eq '--' ) {
      # these are command-line options for the child proc.
      dashdash($manifestfile, @args);
      last;
    }
  }
  # first check for long option syntax
  # preserving @ARGV:
  handleLongOpts($manifestfile);
  # then check for short / std option syntax:
  #handleShortOpts($manifestfile);
  return ();
}

sub handleCmdLn {
  # simple minded cmd-line handler
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "using tool manifest file: $manifestfile"); }
  if( @ARGV eq 0 ) {
    logStdOut(__LINE__, "ARGV == " . @ARGV . " ... please provide at least one cmd-line option -h or -i or -t etc.");
    # briefHelp($manifestfile); exit;
    printSynopsis($manifestfile); exit;
  }
  # 
  if( !(-e $manifestfile) ) { init($manifestfile); } 
  # preserve orig. argv  
  my @args = @ARGV;
  my $arg = undef;

  # set verbosity and save any cmdline text / args meant for tool:
  foreach $arg ( @args ) {
    if( $arg eq '-v' ) { $Verbose = 1; }
    if( $arg eq '-vv' ) { $Verbose = 1; $VeryVerbose = 1; }
    if( $arg eq '--' ) {
      dashdash($manifestfile, @args);
      last;
    }
  }

  # handle cmd-line options left-to-right
  while ( $arg = shift(@args) ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "arg == $arg"); }
    #
    if( $arg eq "-h" ) {
      briefHelp($manifestfile); exit;
    }
    elsif( $arg eq "-help" ) {
      verboseHelp($manifestfile); exit;
    }
    elsif( $arg eq "--help" ) {
      veryverboseHelp($manifestfile); exit;
    }
    elsif( $arg eq "-d" || $arg eq "-debug" || $arg eq "--debug" ) {
      if( @args ) {
        my $ui = shift(@args);
        if( index($ui, '-') < 0 ) {
          # if a toolname is provided, (re)set manifest default
          %runtime_manifest = toolname($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          %runtime_manifest = toolname($manifestfile);
        }
      }
      else {
        %runtime_manifest = toolname($manifestfile);
      }
      debug($manifestfile); exit;
    }
    elsif( $arg eq "-e" || $arg eq "-env" || $arg eq "--env" ) {
      if( @args ) {
        my $ui = shift(@args);
        if( index($ui, '-') < 0 ) {
          # if a toolname is provided, (re)set manifest default
          %runtime_manifest = toolname($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          %runtime_manifest = toolname($manifestfile);
        }
      }
      else {
        %runtime_manifest = toolname($manifestfile);
      }
      noop($manifestfile);
      exit;
    }
    elsif( $arg eq "-i" || $arg eq "-init" || $arg eq "--init" ) {
      if( @args ) {
        my $ui = shift(@args);
        if( index($ui, '-') < 0 ) {
          init($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          init($manifestfile);
        }
      }
      else {
        init($manifestfile);
      }
      exit;
    }
    elsif( $arg eq "-n" || $arg eq "-no" || $arg eq "-noop" || $arg eq "--no"|| $arg eq "--noop" ) {
      if( @args ) {
        my $ui = shift(@args);
        if( index($ui, '-') < 0 ) {
          # if a toolname is provided, (re)set manifest default
          %runtime_manifest = toolname($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          %runtime_manifest = toolname($manifestfile);
        }
      }
      else {
        %runtime_manifest = toolname($manifestfile);
      }
      noop($manifestfile);
      exit;
    }
    elsif( $arg eq "-p" || $arg eq "--param" ) { 
      if( @args ) {
        my $ui = shift(@args);
        if( index($ui, '-') < 0 ) {
          %runtime_manifest = toolname($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          %runtime_manifest = toolname($manifestfile);
        }
      }
      else {
        %runtime_manifest = toolname($manifestfile);
      }
      param($manifestfile);
      exit;
    }
    elsif( $arg eq "-u" || $arg eq "-update" || $arg eq "--update" ) {
      if( @args ) {
        my $ui = shift(@args);
        if( index($ui, '-') < 0 ) { 
          # if a toolname is provided, (re)set manifest default
          %runtime_manifest = toolname($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          %runtime_manifest = toolname($manifestfile);
        }
      }
      else {
        %runtime_manifest = toolname($manifestfile);
      }
      update($manifestfile);
      exit;
    }
    elsif( $arg eq "-s" || $arg eq "-status" || $arg eq "--status" ) {
      if( @args ) {
        my $ui = shift(@args); 
        if( index($ui, '-') < 0 ) {
          %runtime_manifest = exitstatus($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          %runtime_manifest = toolname($manifestfile);
        }
      }
      else {
        %runtime_manifest = toolname($manifestfile);
      }
      exit;
    }
    elsif( $arg eq "-t" || $arg eq "-tool" || $arg eq "--tool" || $arg eq"-toolname" || $arg eq "--toolname" ) {
      if( @args ) {
        my $ui = shift(@args);
        if( index($ui, '-') < 0 ) {
          # if a toolname is provided, (re)set manifest default
          %runtime_manifest = toolname($manifestfile, $ui);
        }
        elsif( $ui && index($ui, '-') == 0 ) {
          unshift(@args, $ui); # restore
          %runtime_manifest = toolname($manifestfile);
        }
      }
      else {
        %runtime_manifest = toolname($manifestfile);
      }
    }
  } # end loop over all cmd-line args

  return %runtime_manifest;
}

sub invokedBy {
  my $invoker = shift;
# when invoked from aht main the call stack should contain something like:
# [execChild, runTool, conductTest, main] but when invoked from noop...
# [execChild, runTool, conductTest, noop, ... , main] or some such
# look for "noop" in stackTrace:
  my $trace = 1;
  my $idx = 1;
  my $stack = (caller($idx))[3];
  my $stackall = $stack;
 
  if( $Verbose ) { logStdOut(__LINE__, "$invoker ?"); }
  while( $trace ) {
    if( $stack eq 'AH::UTInvoke::utmain' ) { 
      if( $VeryVerbose ) { logStdOut(__LINE__, "Ok, reached top of call stack: $stack , saught invoker: $invoker"); } 
      return 0;
    } 
    if( $stack eq $invoker ) {
      if( $VeryVerbose ) { logStdOut(__LINE__, "Ok stack: $stack , found invoker: $invoker"); } 
      return 1;
    }
    else {
      if( $idx++ < 64 ) {
        $stack = (caller($idx))[3];
        $stackall = "$stackall , $stack";
      }
      else {
        $trace = 0;
      }
    }
  }
  logStdErr(__LINE__, "? $invoker not found after stack trace cnt: $idx ?");
  return 0;
}

sub envInvoked {
  my $invocation = __PACKAGE__ . '::environ';
  return invokedBy($invocation); # true or false?
}

sub noOpInvoked {
  my $invocation = __PACKAGE__ . '::noop';
  return invokedBy($invocation); # true or false?
}

sub finalize {
  my $manifestfile = shift;
  my $toolname = shift;
  my $olog = shift;
  my $elog = shift;

  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "> $manifestfile, $toolname, $olog, $elog ..."); }

  # finally, rename out logs for the tested toolname:
  $toolname = basename($toolname);
  my $pathprefx = dirname($olog) . '/';
  my $basename = basename($olog);
  rename($olog, $pathprefx . $toolname . "_" . $basename);

  $pathprefx = dirname($elog) . '/';
  $basename = basename($elog);
  rename($elog, $pathprefx . $toolname . "_" . $basename);
}

sub utmain {
  my $manifestfile = shift;
  my $logdatetime = logDateTime();
  my $olog = "./log/aht_stdout" . $logdatetime;
  my $elog = $olog;
  $elog =~ s/stdout/stderr/;

  # $OUTPUT_AUTOFLUSH >0 forces flush on every write or print
  # select(STDERR); $| = 1; select(STDOUT); $| = 1;
  STDERR->autoflush(1); # from IO::Handle
  STDOUT->autoflush(1); # from IO::Handle
 
  # in case no prior init has occured
  mkpath("./log"); 

  tee(STDOUT, '>', $olog);
  tee(STDERR, '>', $elog);

  # handle command-line and either exit or conduct test of indicated tool via the manifest:
  # simple minded cmd-line options parser (no 'bundling'):
  my %runtime_manifest = handleCmdLn($manifestfile);
  #
  # Getopts::Long 'bundling' cmd-line parser -- but not Getopt::Std
  # my %runtime_manifest = handleCmdOpts($manifestfile);

  if( ! %runtime_manifest ) {
    logStdOut(__LINE__, "all done ...");
    return 0;
  } 

  # end of cmd-line opts
  # if the runtime manifest has been set, proceed with the test:
  my $toolname = $runtime_manifest{toolname}{default}; 
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "> ok, using manifest toolname: $toolname ..."); }
  my $expect = $runtime_manifest{expected_output}{default}; 
  my $expexitstat = $runtime_manifest{exitstatus}{default}; 

  # ok get to it:
  my $child_pid = conductTest(%runtime_manifest);
  if( $child_pid <= 0 ) {
    logStdErr(__LINE__, "unable to test $toolname ...");
    finalize($manifestfile, $toolname, $olog, $elog);
    if( $expexitstat == $ChildExitVal ) { return 0; }
    return $ChildExitVal;
  }
  # once we get here the chikd either died or completed successfully (or not):
  if( $AbortedTest ) {
    logStdErr(__LINE__, "test has been aborted, no point in proceeding to validaion..."); 
    logStdOut(__LINE__, "test has been aborted, no point in proceeding to validaion..."); 
    finalize($manifestfile, $toolname, $olog, $elog);
    if( $expexitstat == $ChildExitVal ) { return 0; }
    return $ChildExitVal;
  }
  logStdOut(__LINE__, "$toolname tool exit status value: $ChildExitVal");
  if( $ChildDumpedCore ) {
    logStdErr(__LINE__, "child dumped core!");
    searchKeywords($manifestfile);
  }
  elsif( exitStatCheck(%runtime_manifest) ) {
    logStdErr(__LINE__, "tool exit status indicates something went wrong, search logs for keywords...");
    searchKeywords($manifestfile);
  }
  elsif( length($expect) > 0 ) {
    logStdOut(__LINE__, "tool exit status indicates success ... validate: $expect");
    validate(%runtime_manifest);
  } 
  else {
    logStdOut(__LINE__, "tool exit status indicates success? but nothing to validate: $expect");
  }

  finalize($manifestfile, $toolname, $olog, $elog);
  if( $expexitstat == $ChildExitVal ) { return 0; }
  return $ChildExitVal;
}

1;
