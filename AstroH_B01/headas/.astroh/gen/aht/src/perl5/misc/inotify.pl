#!/bon/env perl
#
use Linux::Inotify2;
use Params::Validate ':all';

sub watchFiles {
# single array of filename-paths passed by value:
  my @files = validate_pos(@_, { type => ARRAY });
#  my $array_ref = validate_pos(@_, { type => ARRAYREF });
#  push @$array_ref, 'another filename array element';

  my $inotify = Linux::Inotify2->new;
  my $file = "./";
  foreach $file ( @files ) {
    $inotify->watch("$file", IN_MODIFY);
  }
  while () {
    my @events = $inotify->read;
    unless (@events > 0) {
      print "read error: $!";
      last ;
    }
    foreach my $event (@events) {
      print $event->fullname . " was modified\n" if $event->IN_MODIFY;
    }
  }
}

sub watchFolders {
# single array of foldername-paths passed by value:
  my @dirs = validate_pos(@_, { type => ARRAY });
  my $inotify = Linux::Inotify2->new;
  my $dir = "./";
  foreach $dir ( @dirs ) {
    $inotify->watch("$dir", IN_MODIFY);
  }
  while () {
    my @events = $inotify->read;
    unless (@events > 0) {
      print "read error: $!";
      last ;
    }
    foreach my $event (@events) {
      print $event->fullname . " was modified\n" if $event->IN_MODIFY;
    }
  }
}
