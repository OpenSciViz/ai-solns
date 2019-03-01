#!/usr/bin/env perl
# post-processing validation of output results
#
package AH::UTValidate;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  require Exporter;
  @ISA = qw(Exporter);
  # symbols to export on request
  @EXPORT_OK = qw(searchKeywords validate validateText validateFITS validateAncil md5Check);
}

use File::Copy;
use File::Path;
use File::Compare;
use File::Find;
use File::Basename;
use Digest::MD5;
use Net::Domain qw(hostfqdn);

use AH::UTInvoke qw($NoOp $Verbose $VeryVerbose);
use AH::UTEnv qw($SysPFILES);
use AH::UTExec qw($ToolRunTimeDir $ToolOutlog $ToolErrlog);
use AH::UTManifest qw(loadManifest evalManifest);
use AH::UTUtil qw(trim logStdErr logStdOut);

# the two essential hashes provided by evals of the manifest
use vars qw(%runtime_manifest %aht_cmdopts);

my $rcsId = '$Name: AstroH_B01 $ $Id: UTValidate.pm,v 1.13 2012/03/05 19:57:41 dhon Exp $';

sub md5Check {
  # attributiuon: http://search.cpan.org/dist/Digest-MD5/MD5.pm
  my $file = shift;
  open(FILE, $file) or die __LINE__, "Cannot open file: $file $!";
  binmode(FILE);
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "file: $file ..."); }
  # OO style
  my $ctx = Digest::MD5->new;
  $ctx->addfile(*FILE);
  my $digest = $ctx->hexdigest;
  close(FILE);
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "MD5 checksum == $digest"); }
  return $digest;
}

sub checkFITSFile {
  my $fullfilepath = shift;
  my $md5 = shift | "";

  my $filepath = dirname($fullfilepath);
  my $filename = basename($fullfilepath);

  my $minfits = 2880;
  if ( -e $fullfilepath ) {
    my $size = -s $fullfilepath;
    if( $size < $minfits || ($size % $minfits) ) {
      logStdErr(__LINE__, "Bad FITS file size: $size? ... $fullfilepath ...");
      return "";
    }
  }
  else { # file does not exits?
    logStdErr(__LINE__, "file does not exits? ... $fullfilepath ...");
    return "";
  }
  # ok file exists and is a reasonable size, go ahead with checkum
  my $fileMD5 = md5Check($fullfilepath);
  if( $md5 ) {
    if( index($md5, $fileMD5) < 0 ) {
      logStdErr(__LINE__, "file MD5 wrong... $fullfilepath ... $md5 ... $fileMD5");
      return "";
    }
  }
  return $fileMD5;
}

sub fileCompare {
  my $ref_file = shift;
  my $file = shift;
  my $retval = "false";
  if ( compare($ref_file, $file) == 0 ) {
    $retval = "true";
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "successful congruence of expected output: $ref_file with output result: $file"); } 
  }
  else {
    if( $NoOp || $Verbose ) { logStdErr(__LINE__, "failed congruence of expected output: $ref_file with output result: $file"); }
  }
  return $retval;
}

sub validate { 
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "validating results..."); }
  # reset PFILES env to non-local?
  #if( $SysPFILES && length( $SysPFILES ) > 0 ) {
  #  $ENV{PFILES} = $SysPFILES;
  # logStdOut(__LINE__, "reset PFILES env: $ENV{PFILES}");
  #}
  #else {
  #  logStdOut(__LINE__, "unable to reset PFILES env: $ENV{PFILES}");
  #}
  # or reset to sys install?

  my (%runtime_manifest) = @_;
  my @expected = @{$runtime_manifest{expected_output}{valvec}};
  my $exp = $runtime_manifest{expected_output}{default};
  my $ref = "./expected_output";
  my $expref = $ref;
  # single file validation
  if( lc($exp) ne "all" ) {
    if( $NoOp || $Verbose ) { logStdOut(__LINE__, "expected: $exp"); }
    $expref = $ref .  "/" . $exp; 
    $exp = "./output/" . $exp;
    if( index($exp, '.fits') > 0 ) { validateFITS($expref, $exp); }
    return;
  }
  # list of files
  foreach $exp ( @expected ) {
    logStdOut(__LINE__, "expected: $exp"); }
    $expref = $ref .  "/" . $exp; 
    $exp = "./output/" . $exp;
    if( index($exp, '.fits') >= 0 ) { 
      validateFITS($expref, $exp);
    }
    elsif( index($exp, '.txt') >= 0 ) {
      validateText($expref, $exp);
    }
    else {
      validateAncil($expref, $exp);
  }
}

sub validateText { 
  my $ref = shift;
  my $file = shift;
  if( length($ref) <= 0 || length($file) <= 0 ) {
    logStdErr(__LINE__, "ref: $ref, file: $file ... unable to compare ...");
    return 0;
  }
  my $cmp = fileCompare($ref, $file);
  my $refMD5 = md5Check($ref);
  my $md5 = md5Check($file);
  return $md5;
}

sub validateAncil {
  my $ref = shift;
  my $file = shift;
  if( length($ref) <= 0 || length($file) <= 0 ) {
    logStdErr(__LINE__, "ref: $ref, file: $file ... unable to compare ...");
    return 0;
  }
  my $cmp = fileCompare($ref, $file);
  my $refMD5 = md5Check($ref);
  my $md5 = md5Check($file);
  return $md5;
}

sub validateFITS {
  my $ref = shift;
  my $file = shift;
  if( length($ref) <= 0 || length($file) <= 0 ) {
    logStdErr(__LINE__, "ref: $ref, file: $file ... unable to compare ...");
    return 0;
  }
  my $cp = undef;
  #
  # validate uses ftdiff, so need local copy of ftdiff parfile?
  my $ftdpar = "ftdiff.par";
  my $ftdpfile = $ENV{HEADAS} . "/syspfiles/" . $ftdpar;
  my $ftdpdup = "./input/" . $ftdpar;
  if( ! -e $ftdpdup ) {
    # not sure if update should deal with ftdiff par file? so do it here 
    $cp = copy($ftdpfile, $ftdpdup);
    if( $cp <= 0 ) {
      logStdErr(__LINE__, "failed cp $ftdpfile ... trying \$HOME/pfiles ... ");
      $ftdpfile = $ENV{HOME} . "/pfiles/" . $ftdpar;
      $cp = copy($ftdpfile, $ftdpdup);
      if( $cp <= 0 ) { logStdErr(__LINE__, "failed 2nd cp $ftdpfile ? need to manually establish a ftdiff parfile..."); }
    }
  }
  #
  my $comp = fileCompare($ref, $file);
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "ref: $ref, file: $file -- comparison returned: $comp"); }
  # beyond simple comparison boolean, eval md5 checksum:
  my $refMD5 = md5Check($ref);
  my $md5 = checkFITSFile($file, $refMD5);
  my $ftdif = "ftdiff $ref $file";
  if( $NoOp || $Verbose ) { logStdOut(__LINE__, "$ftdif"); }
  my $resdif = `$ftdif`;
  logStdOut(__LINE__, $resdif);
  return $md5;
}

sub searchKeywords {
  my $manifestfile = shift;
  if( !($ToolOutlog && $ToolErrlog) ) {
    logStdErr(__LINE__, "sorry, but the tool logfiles are unknown -- cannot search keywords"); 
    return;
  }
  my $keyword = undef;
  my $searchme = undef;
  my $file = undef; 
  my @logfiles = ($ToolOutlog, $ToolErrlog);
  my %runtime_manifest = evalManifest($manifestfile);
  my @keywords = @{$runtime_manifest{logkeywords}{valvec}};
  logStdOut(__LINE__, "search stdout and stderr log files for keywords (indicated in manifest $manifestfile):");
  foreach $keyword ( @keywords ) {
    $searchme = quotemeta($keyword);
    logStdOut(__LINE__, "keyword -- $keyword -- $searchme");
  }
  foreach my $file ( @logfiles ) {
    if( $Verbose ) { logStdOut(__LINE__, "start $file ..."); }
    open( INFILE, $file ) or die __LINE__, "Cannot open input $file";
    my $line = "";
    my $cnt = 0;
    while( $line = <INFILE> ) { 
      chomp $line;
      #logStdOut(__LINE__, "$file: $line");
      foreach my $keyword ( @keywords ) {
        $searchme = quotemeta($keyword);
        #logStdOut(__LINE__, "? $keyword in $file: $line");
        if( $line =~ /$searchme/i ) {
          $cnt++;
          logStdOut(__LINE__, "$cnt $file:\n$line");
        } # found keyword
      }
    }
    close( INFILE );
    if( $Verbose ) { logStdOut(__LINE__, "finished $file, cnt: $cnt"); }
  }
}

1;

