#!/bin/sh

if [ $# -ne 1 ]; then
  echo "No SQL file provided"
  exit 1
fi

fName=$(echo $1 | sed 's/[ .].*//g')

rm -rf $fName.db && duckdb $fName.db <$1

if [ $? -ne 0 ]; then
  exit 1
fi

duckdb $fName.db
