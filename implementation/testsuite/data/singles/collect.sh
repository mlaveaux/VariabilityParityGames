for p in minepump elevator randomscalegames randomgames
do
        echo Single Zlnk $p
        ./time_single.sh $p VPGSolver_bdd_single > `pwd`/data/singles/${p}_zlnk.data
	echo Single Zlnk local $p
        ./time_single.sh $p VPGSolver_bdd_single l > `pwd`/data/singles/${p}_zlnk_local.data
	echo Single MBR $p
	./time_single.sh $p VPGSolver_bdd_single F > `pwd`/data/singles/${p}_MBR.data
	echo Single MBR local $p
	./time_single.sh $p VPGSolver_bdd_single "F l" > `pwd`/data/singles/${p}_MBR_local.data
done
