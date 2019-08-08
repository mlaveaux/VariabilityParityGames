for p in minepump elevator
do
        echo Single Zlnk $p
        ./time_single.sh $p VPGSolver_bdd_single > `pwd`/data/singles/${p}_zlnk.data
	echo Single MBR $p
	./time_single.sh $p VPGSolver_bdd_single F > `pwd`/data/singles/${p}_MBR.data
done
