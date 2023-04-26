#!/bin/bash

mcrl22lps -n elevator.bits.features.mcrl2 elevator.bits.features.lps
lpssumelm -c elevator.bits.features.lps | lpssuminst  |    lpsconstelm -vst | lpsparelm -v  > elevator.bits.features.1.lps
mv elevator.bits.features.1.lps elevator.bits.features.lps

for prop in ` ls property?.optimised.features.mcf `; do
  echo 'Verifying ' $prop
  lps2pbes elevator.bits.features.lps -f $prop | pbesrewr -pquantifier-one-point | pbesrewr -psimplify | pbesstategraph -v | pbesconstelm -v | pbesparelm -v > elevator.bits.features.$prop.pbes
  echo 'Running pbes2bool...'
  pbes2bool -vrjittyc -s3 elevator.bits.features.$prop.pbes
  rm *.pbes
done;

