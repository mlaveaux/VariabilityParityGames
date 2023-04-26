#!/bin/bash
for i in `seq 0 100`
do
	N=$(((RANDOM%100)+10))
	P=$(((RANDOM%8)+1))
	l=1
	h=$(((RANDOM%($N/10))+1))
	c=$(((RANDOM%4)+2))
	lambda=`seq 0 .01 1 | shuf | head -n1|sed 's/,/./'`
	
	rm -f gameF${i}/*
	rm -f gameC${i}/*
	rmdir gameF${i}
	rmdir gameC${i}
	mkdir gameF${i}
	mkdir gameC${i}

	echo "Create game with $N nodes, maxprio: $P, lowest out degree: $l, subhighest out degree: $h, 2^$c confs and lambda: $lambda in `pwd`/game$i"
	java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda F `pwd`/gameF$i
	VPGSolver_bdd `pwd`/gameF$i/SVPG Q`pwd`/gameF$i/ > /dev/null
        java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda C `pwd`/gameC$i
        VPGSolver_bdd `pwd`/gameC$i/SVPG Q`pwd`/gameC$i/ > /dev/null
done
