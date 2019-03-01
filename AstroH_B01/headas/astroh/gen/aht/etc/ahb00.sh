#!/bin/bash
if [ -e ${HEADAS}/headas-init.sh ] ; then
  . ${HEADAS}/headas-init.sh >& /dev/null
  echo sourced ${HEADAS}/headas-init.sh
else
  echo cannot find ${HEADAS}/headas-init.sh
  exit
fi
#
# this is usually a one-time login:
cvs -d :pserver:${USER}@daria:/headas login
#
cvs -d :pserver:${USER}@daria:/headas co -r AstroH_B00 -d b00astroh headas 
pushd b00astroh
cvs -d :pserver:${USER}@daria:/astroh co -r AstroH_B00 astroh 
pushd BUILD_DIR
#
export HEA=~/local/astrohb00
#
./configure --prefix=$HEA --enable-symbols --with-components=astroh
hmake
hmake install
hmake test
hmake install-test
# now reset HEADAS and setup env. for freshly installed system:
export HEADAS=${HEA}/x86_64-unknown-linux-gnu-libc2.12
#
echo "HEADAS == ${HEADAS}"
. ${HEADAS}/headas-init.sh >& /dev/null
echo sourced new ${HEADAS}/headas-init.sh
# note for (tcsh) a rehash may be needed...
#
exit

