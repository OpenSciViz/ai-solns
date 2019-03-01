#!/bin/env perl
use strict;
use warnings;
use diagnostics;

use File::Copy;
use File::Path;
use Net::Domain qw(hostname hostfqdn hostdomain domainname);
use File::Basename;

sub setTool {
  my $subname = (caller(0))[3];
  my $manifest = shift; # | "./aht_manifest";
  my $toolname = shift; #  | "foo";
  my $pathprefx = dirname($manifest);
  if( length($pathprefx) <= 1) { $pathprefx = "./"; }
  my $newmanifest = $pathprefx . "new" . basename($manifest);

  open( INMANFST, , $manifest) or die "$subname> Cannot open input $manifest $!" ;
  open( OUTMANFST, "> $newmanifest") or die "$subname> Cannot open output $newmanifest $!";

#  my $searchme = 'FROM\s+(.+?)(?:[^,]\s+[^,]|\s*;)';

  my $line = "";
  my $resetline = "";
  my $comment = '^s*#';
  my $closing = '^s*},';
  my $searchtoolname = 'toolname\s*=>\s*{\s*';
  my $searchdefault = 'default\s*=>\s*"*"';
 # double-quoted value string?
  my $dblqval = '\"\s**\s*\"';

  my $seeking = 1; # seek toolname keyword
  while( $seeking ) {
 	  $line = <INMANFST>; chomp $line;
    print OUTMANFST "$line \n";
    if( $line =~ /$searchtoolname/i ) { $seeking = 0; } # found toolname 
  }

  $seeking = 1; # seek default keyward
  while( $seeking ) {
    $line = <INMANFST>; chomp $line;
    if( $line =~ /$searchdefault/i ) { # found non-commented "default" keyword :
      if( $line =~ /$comment/i ) { # ignore commented lines
        print OUTMANFST "$line \n"; 
      } 
      else { #  non-commented line:
        my $left = substr($line, 0, 1+index($line,'"'));
        my $right = substr($line, rindex($line,'"'));
        $resetline = $left . $toolname . $right;
        print OUTMANFST "#$line \n";
        print OUTMANFST "$resetline \n";
      }
    }
    else {
      print OUTMANFST "$line \n";
    }
    if( $line =~ /$closing/i ) { $seeking = 0; }
  } # finished reset of toolname

  while( $line = <INMANFST> ) {
 	  chomp $line;
    print OUTMANFST "$line \n";
  } # finished all file i/o  

  close(INMANFST) or die "$subname> Cannot close input! $!";
  close(OUTMANFST) or die "$subname> Cannot close output! $!";
}

sub setDefault {
# given manifest file and hash keyname, resets 'default' key => value
# to new value, preserving comments and orig. value as comment.
  my $subname = (caller(0))[3];
  my $manifest = shift; # "./aht_manifest";
  my $keyname = shift; # "toolname";
  my $newdefault = shift; # "toolname default";
  my $pathprefx = dirname($manifest);
  if( length($pathprefx) <= 1) { $pathprefx = "./"; }
  my $newmanifest = $pathprefx . "new" . basename($manifest);

  open( INMANFST, , $manifest) or die "$subname> Cannot open input $manifest $!" ;
  open( OUTMANFST, "> $newmanifest") or die "$subname> Cannot open output $newmanifest $!";

#  my $searchme = 'FROM\s+(.+?)(?:[^,]\s+[^,]|\s*;)';

  my $line = "";
  my $resetline = "";
  my $comment = '^s*#';
  my $closing = '^s*},';
  my $searchkeyname = $keyname . '\s*=>\s*{\s*';
  my $searchdefault = 'default\s*=>\s*"*"';
 # double-quoted value string?
  my $dblqval = '\"\s**\s*\"';

  my $seeking = 1; # seek toolname keyword
  while( $seeking ) {
 	  $line = <INMANFST>; chomp $line;
    print OUTMANFST "$line \n";
    if( $line =~ /$searchkeyname/i ) { $seeking = 0; } # found keyname 
  }

  $seeking = 1; # seek default keyward
  while( $seeking ) {
    $line = <INMANFST>; chomp $line;
    if( $line =~ /$searchdefault/i ) { # found non-commented "default" keyword :
      if( $line =~ /$comment/i ) { # ignore commented lines
        print OUTMANFST "$line \n"; 
      } 
      else { #  non-commented line:
        my $left = substr($line, 0, 1+index($line,'"'));
        my $right = substr($line, rindex($line,'"'));
        $resetline = $left . $newdefault . $right;
        print OUTMANFST "#$line \n";
        print OUTMANFST "$resetline \n";
      }
    }
    else {
      print OUTMANFST "$line \n";
    }
    if( $line =~ /$closing/i ) { $seeking = 0; }
  } # finished reset of toolname

  while( $line = <INMANFST> ) {
 	  chomp $line;
    print OUTMANFST "$line \n";
  } # finished all file i/o  

  close(INMANFST) or die "$subname> Cannot close input! $!";
  close(OUTMANFST) or die "$subname> Cannot close output! $!";
  rename($manifest, $pathprefx . "." . basename($manifest));
  rename($newmanifest, $manifest);
}

#setTool("./aht_manifest.pl", "./bin/fitsverify");

setDefault("./aht_manifest.pl", "toolname", "new default value")
