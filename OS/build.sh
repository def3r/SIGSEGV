if [ $# -le 0 ]; then
  echo "no file provided bitch"
  exit
fi

filename="a.out"
compiler="gcc"

isC=$(echo "$1" | grep ".c$")
if [[ $isC != "" ]]; then
  filename=$(echo "$1" | sed "s/..$//")
  compiler="gcc"
fi

isCpp=$(echo "$1" | grep ".cpp$")
if [[ $isCpp != "" ]]; then
  filename=$(echo "$1" | sed "s/....$//")
  compiler="g++"
fi

$compiler $1 -o "$filename.out"
if [ $? -ne 0 ]; then
  exit
fi

"./$filename.out" $(echo $@ | sed "s/^$1 //")
