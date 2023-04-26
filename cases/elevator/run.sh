#!/bin/bash

mcrl22lps -n elevator.mcrl2 elevator.lps
lpssumelm  elevator.lps | lpssuminst  > elevator.1.lps
mv elevator.1.lps elevator.lps

for prop in ` ls *.mcf `; do
  echo 'Verifying ' $prop
  lps2pbes -f $prop elevator.lps elevator.$prop.pbes
  echo 'Running pbes2bool...'
  time pbespgsolve -vrjittyc -sprioprom elevator.$prop.pbes
  rm *.pbes
done;

