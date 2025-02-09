#!/bin/bash

buildit="\033[90mbuildit\033[0m"

if [ ${#1} -eq 0 ]; then
  echo -e $buildit " \033[31mExit;          no file provided\033[0m"
  exit 1
fi

# -A -> associative array of flags
declare -A flags

# init to 1 -> false
flags["-q"]=1
flags["-!x"]=1
flags["-g"]=1

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
    echo -e $buildit " \033[31mFlag Exit;     $arg not a recogonisable flag\033[0m"
    exit 1
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

[ ${flags["-q"]} -ne 0 ] && echo -e $buildit " \033[32mprocess begin\033[0m; $COMPILER \033[33minitiated\033[0m"

if ! $($COMPILER -o $filename.out $1); then
  if [ ${flags["-q"]} -ne 0 ]; then
    echo ""
    echo -e $buildit " \033[31mExit;          $COMPILER refuses to proceed\033[0m"
  fi
  exit 1
fi

[ ${flags["-q"]} -ne 0 ] && echo -e $buildit " \033[32msuccess\033[0m;       $filename.out \033[33mcreated\033[0m"

# Run if ! -!x flag
if [ ${flags["-!x"]} -ne 0 ]; then
  if [ ${flags["-q"]} -ne 0 ]; then
    echo -e $buildit " \033[32mrunning\033[0m;       \033[33mexecute\033[0m $filename.out"
    echo "────────────────"
  fi

  ./$filename.out
fi

exit 0
