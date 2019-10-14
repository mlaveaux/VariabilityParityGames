#!/bin/bash
lambda=0.5
for i in `seq 50 99`
do
	N=$(((RANDOM%500)+100))
	P=$(((RANDOM%10)+1))
	l=1
	h=$(((RANDOM%8)+3))
	c=$(((RANDOM%8)+6))

	for T in FF FC BC
	do
		rm -f game$T${i}/*
		rmdir game$T${i} 2> /dev/null
		mkdir game$T${i}
	
		echo "Create game with $N nodes, maxprio: $P, lowest out degree: $l, subhighest out degree: $h, 2^$c confs and lambda: $lambda with type $T in `pwd`/game$T$i"
		java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda $T `pwd`/game$T$i
	done
	lambda=`echo "scale=2;$lambda+0.01"|bc`
done
for f in game*;do VPGSolver_bdd $f/SVPG Q$f;done

