#!/bin/bash

for f in ` ls *.mcf `; do
  cat $f | sed 's/level/Level/g' > tmp.mcf;
  mv tmp.mcf $f
done;

