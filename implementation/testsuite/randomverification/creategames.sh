#!/bin/bash
for i in `seq 101 200`
do
	N=$(((RANDOM%100)+10))
	P=$(((RANDOM%10)+1))
	l=1
	h=$(((RANDOM%($N/10))+1))
	c=$(((RANDOM%4)+1))
	lambda=`seq 0 .01 1 | shuf | head -n1|sed 's/,/./'`

	rm -f gameF${i}/*
	rm -f gameC${i}/*
	mkdir gameF${i}
	mkdir gameC${i}

	echo "Create game with $N nodes, maxprio: $P, lowest out degree: $l, subhighest out degree: $h, 2^$c confs and lambda: $lambda in `pwd`/game$i"
	java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda F `pwd`/gameF$i
        java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda C `pwd`/gameC$i
done
