#!/bin/bash
T=C
for i in `seq 1 100`
do
	if [ $i == "50" ]
	then
		T=F
	fi
	N=$(((RANDOM%1000)+10))
	P=$(((RANDOM%100)+1))
	l=1
	h=$(((RANDOM%($N/4))+1))
	c=$(((RANDOM%8)+1))
	lambda=`seq 0 .01 1 | shuf | head -n1|sed 's/,/./'`

	rm -f game$T${i}/*
	rmdir game$T${i} 2> /dev/null
	mkdir game$T${i}

	echo "Create game with $N nodes, maxprio: $P, lowest out degree: $l, subhighest out degree: $h, 2^$c confs and lambda: $lambda with type $T in `pwd`/game$T$i"
	java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda $T `pwd`/game$T$i	
done
