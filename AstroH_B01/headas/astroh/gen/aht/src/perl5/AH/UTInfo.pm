#!/usr/bin/env perl
# various verbosity help and encapsulated documentation.
#
package AH::UTInfo;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  require Exporter;
  @ISA = qw(Exporter);
  # symbols to export on request
  @EXPORT_OK = qw(briefHelp verboseHelp veryverboseHelp printManifest printAhtOpts printConOps printDesign printRequirements printSynopsis printUseCases);
}

use File::Path;

use AH::UTInvoke qw($VeryVerbose $NoOp $Verbose envInvoked noOpInvoked);
use AH::UTManifest qw(initManifest loadManifest evalManifest loadAhtOpts evalAhtOpts);
use AH::UTUtil qw($ChildExitVal $ChildDumpedCore $AbortedTest initFileSys setSigHandler logStdErr logStdOut logDateTime exitStatCheck);

# the two essential hashes provided by evals of the manifest:
use vars qw(%runtime_manifest %aht_cmdopts); 

my $rcsId = '$Name: AstroH_B01 $ $Id: UTInfo.pm,v 1.7 2012/02/03 19:09:57 dhon Exp $';

sub printAhtOpts {
  my (%aht_cmdopts) = @_;
  foreach my $ahtkey ( keys %aht_cmdopts ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "ahtkey == $ahtkey"); }
  # attribution -- http://www.kf6nvr.net/blog/archives/000727.html:
    my %opthash = %{$aht_cmdopts{$ahtkey}};
    foreach my $optkey ( keys %opthash ) {
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$ahtkey $optkey == $opthash{$optkey}"); } 
      if( $optkey eq 'valvec' ) {
        foreach my $o ( 0 .. $#{ $opthash{$optkey} } ) {
          my $optval = $opthash{$optkey}[$o];
          logStdOut(__LINE__, "$ahtkey $optkey # $o == $optval");
        }
      }
      elsif( $optkey ne 'filesys' && $optkey ne 'manifest' ) {
        # only show currently suppored options:
        logStdOut(__LINE__, "$ahtkey $optkey == $opthash{$optkey}");
      }
    }
  }
}

sub describeAhtOpts {
  my (%aht_cmdopts) = @_;
  foreach my $ahtkey ( keys %aht_cmdopts ) {
    my %opthash = %{$aht_cmdopts{$ahtkey}};
    foreach my $optkey ( keys %opthash ) {
      if( $optkey eq 'describe' ) {
        logStdOut(__LINE__, "$ahtkey $optkey == $opthash{$optkey}");
      }
    }
  }
}

sub printManifest {
  my (%runtime_manifest) = @_;
  foreach my $rtkey ( keys %runtime_manifest ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "rtkey == $rtkey"); }
  # attribution -- http://www.kf6nvr.net/blog/archives/000727.html:
    my %rthash = %{$runtime_manifest{$rtkey}};
    foreach my $mkey ( keys %rthash ) {
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$rtkey $mkey == $rthash{$mkey}"); } 
      if( $mkey eq 'valvec' ) {
        foreach my $m ( 0 .. $#{ $rthash{$mkey} } ) {
          my $mval = $rthash{$mkey}[$m];
          logStdOut(__LINE__, "$rtkey $mkey # $m == $mval");
        }
      }
      else {
        logStdOut(__LINE__, "$rtkey $mkey == $rthash{$mkey}");
      }
    }
  }
}

sub printHashes {
  my $ref_manifest = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$ref_manifest"); }
  my %aht_cmdopts = evalAhtOpts($ref_manifest);
  printAhtOpts(%aht_cmdopts);
  my %runtime_manifest = evalManifest($ref_manifest);
  printManifest(%runtime_manifest);
}

sub briefHelp {
  my $manifestfile = shift; # "./aht_manifest.pl";
  # if ( ! -e $manifestfile ) { initManifest($manifestfile); }
  logStdOut(__LINE__, "Please note that AstroH_B00 (build 0) aht command-line arg. handling is very simple-minded,");
  logStdOut(__LINE__, "and only one option at a time is properly handled -- bundling aka \'aht -dehinpstuv ...\'");
  logStdOut(__LINE__, "will recognize -d and ignore the rest!");
  my %aht_cmdopts = evalAhtOpts($manifestfile);   
  describeAhtOpts(%aht_cmdopts);
}

sub verboseHelp {
  my $manifestfile = shift; # "./aht_manifest.pl";
  # if ( ! -e $manifestfile ) { initManifest($manifestfile); }
  # briefHelp($manifestfile);
  # my %runtime_manifest = evalManifest($manifestfile);   
  # printManifest(%runtime_manifest);
  briefHelp($manifestfile);
  printUseCases($manifestfile);
  printSynopsis($manifestfile);
}

sub veryverboseHelp {
  my $manifestfile = shift; # "./aht_manifest.pl";
  # if ( ! -e $manifestfile ) { initManifest($manifestfile); }
  verboseHelp($manifestfile);
  printDesign($manifestfile);
  printConOps($manifestfile);
}

#
# the following shall simply make use of HERE blocks of text extracted from MSOffice docs, ppts, xls, and/or pdfs
#

sub printUseCases {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "manifest file == $manifestfile"); }
  print <<'  ENDUseCase'; 

Astro-H  SoftCal Analysis System Use Case Terminology
Command Line Tool (Tool): an executable tool (compiled binary, or a script) that may be run from the shell command line, 
and whose user inputs are entirely provided through the SAO Host-based parameter interface. Example: ftlist.
Command Line Tool Test Case (Test Case): a Test Case consists of the runtime environment, the set of required inputs,
and the set of expected outputs that uniquely determine the correct behavior for a specific mode of execution.
Command Line Tool Test (Tool Test): an executable tool (script) that tests one Test Case for a Tool, analyzes the out
come, and reports the results.
Command Line Unit Test (CLUT): an executable tool (script) that performs all the defined Tool Tests for given Tool.
Operating Environment: the set of environment variables needed by a Test Case, and the working directory.
Parameters (Params): unless otherwise noted, this refers specifically to standard SAO Host-based parameter file inter
face parameters.
Input and Output Files: data files in any format (e.g., FITS or text) that are used or produced by the software.
Tool Exit Status: the status returned to the shell environment when a Tool exits.
Text Output: the text displayed by a Tool and sent to the standard output and standard error streams.
Command Line Tool Test Manifest (Manifest): a file listing all inputs and outputs files needed for a Tool Test.

Calibration Database (CALDB)
Operating Environment
Tool Name and full path to binary executable
Tool Test Inputs
Input Parameters
Input Data File List and Reference Input Data File(s)
Operating Environment
Expected Tool Exit Status and List of all possible Exit Status
Expected Output Parameters
Expected Output Data File List and Reference Output Data File(s)
Expected Text Output

Report Tool Exit Status and Analyze Tool Test Results

  ENDUseCase
  print "\n";
}

sub printRequirements {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "manifest file == $manifestfile"); }
  print <<'  ENDReqs'; 

Generic Test Scripts: because Command Line Tool behavior is defined by its inputs and outputs,
and because these inputs and outputs may be specified in a completely generic way, the Tool Test and the
CLUT shall be implemented generically. These utilities shall thus be capable of testing any Command Line Tool.
Tool Test: this shall be implemented as an executable script that is callable from the command line.
The script shall only invoke a top-level library function that is also available to the CLUT script.
CLUT: this shall be implemented as an executable script that is callable from the command line.
It shall utilize the top-level library function defined in the Tool Test above to run each individual Tool Test.
A File/Directory Convention for input and output files shall be developed that allows the Tool Test and CLUT to
determine as much as possible about the Test Case directly from the files located in the local test area.
The Tool Test and CLUT shall be capable of accepting all remaining required inputs and expected outputs on the command line.
The Tool Test and CLUT shall also be capable of saving all inputs and outputs in the Manifest file.
Developers shall be able to create Tool Tests naturally while developing code, and validate the outputs manually.
Developers shall be able easily to generate, from the current configuration of directories and files, the Manifest file,
and to update the expected output files based on the manually validated output files.
The developer runs the Tool by hand, specifying input parameters on the command line.
Parameters shall locate input and output data files according to the File/Directory Convention.
Run-time Directory: the actual test command required by any Tool Test shall be executed in the output subdirectory
of the unit test directory
The input parameter file shall cause all input data to be taken from the input subdirectory of the unit test directory.
The input parameter file shall cause all output data to be written to the output subdirectory of the unit test directory.

  ENDReqs
  print "\n";
}

sub printSynopsis {
  my $ref_manifest = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "ref. manifest file == $ref_manifest"); }
  print <<'  ENDSynopsis';

Aht is a Perl script meant to support unit test during development and for acceptance and regression testing
of the Astro-H FITS tool (Ftool) binary apps.
The Astro-H Ftool app. developer establishes test input and runs the tool app. manually to produce output.
A Manifest text file -- implemented as a perl hash statement -- indicates the nature and specifics of a unit test. 
The Astro-H test Perl script (aht) executes the Ftool app. as a child proc., once expected output has been established 
(aht -u copies successful output to the special sub-directory './expected_output/').
All Ftool app. printout to stdout and stderr are redirected by aht to text log files.
The user may interrupt an aht test via Control-C (interrupt signal).
Aht monitors child proc. signals to determine if the Ftool app. completes execution successfully or fails,
and reports its exit value.
Upon a successful exit value, aht compares/validates output with what is expected.
On unsuccessful exit value, aht searches logs for keywords (described in the Manifest) and reports accordingly.

Usage: aht [Arguments] 

Arguments that have evolved as result of prototype:

Was : [ -n | --name tool-name ]: name of Tool to run for this Tool Test (needed only with -u)
Changed (due to -noop ambiguity): [-t | --tool | --toolname whatever ]: name of Tool to run for this Tool Test.
Prototype behavior: Manifest file is updated with new tool name and test is attempted.

Was: [ -t | --text test-case-hint ]: cryptic test description (needed only with -u)
Changed (due to above change): [  -i | --init test-case-hint/header ]: cryptic test description 

Was: [ Tool Arguments ]: arguments valid for the Tool being tested.
Changed (for syntax clarity): [ -- Tool Arguments ]: arguments valid for the Tool being tested.

Arguments: 

[ -- Tool Arguments ]: arguments valid for the Tool being tested.
Prototype behavior: parses all the text following the '--' and inserts as-is into the manifest.

[ -d | --debug ]: set up the test environment and then start the Ftool app. binary in the gdb debugger
Prototype behavior: execs gdb, but currenlty there is no '~/.gdbinit'.

[ -e | --env ]: display all environment variables as set prior to executing the Tool
Prototype behavior: mostly cleaned-up.

[ -h | --help ]: display help and exit
Prototype behavior: prints this Synopsis.

[ -i | --init test-case-hint/header ]: cryptic test description 
Prototype behavior: manifest file is initialized with optional text header, if provided in cmd-line.

[ -n | --noop ]: display the command that would be executed if the Tool ran but do not actually run the Tool
Prototype behavior: mostly cleaned-up.

[ -s | --status expected-status ]: integer value expected when the Tool exits (needed only with -u)
Prototype behavior: manifest file updated with default and list (valvec) values.

[-t | --tool | --toolname whatever ]: name of Tool to run for this Tool Test.
Prototype behavior: manifest file is updated with new tool name (if provided). Test is conducted. Exit status is checked
                    and unsuccessful tests are partially handled (keyword seach is TBD). Successfull test
                    validation is TBD.

[ -u | --update ]: replace the contents of the output-expected subdirectory with the current files in the output
                   subdir ectory; used when the expected behavior of the tool changed, and the new output has been
                   validated by the developer. This option will also update the inventory of input and expected 
                   output in the Manifest file.
Prototype behavior: all ouputs found are replicated to ./expected_output/... and manifest file content inventory is (re)set.

[ -p | --par ]: display all parameter values as set prior to executing the Tool
Prototype behavior: simply invokes the standard Ftool 'plist' via perl backtick and prints to stdout.

Generic Tool Test (aht.pl) Example invocations:

Example 1: create (update) a test case for ftlist; just display summary of file:
design-spec.: aht --name ahdemo --status=0 --text display-file-summary --update  infile=./input/ftlist.fits  option=HK
build 0 prototype: aht -i[--init] 'test header info'
or         aht -t[--test/tool] ahdemo  -- option=HK 
or         aht -u[--update] ahdemo -s 0

Example 2: run this test case, which was already set up (in the current manifest):
design-spec.: aht
build 0 prototype: aht -t

Example 2: print test case scenario described in the current manifest, doing everythong short of running the ftool app. (so-called no-operation):
build 0 prototype: aht -n[--noop] 

Example 4: debug this test case, which was already set up:
build 0 protoype: aht.pl -d[--debug]

Example 5: run this test case, which was already set up, but display all information about the test environment and parameter input:
build 0 prototype: aht.pl -p[--par] -e[--env]

  ENDSynopsis
  print "\n";
}

sub printConOps {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "manifest file == $manifestfile"); }
  printSynopsis($manifestfile);
  print <<'  ENDConOps';

The developer sets up empty test area ('aht.pl -i' may assist with this) and (optionally) customizes an initialization
script that sets up the test environment (e.g. setup.sh for a Tool-specific CALDB set-up).
The developer creates/copies test input data into location for input data as specified by the File/Directory Convention.
The developer runs the Tool by hand, specifying input parameters on the command line. Parameters shall locate input a
nd output data files according to the File/Directory Convention.
The developer inspects, verifies and validates the output to ensure the Tool output is correct, then runs the Tool Test
(aht.pl) in 'update' mode.
The developer must specify the name of the Tool and the expected exit status. The update mode uses the File/Directory 
Convention to locate and copy all current inputs and outputs to their reference locations.
In the infrequent case where the output parameter file is different from the input parameter file, it may be necessar
y to restore the input parameter file by hand.

Directory and File Conventions
Run-time Directory: the actual test command required by any Tool Test shall be executed in the output subdirectory of the unit test directory
Input Parameter File:
The input parameter file shall cause all input data to be taken from the input subdirectory of the unit test directory.
The input parameter file shall cause all output data to be written to the output subdirectory of the unit test directory.

Astro-H Analysis System Architecture
HEASARC Build System (hmake tool, standard Makefiles)
Typical GNU build pattern: configure, make, make install
Hmake tool supports full system build and installation
Hmake also supports build/installation of  individual components
Makefiles are simple; use standard patterns
Standard targets supported: all, default, install, test, clean, distclean
Staged installation:
Hmake (build step): for each component (library/tool):
First build, then install in BLD (staged area)
Libraries and their public headers are gathered in one place
Hmake install (installation step): for each component (library/tool):
Put files in target location
Location can be set by configure using --prefix
this is under construction...
Directory Layout
Utilities

TODO: put in the tree here
TODO: explain this: Header File Conventions

Event-Processing Tool Flow:
Std. Startup: Get Pars, Query user, Initialize/Allocate buffers, Open files
Read/buffer evts, Do work Algorithms, Write/buffer evts. to FITS
Last evt? Finalize, Free buffers, Closefiles

Standard behaviors for all tools
Event Processing Flow Description Step:

1)
Startup
Astro-H Lib
Global initializations
Independent of user input

2) Get Pars
Astro-H Lib/Ape -- Get tool-specific parameters from user
Library simplifies/standardizes tool code

3.) Initialize
init/
bgnrun
Tool/
Astro-H Lib/Cfitsio
Tool
Initializations that depend on user input
Library opens input files
Library sets up efficient tool-specific buffering, etc.

4.) Read/buffer events
ana/bank
Astro-H Lib/Cfitsio
Astro-H
Tool calls library to get event data
Library calls cfitsio and buffers data efficiently
Library supports ¿get next event¿

5.) Do Work -- ANL Parallel Where Implemented
ana/bank
Tool/
Astro-H Lib
Tool
Tool calls algorithms from library
Algorithms conform to standard interfaces

6.) Write/buffer events
ana/bank
Astro-H Lib/Cfitsio
Astro-H
Tool calls library to write event data
Library calls cfitsio and buffers data efficiently
May not be necessary depending on tool

7.) Last evt?
ana
Library keeps track of input files
If input files not completely processed, return to step 4

8.) Finalize
Reformatting and Calibration Algorithm Attributes:
Define Record as one row of data in only the columns of interest
Example: suppose the FITS file has columns TIME, ENERGY, RA, DEC, THETA, PHI, and ZENITH_ANGLE. Assume only RA and DEC
are relevant for a given algorithm. The Record in this case would be { RA, DEC }.
Required: a generic way to specify the contents of a Record, i.e., which columns are to be read.

Row-based Algorithm Specification
A row in a FITS table has data {x1, x2, ..., xn}
An algorithm is an operation f(R), where R is a Record

  ENDConOps
  print "\n";
}

sub printDesign {
  my $manifestfile = shift; # "./aht_manifest.pl";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "manifest file == $manifestfile"); }
  print <<'  ENDDesign';

There are currently 9 Perl modules (AH::) in the aht design implementation that export
the globals and functions listed below. The aht script itself is simply an entry mechanism
into AH::UTInvoke::utmain(), and currently uses a hard-coded manifest file: './aht_manifest.pl'.

AH::UTEnv exports global variable: $SysPFILES
AH::UTEnv exports functions: cshSetup shSetup setupEnv)

AH::UTExec exports global variables: $ChildPID $TimeOut $ToolRunTimeDir $ToolOutlog $ToolErrlog
AH::UTExec exports functions: conductTest runTool execChild validChild watchFolders watchFiles

AH::UTInfo exports functions: briefHelp verboseHelp veryverboseHelp printManifest printAhtOpts printConOps printDesign printRequirements printSynopsis printUseCases

AH::UTInvoke exports global variables: $NoOp $Verbose $VeryVerbose 
AH::UTInvoke exports functions: utmain envInvoked noOpInvoked

AH::UTManifest exports functions: initManifest loadManifest loadAhtOpts evalManifest evalAhtOpts setDefault setDescribe setValVec updateManifest)

AH::UTRCopy exports functions: fcopy rcopy dircopy fmove rmove dirmove pathmk pathrm pathempty pathrmdir

AH::UTTee exports functions: tee

AH::UTUtil exports global variables: $SigRecvd $ChildExitVal $ChildDumpedCore $AbortedTest
AH::UTUtil exports functions: setSigHandler cpParfile initFileSys exitStatCheck trim lnkRuntimePath dataDirListTxt mkDirDateTime logStdErr logStdOut logDateTime

AH::UTValidate exports functions: searchKeywords validate validateText validateFITS validateAncil md5Check

  ENDDesign
  print "\n";
}

1;
