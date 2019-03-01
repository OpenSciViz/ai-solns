
#!/usr/bin/env perl
# handle all i/o to/from manifest  -- init, update, etc.
# 
package AH::UTManifest;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  require Exporter;
  @ISA = qw(Exporter);
  # symbols to export on request
  @EXPORT_OK = qw(initManifest loadManifest loadAhtOpts evalManifest evalAhtOpts setDefault setDescribe setValVec updateManifest);
}

use File::Basename;
use File::Copy;
use File::Path;
use Net::Domain qw(hostname hostfqdn hostdomain domainname);

use AH::UTInfo qw(printAhtOpts printManifest);
use AH::UTInvoke qw($VeryVerbose $NoOp $Verbose envInvoked noOpInvoked);
use AH::UTRCopy qw(dircopy fcopy rcopy);
use AH::UTUtil qw(trim dataDirListTxt logStdErr logStdOut);

# the two essential hashes provided by evals of the manifest
use vars qw(%runtime_manifest %aht_cmdopts); 

my $rcsId = '$Name: AstroH_B01 $ $Id: UTManifest.pm,v 1.20 2012/02/03 19:09:57 dhon Exp $';

sub readManifestFile {
  my $manifestfile = shift;
  my $text = do {
    local $/ = undef;
    open my $fh, "<", $manifestfile or die "could not open $manifestfile $!";
    <$fh>;
  };
  return $text;
}

sub extractEval {
# my $extractstart = quotemeta(shift);
  my $extractstart = shift;
  my $fulltext = shift;
  my $extract = "";
  # if( $Verbose ) { logStdOut(__LINE__, "fetch substring $extractstart ..."); }
  if( $VeryVerbose ) { logStdOut(__LINE__, "fulltext: $fulltext"); }
  my $startidx = index($fulltext, $extractstart);
  if( $startidx < 0 ) { 
    logStdErr(__LINE__, "oops, failed to find subtext: $extractstart !");
    return $extract; 
  }
  my $endidx = index($fulltext, ");");
  my $slen = $endidx - $startidx + 2; # ensure slen includes coda: ");"
  if( $endidx <= $startidx ) { $slen = length($fulltext) - $startidx; }
  # if( $Verbose ) { logStdOut(__LINE__, "start: $startidx, end: $endidx, slen: $slen"); }
  $extract = substr($fulltext, $startidx, $slen) . "\n"; # terminate with newline?
  return $extract;
}

sub loadManifest {
  my $manifestfile = shift;
  if( $VeryVerbose ) { logStdOut(__LINE__, "$manifestfile"); }
  my $fulltext = readManifestFile($manifestfile);
# current rendition of manifest declares two perl string statements
# for feeding to perl eval -- $eval_manifest and $eval_ahtopts
# need to parse 'our $eval_manifest = blah blah' and return as
# single string that can be fed to eval
# please note (kludgey) expectation of (mandatory!) "our \%" text prefix
  my $extraction = "our \%runtime_manifest"; 
  my $eval_manifest = extractEval($extraction, $fulltext);
  return $eval_manifest;
}

sub loadAhtOpts {
  my $manifestfile = shift;
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$manifestfile"); }
  my $fulltext = readManifestFile($manifestfile);
# current rendition of manifest declares two perl string statements
# for feeding to perl eval -- $eval_manifest and $eval_ahtopts
# need to parse 'our $eval_ahtopts = blah blah' and return as
# single string that can be fed to eval
# please note (kludgey) expectation of (mandatory!) "our \%" text prefix
  my $extraction = "our \%aht_cmdopts";
  my $eval_ahtopts = extractEval($extraction, $fulltext);
  return $eval_ahtopts;
}

sub evalManifest {
  my $manifestfile = shift;
  my $eval_manifest = loadManifest($manifestfile);
  if ( length($eval_manifest) <= 0 ) {
    logStdErr(__LINE__, "empty eval string?");
    return;
  }
  if ( index($eval_manifest, '=') < 0 ) {
    logStdErr(__LINE__, "eval string lacks assignment (no = )?");
    return;
  }
  if( $VeryVerbose ) { logStdOut(__LINE__, "$eval_manifest"); }
  eval $eval_manifest; # declare/define %runtime_manifest hash ...
  if( $VeryVerbose ) { printManifest(%runtime_manifest); }
  return %runtime_manifest;
}

sub evalAhtOpts {
  my $manifestfile = shift;
  my $eval_ahtopts = loadAhtOpts($manifestfile);
  if ( length($eval_ahtopts) <= 0 ) { 
    logStdErr(__LINE__, "empty eval string?");
    return;
  }
  if ( index($eval_ahtopts, '=') < 0 ) {
    logStdErr(__LINE__, "eval string lacks assignment (no = )?");
    return;
  }
  # if( $VeryVerbose ) { logStdOut(__LINE__, "eval_ahtopts"); }
  eval $eval_ahtopts; # declare/define %aht_cmdopts hash ...
  if( $VeryVerbose ) { printAhtOpts(%aht_cmdopts); }
  return %aht_cmdopts;
}

sub setManifestHash {
  my $manifestfile = shift; # "./aht_manifest";
  my $primekeyname = shift; # 'toolname' in %runtime_manifest or other key
  my $subkeyname = shift; # "toolname default or describe or valvec"
  my $newval = shift;
  $newval = trim($newval); # rm leading and trailing whitespaces
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$primekeyname , $subkeyname ==> $newval"); }

  my $subkeys = "default describe valvec";
  if( index($subkeys, $subkeyname) < 0 ) {
    logStdErr(__LINE__, "subkeyname $subkeyname not a valid manifest hash item...");
    return;
  }

  my %runtime_manifest = evalManifest($manifestfile);
  my $final = $runtime_manifest{$primekeyname}{final};
  if( lc($final) ne "false" ) {
    logStdOut(__LINE__, "keyname $primekeyname final == $final ... shall not reset.");
    return;
  }

  my $pathprefx = dirname($manifestfile);
  if( length($pathprefx) <= 1 ) { $pathprefx = "./"; }
  my $newmanifest = $pathprefx . "new" . basename($manifestfile);

  open( INMANFST, , $manifestfile) or die __LINE__, "Cannot open input $manifestfile $!";
  open( OUTMANFST, "> $newmanifest") or die __LINE__, "Cannot open output $newmanifest $!";

  my $line = "";
  my $resetline = "";
  my $comment = '^s*#';
  my $closing = '^s*},';
  my $searchkeyname = $primekeyname . '\s*=>\s*{\s*';
  # my $searchsubkey = $subkeyname . '\s*=>\s*"*"';
  # my $searchsubkey = $subkeyname . '\s*=>\s*\,';
  my $searchsubkey = $subkeyname . '\s*=>';
  # double-quoted value string?
  my $dblqval = '\"\s**\s*\"';

  my $seeking = 1; # seek prime keyword
  while( $seeking ) {
 	  $line = <INMANFST>; chomp $line;
    print OUTMANFST "$line\n";
    if( $line =~ /$searchkeyname/i ) {
      $seeking = 0;
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "found primarykey: $primekeyname"); }
    } # found keyname 
  }

  $seeking = 1; # seek subkey keyword
  my $prevline = undef;
  while( $seeking ) {
    $prevline = $line;
    $line = <INMANFST>; chomp $line;
    if( $line =~ /$searchsubkey/i ) { # found non-commented subkey :
      if( $NoOp || $Verbose ) { logStdOut(__LINE__, "found subkey: $subkeyname ==> $newval"); }
      if( $line =~ /$comment/i ) { # ignore commented lines
        # preserve comments, but not duplicates
        # if( $line ne $prevline ) { print OUTMANFST "$line\n"; }
        if( index($line, $prevline) < 0 ) { print OUTMANFST "$line\n"; }
      } 
      else { #  non-commented line:
        my $left = substr($line, 0, 1+index($line,'"'));
        my $right = substr($line, rindex($line,'"'));
        #my $left = substr($line, 0, 2+index($line,'=>'));
        #my $right = substr($line, rindex($line,','));
        $resetline = '  ' . trim($left . $newval . $right); # indent 'subkey => val' by 2 spaces
        if( $NoOp || $Verbose ) { logStdOut(__LINE__, "comment out: $line"); }
        # preserve original via (non-duplicate) comment
        #if( $prevline ne "#$line" ) { print OUTMANFST "#$line\n"; }
        if( index($prevline, "#$line") < 0 ) { print OUTMANFST "#$line\n"; }
        print OUTMANFST "$resetline\n";
        if( $NoOp || $Verbose ) { logStdOut(__LINE__, "set to: $resetline"); }
      }
    }
    else {
      print OUTMANFST "$line \n"; # preserve everything else
    }
    if( $line =~ /$closing/i ) { $seeking = 0; }   
  } # finished reset of keyval

  while( $line = <INMANFST> ) {
 	  chomp $line;
    print OUTMANFST "$line \n";
  } # finished all file i/o  

  close(INMANFST) or die __LINE__, "Cannot close input! $!";
  close(OUTMANFST) or die __LINE__, "Cannot close output! $!";
  rename($manifestfile, $pathprefx . "." . basename($manifestfile));
  rename($newmanifest, $manifestfile);
}

sub setDefault {
# given manifest file and hash keyname, resets 'default' key => value
# to new value, preserving comments and orig. value as comment.
  my $manifest = shift; # "./aht_manifest";
  my $keyname = shift; # "toolname";
  my $newdefault = shift; # "toolname default";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$keyname ... $newdefault"); }
  setManifestHash($manifest, $keyname, "default", $newdefault);
}

sub setDescribe {
# given manifest file and hash keyname, resets 'default' key => value
# to new value, preserving comments and orig. value as comment.
  my $manifest = shift; # "./aht_manifest";
  my $keyname = shift; # "toolname";
  my $newdescription = shift; # "toolname describe";
  $newdescription = $keyname . ": " . $newdescription;
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$keyname ... $newdescription"); }
  setManifestHash($manifest, $keyname, "describe", $newdescription);
}

sub setValVec {
# given manifest file and hash keyname, resets 'valvec' key => value
# to new value, preserving comments and orig. value as comment.
  my $manifest = shift; # "./aht_manifest";
  my $keyname = shift; # "output" or "expected_output" or ?;
  my @newvals = shift; # "toolname valvec";
  my $valvectxt = ""; # construct text from @newvals? scalar string syntax: '[ "foo", "bar", "whatev"]' ?
  foreach my $val ( @newvals ) {
    $valvectxt = $valvectxt . '"' . $val. '", ';
  }
  trim($valvectxt); chomp($valvectxt);
  setManifestHash($manifest, $keyname, "valvec", $valvectxt);
}

sub initManifest {
  my $ref_manifest = shift; # "./aht_manifest.pl";
  refManifest($ref_manifest); # generate from HEREDOC below and write to files
  my $bck_manifest = '.' . $ref_manifest; # always keep a backup ?
  copy( $ref_manifest, $bck_manifest ) or die __LINE__, " failed to init. manifest $!";
}

sub updateManifest {
  # reset any/all non-final manifest hash items ...
  my $manifestfile = shift; # "./aht_manifest.pl";
  if ( ! -e $manifestfile ) { initManifest($manifestfile); }
  my $deflt = undef;
  my $comma = undef;
  my $subdir = "./input/";
  # parse a directory listing of ./input and use result to update the manifest..
  my $valvectxt = dataDirListTxt($subdir);
  if( length($valvectxt) > 0 ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$subdir: $valvectxt"); }
    setManifestHash($manifestfile, 'input', 'valvec', $valvectxt);
    $deflt = $valvectxt; # also (re)set default to first element in valvec?
    $comma = index($valvectxt, ',');
    if( $comma > 0 ) { $deflt = substr($valvectxt, 0, $comma); }
    setDefault($manifestfile, 'input', $deflt);
  }
  else {
    logStdErr(__LINE__, "$subdir empty?");
  }

  # parse a directory listing of ./output and use result to update the manifest..
  $subdir = "./output/";
  $valvectxt = dataDirListTxt($subdir);
  if( length($valvectxt) > 0 ) { 
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$subdir: $valvectxt"); }
    setManifestHash($manifestfile, 'expected_output', 'valvec', $valvectxt);
    $deflt = $valvectxt; # also (re)set default to first element in valvec?
    $comma = index($valvectxt, ',');
    if( $comma > 0 ) { $deflt = substr($valvectxt, 0, $comma); }
    setDefault($manifestfile, 'expected_output', $deflt);
  }
  else {
    logStdErr(__LINE__, "$subdir empty?");
  }

  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "tool.par and $manifestfile updated, and latest outputs copied to expected ..."); }
}

#
################################## private / internal module-package func. ####################################
#
# please edit this HERE area with caution! note the current extraction for the two perl assignment statements
# to be evaled relies on finding the index of ");" terminating each statement below -- please do NOT indroduce
# gratuitous ");" in any comments within the statements below...
#
sub refManifest {
  my $refmanifest = shift;
  open(OUTMANFST, ">$refmanifest") or die __LINE__, " failed to init. refmanifest $!";
# note two space prefix '  ENDHERE' ... please do not break this...
  print OUTMANFST <<'  ENDHERE';
#!/usr/bin/env perl
#
# declare and define/init test-tool hashes (of hashes):
# as our runtime manifest -- perl statements to be eval'd...
#
######################################### start of astro-h unit test manifest ##################################
#
# please do NOT change this hash variable name!
#
our %runtime_manifest = (
#
# the manifest provides direction to aht.pl regarding specifics
# of input and output files, including logs. the testheader should indicate something
# about the nature of the test and who when where it ius/was conducted.
# typically expect the testheader to be set at aht runtime via "aht.pl -i or -u"
# (or "aht.pl --init or --update"). but the developer can override updates by manually
# editing and setting the final attibute to "true" -- which indicates update 
# should not reset this manifest entry:
testheader => {
  final => "false",
  describe => "testheader: indicates version, who, where, when", 
  default => "version, accnt., hostname, date-time",
  valvec => ["v000", "me", "hostname", "today-now"]
},
#
# support either (t)csh or (ba)sh login/runtime...
# the indicated file should establish all required tool runtime 
# environment variables (PATH, LD_LIBRARY_PATH, PERL* etc.)
setup => {
  final => "true",
  describe => "setup: runtime env. in (t)csh or bash",
  default => "./etc/setup.sh",
  valvec => ["./etc/setup.csh", "./etc/setup.sh"]
},
#
# aht.pl must be informed of the specific tool apps this manifest supports
# invoking "aht.pl -t" or "aht.pl -toolname" with the name of an existing
# binary app. will run the app. as well as updating this manifest entry with
# a new "default" app. path-name. i.e. "apt.pl -t foo" should indicate the
# foo binary currently residing in ./bin/.
toolname => {
  final => "false",
  describe => "toolname: binary executable to run/test",
#  default => "",
  default => "ahdemo",
  valvec => ["ahdemo", "./bin/status", "./bin/fitsverify", "./bin/suzakuversion", "./bin/hxdpi", "./bin/xispi", "./bin/xiscoord", "./bin/xissim"]
},
#
# aht.pl should pass along any cmd-line options it does not recognize as its own
# to the tool binary app. it executes/tests. if the tool app. is happy with
# the provided cmd-line options, aht.pl may update this manifest entry (maybe?) 
toolcmdopts => {
  final => "false",
  describe => "toolcmdopts: optional command-line options to feed binary executable",
  default => "mode=h clobber=yes",
#  default => "-l ./input/astroh-short.fits ./input/astroh-long.fits",
# test status binary for coredump (sig abrt & hup):
#  default => "10 7 \'10 sec. duration exit code 7\'",
#  default => "-10 7 \'10 sec. duration, but dumpcore with 3 sec. left to run!\'",
  valvec => ["mode=h", "clobber=yes", "mode=h clobber=yes", "-l", "-q", "-e"]
},
#
# a non-zero timeout informs aht.pl that the tool app. should be expected to
# run to completion within some given time frame. aht.pl may update this manifest entry.
tooltimeout => {
  final => "false",
  describe => "tooltimeout: in seconds -- 0 indicates forever",
  default => "20",
  valvec => ["20", "10", "5", "2"]
},
#
# tool exit status codes
exitstatus => {
  final => "false",
  describe => "exitstatus: expected status codes for success and known error / failure modes.",
  default => "0",
  valvec => ["0", "1", "2", "3", "4", "5"]
},
#
# the full directory-path to the calibration database shall be explicitly indicated here
# or perhaps via an environment variable or some such combination?
caldb => {
  final => "false",
  describe => "caldb: path to the calibration database used by the fits apps. to be tested",
#  default => "${CALDB}",
  default => "./caldb",
  valvec => ["./caldb"]
#  valvec => ["./caldb", "${CALDB}", "${HEADAS}/caldb"]
},
#
# the update ("aht.pl -u") behavior triggers a directory scan of the input directory(ies)
# with the results placed into this entry -- the default will typicall be the first file found.
input => {
  final => "false",  
  describe => "input: location and names of data and ancillary files",  
  default => "",
  valvec => ["./input/event_file_3.fits[events]", "./input/event_file_2.fits[events]", "./input/event_file_1.fits[events]", "./input/astroh-short.fits", "./input/astroh-long.fits"] 
},  
#  
# the update ("aht.pl -u") behavior triggers a directory scan of the output directory(ies)  
# that results in a copy of all currently available files to the expected_output  
# directory(ies), and an update/reset of this entry.  
# note that the filenames will be prefixed automatically with "./expected_output/hostname.domainname/"  
expected_output => {  
  final => "false",  
  describe => "expected_output: files that the test shall compare to the runtime results/outputs for validation",  
  default => "",  
  valvec => ["ahcxxdemo_out3.fits", "ahcxxdemo_out2.fits", "ahcxxdemo_out1.fits", "astroh-short.fits.md5", "astroh-long.fits.md5"]  
},  
#  
# post procesing validation should include searches for specific keywords in output log  
# files (stdout and stderr). this manifest entry must be manually edited and is not affected  
# by an update. consequently it is always final.  
logkeywords => {  
  final => "true",  
  describe => "logkeywords: optional list of keyword to seach in logfiles.",  
  default => "all",  
  valvec => ["success", "* warning", "** error", "*** fatal"]
}  
); # runtime_manifest hash
#  
######################################### end of astro-h unit test manifest ####################################  
#  
# the aht.pl test driver supports its own cmd-line options  
# as expressed in the following perl statment to be eval'd  
# our aht cmd-line options:  
our %aht_cmdopts = (  
help => {  
  describe => "help: -h or or [-]-help synopsis and related info.",  
  default => "brief",  
  valvec => ["brief", "verbose", "veryverbose"]  
},  
#   
tooltest => {  
  describe => "tooltest: -t or [-]-tool or [-]-toolname [ftool name] binary executable of test -- ftool cmdline options",  
  default => "false",  
  valvec => ["test_tool_name", "run the test on the indicated tool"]  
},   
#  
filesys => {  
  describe => "filesys: -f filesystem info. used by init and for runtime test",  
  default => "all",  
#  valvec => [ "./log", "./etc", "./lib", "./bin", "./input", "./output", "./output/stderr", "./output/stdout", "./expected_output", "./expected_output/stderr", "./expected_output/stdout"]  
  valvec => [ "./log", "./etc", "./input", "./output", "./output/stderr", "./output/stdout", "./expected_output", "./expected_output/stderr", "./expected_output/stdout"]  
},  
#  
init => {  
  describe => "init: -i or [-]-init [ftool name] initialize filesystem and create working manifest, and implicit update if possible.",  
  default => "testheader",  
  valvec => ["teastheader", "init the test manifest of this tool"]  
},   
#  
debug => {  
  describe => "debug: -d or [-]-debug [ftool name] exec gdb debugger on current tool binary.",  
  default => "false",  
  valvec => ["false", "true"]  
},   
#  
env => {  
  describe => "env: -e or [-]-env print runtime environment of test.",  
  default => "false",  
  valvec => ["false", "true"]  
},   
#  
noop => {  
  describe => "noop: -n or [-]-noop [ftool name] print runtime execution info.",  
  default => "false",  
  valvec => ["false", "true"]  
},
#  
param => {
  describe => "param: -p or [-]-par [ftool name] print runtime tool parameter info.",  
  default => "false",  
  valvec => ["false", "true"]  
},   
#  
update => {  
  describe => "update: -u or [-]-update [ftool name] recursively copy latest -- good -- test results from runtime ./output subdir to ./expected_output.",  
# the update option has become complicated a bit by the requirement to segregate tests by hostname and date-time.  
# assume the most recent output is something like:  ./hostname/YearMonDay/Hour.Min.Sec/output"  
# and ./output is a symlink to it, and update should recursively copy all ./output to ./expected_output.   
  default => "false",  
  valvec => ["false", "true"]
},
#
verbose => {
  describe => "verbose: -v[v] or [-]-[very]verbose print verbose dscription of runtime test activities.",  
  default => "false",  
  valvec => ["false", "true"]
},   
#  
manifest => {  
  describe => "manifest: -m indicate manifest-file of test",  
  default => "false",  
  valvec => ["false", "true"]
}  
); # aht_cmdopts hash
  ENDHERE
# note two space prefix '  ENDHERE' as indicated at start of HERE above...
# please do not break this...
  close(OUTMANFST) or die
 __LINE__, "Cannot close outlog! $!";
}
  
1;
