#!/bin/bash
lambda=0.92
for sub in a b c d
do
	for i in `seq 2 15`
	do
		N=$(((RANDOM%500)+100))
		P=$(((RANDOM%10)+1))
		l=1
		h=$(((RANDOM%(17))+3))
		c=$i	

		rm -f `pwd`/game_scaleconfs_$i$sub/*
		rmdir `pwd`/game_scaleconfs_$i$sub
		mkdir `pwd`/game_scaleconfs_$i$sub
		echo "Create game with $N nodes, maxprio: $P, lowest out degree: $l, subhighest out degree: $h, 2^$c confs and lambda: $lambda with type FF in `pwd`/game_scaleconfs_$i$sub"
		java -jar ../housekeeping.jar randomgame $N $P $l $h $c $lambda FF `pwd`/game_scaleconfs_$i$sub
	done
done
