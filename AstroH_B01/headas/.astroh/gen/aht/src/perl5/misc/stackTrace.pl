# http://www.perlmonks.org/?node_id=640319
use strict;
use warnings;
use Log::Log4perl qw(:easy);

Log::Log4perl->easy_init($WARN);
my $log = Log::Log4perl->get_logger('stack');

sub show_call_stack {
  my ( $path, $line, $subr );
  my $max_depth = 30;
  my $i = 1;
  if ($log->is_warn()) {
    $log->warn("--- Begin stack trace ---");
    while ( (my @call_details = (caller($i++))) && ($i<$max_depth) ) {
      $log->warn("$call_details[1] line $call_details[2] in function $call_details[3]");
    }
    $log->warn("--- End stack trace ---");
  }
}
