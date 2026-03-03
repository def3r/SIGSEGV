#!/bin/sh
set -e
. ./config.sh

BEAR=""
B_FLAG=""
if [[ $1 == "-B" ]]; then
  BEAR="bear -- "
  B_FLAG="-B"
fi

mkdir -p "$SYSROOT"

for PROJECT in $SYSTEM_HEADER_PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $BEAR $MAKE $B_FLAG install-headers)
done
