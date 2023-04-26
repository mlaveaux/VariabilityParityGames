#!/bin/bash

mcrl22lps -n elevator.original.features.mcrl2 elevator.original.features.lps
lpssumelm  elevator.original.features.lps | lpssuminst | lpsrewr  |  lpsconstelm -vst | lpsparelm -v  > elevator.original.features.1.lps
mv elevator.original.features.1.lps elevator.original.features.lps

for prop in ` ls property?.original.features.mcf `; do
  echo 'Verifying ' $prop
  lps2pbes -f $prop elevator.original.features.lps | pbesrewr -pquantifier-one-point | pbesrewr -psimplify | pbesstategraph -v | pbesconstelm -v | pbesparelm -v > elevator.original.features.$prop.pbes
  #lps2pbes -f $prop elevator.original.features.lps |  pbesrewr -pquantifier-one-point | pbesrewr -psimplify |  pbesconstelm -v | pbesparelm -v > elevator.original.features.$prop.pbes
  echo 'Running pbes2bool...'
  pbes2bool -vrjittyc -s3 elevator.original.features.$prop.pbes
  rm *.pbes
done;

