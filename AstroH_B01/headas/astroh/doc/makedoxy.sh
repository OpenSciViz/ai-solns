#!/bin/bash
#
# Generate Doxygen documentation for AstroH.  There are two modes of operation:
# make all standard docs or generate custom documentation.  The first mode is
# performed when no command line arguments are supplied, i.e.
#
#   $ ./makedoxy.sh
#
# Alternatively, a single, custom Doxyfile can be supplied, e.g.
#
#   $ ./makedoxy.sh Doxyfile.custom
#
# The standard docs are:
#
#  * internal html -- complete listing of code, including todo and bug lists
#                  -- input: Doxyfile.internal
#                  -- output: doc/internal/index.html
#
#  * public html   -- public documentation and API
#                  -- input: Doxyfile.html
#                  -- output: doc/html/index.html
#
#  * public latex  -- similar to public html, but as a latex PDF
#                  -- input: Doxyfile.latex
#                  -- output: doc/latex/refman.pdf
#
# The input files/paths used in the standard operation are taken from the file,
# doxy.incl with one entry per line (no comments allowed).  This file ensures
# that all of the standard docs use the same set of inputs.  The entries 
# assume a starting path of $HEADAS/../astroh.
#

#------------------------------------------------------------------------------

## \brief check if given Doxygen file exists and run Doxygen
## \param $1 name of Doxygen input file
function genDoxy {

  # check if Doxyfile exists
  if [ ! -f $1 ]
  then
    echo Doxyfile, $1, not found
    exit
  fi

  # Check if $HEADAS is set
  if [ -z "$HEADAS" ]
  then
    echo Need to set the HEADAS environment variable
    exit
  fi

  # run Doxygen
  doxygen $1
}

#------------------------------------------------------------------------------

## \brief fill INPUT keyword in Doxygen input file with files in doxy.incl;
##   the function will add the prefix, $(HEADAS)/../astroh, to each file
## \param $1 name of Doxygen input file to modify
function addInputs {
  if [ ! -f doxy.incl ] 
  then
    echo "need list of INPUT files to include: doxy.incl"
    exit
  fi

  filelist=( `cat "doxy.incl" `)

  prefix=\$\(HEADAS\)/../astroh/
  inval=""
  for t in "${filelist[@]}"
  do
    inval="$inval $prefix$t"
  done
  echo $inval
  inval="${inval//\//\\/}"

  inpstr="INPUT                  ="
  sed -i "s/$inpstr/$inpstr $inval/g" $1
}

#------------------------------------------------------------------------------

## \brief run single Doxygen input file
function runOne {
  genDoxy $1
}

#------------------------------------------------------------------------------

## \brief run Doxygen with all standard input files
function runAll {

  # Public HTML
  cp Doxyfile.html Doxyfile
  addInputs Doxyfile
  genDoxy Doxyfile
  ed -s doc/html/navtree.css <<< $'89s/300/200/g\nw'

  # Internal HTML
  cp Doxyfile.internal Doxyfile
  addInputs Doxyfile
  genDoxy Doxyfile
  ed -s doc/internal/navtree.css <<< $'89s/300/200/g\nw'

  # Public latex
  cp Doxyfile.latex Doxyfile
  addInputs Doxyfile
  genDoxy Doxyfile
  echo \\AtBeginDocument{\\setcounter{tocdepth}{1}} >> doc/latex/doxygen.sty
  cd doc/latex
  make
}


#------------------------------------------------------------------------------

# *** main ***

# make doc directory, if needed
if [ ! -d doc ] 
then
  mkdir doc
fi

# if Doxyfile given on command-line, then use it; otherwise generate
# documentation for all standard input files
if [ $# -gt 0 ]
then
  runOne $1
else
  runAll
fi

