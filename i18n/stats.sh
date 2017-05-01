#!/bin/bash

for file in po/*.po; do
  printf "$file"": "
  (
    msgfmt "$file" -o - --statistics > /dev/null 
  ) 2>&1
done
