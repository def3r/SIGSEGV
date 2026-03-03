#!/bin/sh
set -e

BEAR=""
B_FLAG=""
if [[ $1 == "-B" ]]; then
  BEAR="bear -- "
  B_FLAG="-B"
fi

. ./headers.sh $B_FLAG

for PROJECT in $PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $BEAR $MAKE $B_FLAG install)
done
