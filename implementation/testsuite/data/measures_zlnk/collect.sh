for p in minepump randomgames elevator
do
	echo measureZlnk $p
	./measureZlnk.sh $p VPGSolver_bdd  > `pwd`/data/measures_zlnk/$p.data
done
