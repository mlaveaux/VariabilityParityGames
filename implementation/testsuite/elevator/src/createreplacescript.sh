#!/bin/bash
input="replacersinaut"
while IFS= read -r line
do
  f=`echo $line|awk -F'=' '{ print $1 }'`
  s=`echo $line|awk -F'=' '{ print $2 }'`
  echo "sed -i 's/$f/$s/g' $1"
done < "$input"
