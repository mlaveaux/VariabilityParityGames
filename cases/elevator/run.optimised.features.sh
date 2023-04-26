#!/bin/bash

mcrl22lps -n elevator.optimised.features.mcrl2 elevator.optimised.features.lps
lpssumelm -c elevator.optimised.features.lps | lpssuminst  | lpsparunfold -l -s"Buttons" |   lpsconstelm -vst | lpsparelm -v  > elevator.optimised.features.1.lps
mv elevator.optimised.features.1.lps elevator.optimised.features.lps

for prop in ` ls property?.optimised.features.mcf `; do
  echo 'Verifying ' $prop
  lps2pbes elevator.optimised.features.lps -f $prop | pbesrewr -pquantifier-one-point | pbesrewr -psimplify | pbesstategraph -v | pbesconstelm -v | pbesparelm -v > elevator.optimised.features.$prop.pbes
  echo 'Running pbes2bool...'
  time pbespgsolve -vrjittyc -sprioprom elevator.optimised.features.$prop.pbes
  rm *.pbes
done;

