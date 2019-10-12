for p in minepump elevator randomgames randomscalegames
do
	echo Family zlnk explicit $p
	./time.sh $p VPGSolver_explicit > `pwd`/data/family/${p}_zlnk_explicit.data
        echo Family zlnk explicit local $p
        ./time.sh $p VPGSolver_explicit l > `pwd`/data/family/${p}_zlnk_explicit_local.data
	echo Family zlnk $p
        ./time.sh $p VPGSolver_bdd > `pwd`/data/family/${p}_zlnk.data
        echo Family zlnk local $p
        ./time.sh $p VPGSolver_bdd l > `pwd`/data/family/${p}_zlnk_local.data
	echo Family MBR $p
	./time.sh $p VPGSolver_bdd F > `pwd`/data/family/${p}_MBR.data
	echo Family MBR local $p
	./time.sh $p VPGSolver_bdd "F l" > `pwd`/data/family/${p}_MBR_local.data
done
