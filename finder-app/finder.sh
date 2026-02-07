#!/bin/sh


if [ $# != 2 ]
then
  echo "Expected 2 argument but got $#"
  exit 1
fi

if [ ! -d "$1" ]
then
  echo "$1 is not a found or is not a directory"
  exit 1
fi

# shellcheck disable=SC2046
NUM_FILES=$(find "$1" -type f | wc -l)
MATCHES=$(grep -r "$2" "$1" | wc -l)

echo "The number of files are ${NUM_FILES} and the number of matching lines are ${MATCHES}"






