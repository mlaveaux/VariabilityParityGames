for p in minepump randomgames elevator
do
	echo measureMBR $p
	./measureMBR.sh $p VPGSolver_bdd `pwd`/data/measures/${p}_pess_games/ > `pwd`/data/measures/$p.data
done
