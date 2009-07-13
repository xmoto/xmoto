#!/bin/bash

ls -1 | grep -E "^.*\.po$" |
while read FILE
do
  printf "$FILE"": "
  (
      msgfmt "$FILE" -o - --statistics > /dev/null 
  ) 2>&1
done
