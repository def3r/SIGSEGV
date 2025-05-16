#!/bin/bash

if [ $# -eq 0 ]; then
  echo "Program Termination; No File Provided as the CLA"
  exit
fi

file=$1

sed -n '' $file
if [ $? -ne 0 ]; then # this $? of prev cmd is only available here, the exact adjacent next.
  echo "Program Termination; File Not Found."
  exit
fi

fileLen=$(cat $file | wc -l)

echo ""

while true; do
  echo "What to do with the file \`$1\` 😈"
  echo "1. Display Entire File Content"
  echo "2. Display Total No of Developers"
  echo "3. Display Total No of Analyst"
  echo "4. Display Details of Emp having Salary less than 45000"
  echo "5. Display Details of Emp having Salary greater than 45000"
  echo "6. Display Details of First 5 Emp as per IDNO"
  echo "7. Display Details of Last 5 Emp as per IDNO"
  echo "8. Enter String to search in $file"

  echo

  echo -n ">> "
  read input

  echo ""

  if [ $input -eq 1 ]; then
    cat $file
    echo

  elif [ $input -eq 2 ]; then
    echo -n "Total Number of Employee with designation \`Developer\`: "
    grep -c 'Developer' $file

  elif [ $input -eq 3 ]; then
    echo -n "Total Number of Employee with designation \`Analyst\`: "
    grep -c 'Analyst' $file

  elif [ $input -eq 4 ]; then
    count=0
    echo "Details of Employees with Salary Less than 45000"
    sed -n '1p' $file

    for i in $(seq 2 $fileLen); do
      salary=$(sed -n "$i p" $file | awk '{print $NF}' | bc)
      if [ $salary -lt 45000 ]; then
        sed -n "$i p" $file
        count=$(expr $count + 1)
      fi
    done
    echo "Total: $count"

  elif [ $input -eq 5 ]; then
    count=0
    echo "Details of Employees with Salary Greater than 45000"
    sed -n '1p' $file

    for i in $(seq 2 $fileLen); do
      salary=$(sed -n "$i p" $file | awk '{print $NF}' | bc)
      if [ $salary -gt 45000 ]; then
        sed -n "$i p" $file
        count=$(expr $count + 1)
      fi
    done
    echo "Total: $count"

  elif [ $input -eq 6 ]; then
    sed -n '1p' $file
    cat $file | sort | head -5

  elif [ $input -eq 7 ]; then
    sed -n '1p' $file
    cat $file | sort -nr | head -5

  elif [ $input -eq 8 ]; then
    echo -n "Enter what to Find: "
    read bffr
    grep -n "$bffr" $file
    if [ $? -ne 0 ]; then
      echo "No Match Found."
    fi

  elif [ $input -eq 0 ]; then
    exit

  fi

  echo -n "(Enter To Continue)"
  read bffr
  echo

done
