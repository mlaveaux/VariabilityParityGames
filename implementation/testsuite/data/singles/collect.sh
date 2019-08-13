for p in randomgames
do
        echo Single Zlnk $p
        ./time_single.sh $p VPGSolver_bdd_single > `pwd`/data/singles/${p}_zlnk.data
	echo Single MBR $p
	./time_single.sh $p VPGSolver_bdd_single F > `pwd`/data/singles/${p}_MBR.data
done
for p in randomgames minepump elevator
do
	echo Single MBR local $p
	./time_single.sh $p VPGSolver_bdd_single "F l" > `pwd`/data/singles/${p}_MBR_local.data
done
