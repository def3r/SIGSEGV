#!/bin/bash

if [ ${#1} -eq 0 ]; then
  echo "buildit Exit;          no file provided"
  exit 1;
fi

# -A -> associative array of flags
declare -A flags

# init to 1 -> false
flags["-q"]=1;
flags["-!x"]=1;
flags["-g"]=1;

for arg in "$@"; do
  if [ $arg == $1 ]; then
    continue  
  fi

  findFlag=1

  for aFlag in "${!flags[@]}"; do
    if [ $aFlag == $arg ]; then
      flags[$arg]=0
      findFlag=0
      break
    fi
  done

  if [ $findFlag -eq 1 ]; then
    echo "buildit Flag Exit;     $arg not a recogonisable flag"
    exit 1;
  fi

done

COMPILER="gcc"
EXTENSION="c"

if echo $1 | grep -q ".cpp"; then
  COMPILER="g++"
  EXTENSION="cpp"
fi

if [ ${flags["-g"]} -ne 1 ]; then
  COMPILER="$COMPILER -g"
fi

# echo "-q flag: "${flags["-q"]}

regex="s/\.$EXTENSION.*//"
filename=$(echo $1 | sed $regex)

[ ${flags["-q"]} -ne 0 ] && echo "buildit Process begin; $COMPILER initiated"

if ! $($COMPILER -o $filename.out $1); then
  if [ ${flags["-q"]} -ne 0 ]; then
    echo ""
    echo "buildit Exit;          $COMPILER refuses to proceed"
  fi
  exit 1;
fi

[ ${flags["-q"]} -ne 0 ] && echo "buildit success;       $filename.out created"
 
# Run if ! -!x flag
if [ ${flags["-!x"]} -ne 0 ]; then
  if [ ${flags["-q"]} -ne 0 ]; then
    echo "buildit Running;       executing $filename.out"
    echo "-----------------------"
    echo
    echo 
  fi
  
  ./$filename.out
fi

exit 0
