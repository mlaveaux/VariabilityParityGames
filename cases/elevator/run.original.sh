#!/bin/bash

mcrl22lps -n elevator.original.mcrl2 elevator.original.lps
lpssumelm  elevator.original.lps | lpssuminst  > elevator.original.1.lps
mv elevator.original.1.lps elevator.original.lps

for prop in ` ls property?.mcf `; do
  echo 'Verifying ' $prop
  lps2pbes -f $prop elevator.original.lps elevator.original.$prop.pbes
  echo 'Running pbes2bool...'
  pbes2bool -vrjittyc -s1 elevator.original.$prop.pbes
  rm *.pbes
done;

