#!/bin/bash

read -p "Loop until: " n
for i in $(seq 1 $n); do
    echo $i;
    echo "";
     curl -s -i -H "Content-Type': $1" -H "Authorization: Bearer $2" "$3" -d @$4

done
