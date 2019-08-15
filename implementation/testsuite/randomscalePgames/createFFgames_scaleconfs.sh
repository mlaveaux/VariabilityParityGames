#!/bin/bash
lambda=0.92
for sub in a
do
	for i in `seq 1 10`
	do
		N=$(((RANDOM%400)+10))
		P=$i
		l=1
		h=$(((RANDOM%($N/4))+1))
		c=8
	
		mkdir `pwd`/game_scaleconfs_$i$sub
		echo "Create game with $N nodes, maxprio: $P, lowest out degree: $l, subhighest out degree: $h, 2^$c confs and lambda: $lambda with type FF in `pwd`/game_scaleconfs_$i$sub"
		java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda FF `pwd`/game_scaleconfs_$i$sub
	done
done
