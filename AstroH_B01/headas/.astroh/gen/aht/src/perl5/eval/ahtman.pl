#!/bin/env perl
use File::Touch;
use File::Path;
#
# orginal concept:
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
