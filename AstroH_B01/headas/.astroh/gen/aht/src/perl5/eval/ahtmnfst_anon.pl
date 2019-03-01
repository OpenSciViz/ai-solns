#!/bin/env perl
#
# declare and define/init test-tool hashes (of hashes):
# as our runtime manifest -- a perl statement to be eval'd...
#
our $eval_manifest = 'our %runtime_manifest = (
#
testheader => {
  final => "false",
  describe => "testheader: indicates version, who, where, when", 
  default => "version, accnt., hostname, date-time",
  valvec => ["v000", "me", "hostname", "today-now"]
},
#
setup => {
  final => "true",
  describe => "setup: runtime env. in (t)csh or bash",
  default => "./etc/setup.csh",
  valvec => ["./etc/setup.csh", "./etc/setup.sh"]
},
#
toolname => {
  final => "false",
  describe => "toolname: binary executable to run/test",
  default => "./bin/fitsverify",
  valvec => ["./bin/fitsverify", "./bin/suzakuversion", "./bin/xiscoord", "./bin/xissim"]
},
#
toolcmdopts => {
  final => "false",
  describe => "toolcmdopts: optional command-line options to feed binary executable",
  default => "-l",
  valvec => ["-l", "-q", "-e" ]
},
#
tooltimeout => {
  final => "false",
  describe => "tooltimeout: in seconds -- 0 indicates forever",
  default => "0",
  valvec => ["0" ]
},
#
input => {
  final => "false",
  describe => "input: location and names of data and ancillary files",
  default => "./input/ref/astroh-short.fits",
  valvec => ["./input/ref/astroh-short.fits", "ref/astroh-long.fits"]
},
#
output => {
  final => "false",
  describe => "output: location and names of all tool runtime outputs",
  default => "all",
  valvec => ["./output/a.fits", "./output/b.fits", "./output/c.fits" ]
},
#
expected_output => {
  final => "false",
  describe => "expected_output: files that the test shall compare to the runtime results/outputs for validation",
  default => "all",
  valvec => ["fitsverify.log", "aout.fits", "bout.fits"]
},
#
logkeywords => {
  final => "true",
  describe => "logkeywords",
  default => "all",
  valvec => ["* warning", "** error", "*** fatal"]}, 
);';
#
sub evalManifest {
  my $eval_manifest = shift;
  eval $eval_manifest;

  foreach $rtkey ( keys %runtime_manifest ) {
    print "rtkey == $rtkey\n";
  # attribution -- http://www.kf6nvr.net/blog/archives/000727.html:
    my %rthash = %{$runtime_manifest{$rtkey}};
    foreach my $mkey ( keys %rthash ) {
      print "$rtkey $mkey == $rthash{$mkey} \n"; 
      if( $mkey == "valvec" ) {
        foreach my $m ( 0 .. $#{ $rthash{$mkey} } ) {
          my $mval = $rthash{$mkey}[$m];
          print "$rtkey $mkey # $m == $mval \n";
        }
      }
    }
  }
}
#
# our aht cmd-line options:
our $eval_ahtopts = 
'our %aht_cmdopts = (
help => {name => "help", default => "", valvec => ["-h", "--help", "print out all available %aht_cmdopts content"]}, 
tooltest => {name => "tooltest", default => "-t fitsverify", valvec => ["-t test_tool_name", "--tooltest=test_tool_name", "run the test on the indicated tool"]}, 
filesys => {name => "filesys", default => "all", valvec => ["./log/", "./logstdout", "./logstderr", "./etc", "./lib", "./bin", "./input", "./output", "./expected_output"]}, 
init => {name => "init", default => "", valvec => ["-i", "--init", "init the test manifest of this tool"]}, 
update => {name => "help", default => "", valvec => ["-h", "--help", "print out all available %ahtcmdopts content"]}, 
);';
#
sub evalAhtOpts {
  my $eval_ahtopts = shift;
  eval $eval_ahtopts;
  foreach $ahtkey ( keys %aht_cmdopts ) {
    print "ahtkey == $ahtkey\n";
  # attribution -- http://www.kf6nvr.net/blog/archives/000727.html:
    my %opthash = %{$aht_cmdopts{$ahtkey}};
    foreach my $optkey ( keys %opthash ) {
      print "$ahtkey $optkey == $opthash{$optkey} \n"; 
      if( $optkey == "valvec" ) {
        foreach my $o ( 0 .. $#{ $opthash{$optkey} } ) {
          my $optval = $opthash{$optkey}[$o];
          print "$ahtkey $optkey # $o == $optval \n";
        }
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
evalAhtOpts($eval_ahtopts);
evalManifest($eval_manifest);
