#! /usr/bin/perl
# File: aht.pl
# Description: develop and/or execute a single unit test case for any command line tool.
#   When invoked without arguments, aht.pl executes a unit test based on the manifest file
#   (aht-manifest.pl) in the working directory. The manifest file contains a complete description
#   of all aspects of the test, including the input required, output expected and exit status expected.
#   Aht.pl compares the actual behavior of the command line tool to the behavior defined as correct
#   by the manifest file. When invoked with arguments, aht.pl uses the current content of the working
#   directory to construct or update the manifest file. For this latter application, the developer
#   should first verify that all input and output files are valid. Usage:
#
#     aht.pl [ -s expected-status ] [ -t tool-name ] [ -u ]
#
# Author: James Peachey, HEASARC
#
# Interpret command line options.
# TODO: put command line parsing and validation into a sub.
use Getopt::Std;
getopts('s:t:u');

# Retrieve the tool expected status, if present on the command line.
my $aht_expected_status_update = $opt_s;

# Retrieve the tool name parameter, if present on the command line.
my $aht_tool_update = $opt_t;

# Check for update mode, if selected on the command line.
my $aht_update = $opt_u;

# Needed for file operations.
use File::Basename;
use File::Copy;

# TODO: validate the input:
#   1. -t and/or -s require or perhaps imply -u because name and status are saved in the manifest.

# Read the input manifest.
# TODO: put manifest handler in a function.
my $manifest_file = './aht-manifest.pl';
open FILE, $manifest_file or die "Unable to open file $manifest_file\n";
my @input_manifest = <FILE>;
close FILE;

# Evaluate the input manifest in order to set aht's variables to run the unit test. Think of this
# as sourcing the setup script for this test.
# TODO: an issue will be that this changes the environment of the test executable itself. If
#       the script is executing multiple tests, this may be an issue since we would ideally
#       like each test to be independent of the rest.
eval join "", @input_manifest;

# Update the manifest based on user input if update mode enabled.
my @updated_manifest;
if ($aht_update) {
  # Check whether any of the reserved optional "update" variables (ending in _update) are blank. If so,
  # reset them with their values from the input manifest. This is so that the original values will
  # be preserved when the updated manifest is built up below.
  $aht_expected_status_update = $aht_expected_status unless ($aht_expected_status_update =~ /\S/);
  $aht_tool_update = $aht_tool unless ($aht_tool_update =~ /\S/);

  # Find all input files. Add cosmetic white space around each element, because this array
  # will be expanded by the eval below.
  @aht_input_file_list_update = &format_file_list(&find_all('input'));

  # Find all output files. Add cosmetic white space around each element, because this array
  # will be expanded by the eval below.
  @aht_output_file_list_update = &format_file_list(&find_all('output'));

  # This next block performs the actual update operation:
  #   Iterates over the input manifest and builds the updated manifest line by line.
  #   Replaces parts of the input manifest that are between the tags aht-update and /aht-update.
  #   Uses perl's eval to punch the updated contents into the updated manifest.
  while ($_ = shift @input_manifest) {
    # Put the line onto the updated manifest as-is.
    push @updated_manifest, $_;

    # Find leading tag "aht-update", and match the expression that follows the tag for eval purposes.
    if (/\s*#\s*aht-update\s*(.*)/) {
      # Eval the expression that followed the tag, storing the result in $result.
      eval "\$result = \"$1\"";

      # Save the result in the updated manifest, adding final newline.
      push @updated_manifest, "$result\n";

      # Now discard lines until the terminating tag "/aht-update" is found.
      while ($_ = shift @input_manifest) {
        if (/\s*#\s*\/aht-update\s*(.*)/) {
          # This line matches the terminating tag, so write it on the updated manifest and
          # exit the loop.
          push @updated_manifest, $_;
          last;
        }
      }
    }
  }
  # TODO:
  #   1. Save the updated manifest (replacing input manifest).
  #   2. Replace output-expected/ directory contents with copy of output/ directory.

  # Update the parameter file stored in the input directory; use the parameter file
  # in the developer's environment.
  # TODO: put this in a sub.
  my $input_par_file = &find_par_file($aht_tool_update), "\n";
  if ($input_par_file =~ /\S/) {
    print "Copying parameter file $input_par_file to the input directory\n";
    my ($name, $path, $suffix) = fileparse($input_par_file);
    copy ($input_par_file, "input/$name$suffix");
  }
} else {
  # No update requested, so updated manifest is the same as the input manifest.
  @updated_manifest = @input_manifest;
  # TODO aht would next do the following:\n";
  #    1. Confirm the existence of input files \@aht_input_file_list =\n\t\t".join ("\n\t\t", @aht_input_file_list)."\n";
  #    2. Confirm the environment, existence of reference output files, etc. Error out if can't run the test.\n";
  #    3. Test the tool \$aht_tool = $aht_tool\n";
  #    4. Check the outcomes to make sure evrything is OK.\n";
}

# For now, just print the updated manifest in all cases. This will be taken out when the tool is complete.
print @updated_manifest;

# Utility to find all files under a given subdirectory.
# The following glob produces an array with the list of all files in the given directory.
sub find_all {
  my $directory = shift;
  # TODO: use find instead of glab.
  return glob ("$directory/*");
}

# Utility to add whitespace around array elements, in order to format output.
sub format_file_list {
  # Add tab before and newline after each element.
  grep s/(.*)/\t\1\n/, @_;
  # Also add leading newline to first element.
  $_[0] = "\n$_[0]";
  return @_;
}

# Utlity to find the parameter file in the user's environment.
# Returns empty string if not found.
sub find_par_file {
  my $tool_name = shift;
  # Get PFILES from environment, or else default to current directory.
  my $pfiles = defined($ENV{"PFILES"}) ? $ENV{"PFILES"} : ".";

  # Split into local pfiles path...
  my $loc_pfiles = $pfiles; $loc_pfiles =~ s/;.*//;

  # ... and system pfiles path.
  my $sys_pfiles = $pfiles; $sys_pfiles =~ s/^$loc_pfiles;//;

  # Look for a local parameter file.
  my $par_file = &search_path("$tool_name.par", $loc_pfiles);

  # If local parameter file not found, look for a system parameter file.
  if ($par_file !~ /\S/) {
    $par_file = &search_path("$tool_name.par", $sys_pfiles);
  }

  # Return the (possibly blank) file name.
  return $par_file;
}

# Utility to search an input colon-delimited path for a file.
sub search_path {
  my $file = shift;
  my @path = split /:/, shift;
  foreach my $dir (@path) {
    return "$dir/$file" if (-e "$dir/$file" and -f "$dir/$file");
  }
  return "";
}
