use Config;
$has_nonblocking = $Config{d_waitpid} eq "define" ||
                   $Config{d_wait4}   eq "define";

use POSIX qw(:signal_h :errno_h :sys_wait_h);

$SIG{CHLD} = \&REAPER;
sub REAPER {
    my $pid;

    $pid = waitpid(-1, &WNOHANG);

    if ($pid == -1) {
        # no child waiting.  Ignore it.
    } elsif (WIFEXITED($?)) {
        print "Process $pid exited.\n";
    } else {
        print "False alarm on $pid.\n";
    }
    $SIG{CHLD} = \&REAPER;          # in case of unreliable signals
}

$exit_value  = $? >> 8;
$signal_num  = $? & 127;
$dumped_core = $? & 128;

validChild {
  my $childpid = shift;
  if( kill 0 => $childpid ) {
    print "child $childpid is alive!\n";
    return $childpid;
  }
  elsif( $! == EPERM ) { # changed uid?
    print "child $childpid has escaped my control? \n";
  }
  elsif( $! == ESRCH ) {
    print "child $childpid  deceased or zombied? \n";
  }
  else {
    warn "unable check on the status of child $childpid $!\n";
  }
  return 0;
}
