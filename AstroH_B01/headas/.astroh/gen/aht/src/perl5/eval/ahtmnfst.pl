#!/bin/env perl
use File::Touch;
use File::Path;
#
my %testheader = (name => "testheader", default => "version, what, who, host, date-time", valvec => ["v000", "fitsverify", "me", "hostname", "today-now"]);
my %setup = (name => "setup", default => "./etc/setup.sh", valvec => ["runtime environment setup script"]);
my %toolname = (name => "toolname", default => "fitsverify", valvec => ["fitsverify"]);  
my %toolcmdopts = ( name => "toolcmdopts", default => "-l", valvec => ["-l", "-q", "-e" ] );
# note that the inputs below are expected to be in the local-working sub-directory: ./input (see filesys hash above)
my %input = ( name => "input", default => "ref/astroh-short.fits", valvec => ["ref/astroh-short.fits", "ref/astroh-long.fits" ] );
# and the other directories should exist (aht.pl -i should ensure this)
my %output = ( name => "output", default => "all", valvec => ["a.fits", "b.fits", "c.fits" ] );
my %expected_output =  ( name => "expected_output", default => "all", valvec => ["fitsverify.log", "aout.fits", "bout.fits" ] );
#
our $eval_manifest = '
our %runtime_manifest = (testheader => %testheader, setup => %setup, toolname => %toolname, toolcmdopts => %toolcmdopts, input => %input, output => %output, expected_output => %expected_output);';
#eval $eval_manifest;
#print "evaled declared/defined runtime_manifest hash ... ", %runtime_manifest, "\n";
#
#
# declare and define/init (two) test-tool hashes (of hashes):
my %tooltest = (name => "tooltest", default => "-t fitsverify", valvec => ["-t test_tool_name", "--tooltest=test_tool_name", "run the test on the indicated tool"]);  
my %filesys = (name => "filesys", default => "all", valvec => ["./log/", "./logstdout", "./logstderr", "./etc", "./lib", "./bin", "./input", "./output", "./expected_output"]);  
my %update = (name => "update", default => "", valvec => ["-u", "--update", "update the test manifest of this tool"]); 
my %init = (name => "init", default => "", valvec => ["-i", "--init", "init the test manifest of this tool"]); 
my %help = (name => "help", default => "", valvec => ["-h", "--help", "print out all available %ahtcmdopts content"]); 
#
our $eval_ahtopts = 
'our %aht_cmdopts = (help => $help, tooltest => $tooltest, filesys => $filesys, init => $init, update => $update);';
#'our %aht_cmdopts = (help => %help, tooltest => %tooltest, filesys => %filesys, init => %init, update => %update);';
#'our %aht_cmdopts = (help => %{$help}, tooltest => %{$tooltest}, filesys => %{%filesys}, init => %{$init}, update => %{$update});';
#
eval $eval_ahtopts;
#print "evaled declared/defined aht_cmdopts hash ... ",  $aht_cmdopts, "\n";
#print "evaled declared/defined aht_cmdopts hash ... ",  %aht_cmdopts, "\n";
#print "evaled declared/defined aht_cmdopts hash ... ",  %{$aht_cmdopts}, "\n";

foreach $ahtkey ( keys %aht_cmdopts ) {
  print "ahtkey == $ahtkey\n";
# attribution -- http://www.kf6nvr.net/blog/archives/000727.html:
  my %opthash = %{$aht_cmdopts{$ahtkey}};
#  print "opthash ... ",  %opthash, "\n";
  foreach $key ( keys %opthash ) {
    print "$key == $opthash{$key} \n"; 
    if( $key == "valvec" ) {
      foreach $i ( 0 .. $#{ $hash{$key} } ) {
        my $val = $hash{$key}[$i];
        print "$key # $i == $val \n";
      }
    }
  }
}
#
#
# orginal concept:
sub orig_concept {
  my $evalthis = 'our %runtime = (input => ["fooA.fits", "fooB.fit", "fooC.fits"], expected_output => ["barA.fits", "bar.txt"]);';
  eval $evalthis;
  # print the whole thing with indices
  foreach $dirname ( keys %runtime ) {
   mkpath($dirname);
   foreach $i ( 0 .. $#{ $runtime{$dirname} } ) {
      my $filename = $runtime{$dirname}[$i];
      touch("$dirname/$filename");
      print "($dirname) val # $i = $filename \n";
    }
  }
}

#orig_concept();
