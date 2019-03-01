#! /usr/bin/perl
# Filename: aht-manifest.pl
# Description: this file forms a complete description of a single unit test case for a tool.
# This file is edited by aht.pl in update mode to punch in information that aht.pl can glean
# from the developer's current test environment. Only lines between tags aht-update and /aht-update
# are affected.
# TODO: add more details for how a developer should use this.
#
# AHT required: the name of the tool being tested.
# aht-update \$aht_tool = '$aht_tool_update';
$aht_tool = 'orig-tool';
# /aht-update (must be on a separate line from input).

# AHT required: list of input files needed by the tool.
# aht-update \@aht_input_file_list = qw(@aht_input_file_list_update);
@aht_input_file_list = qw (
	input/orig-input1.fits
	input/orig-input2.fits
	input/orig-tool.par
);
# /aht-update

# TODO: add all other aht_ variables as specified in the use cases.
