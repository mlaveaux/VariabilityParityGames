#!/bin/bash
for i in `seq 1 10`
do
	N=$(((RANDOM%1000)+10))
	P=$(((RANDOM%100)+1))
	l=1
	h=$(((RANDOM%($N/4))+1))
	c=$(((RANDOM%10)+1))
	lambda=`seq 0 .01 1 | shuf | head -n1|sed 's/,/./'`

	rm -f game${i}/*
	rmdir game${i} 2> /dev/null
	mkdir game${i}

	echo "Create game with $N nodes, maxprio: $P, lowest out degree: $l, subhighest out degree: $h, 2^$c confs and lambda: $lambda in `pwd`/game$i"
	java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda `pwd`/game$i	
done
