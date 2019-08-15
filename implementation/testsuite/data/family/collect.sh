for p in randomscalegames
do
        echo Family zlnk $p
        ./time.sh $p VPGSolver_bdd > `pwd`/data/family/${p}_zlnk.data
	echo Family MBR $p
	./time.sh $p VPGSolver_bdd F > `pwd`/data/family/${p}_MBR.data
	echo Family MBR local $p
	./time.sh $p VPGSolver_bdd "F l" > `pwd`/data/family/${p}_MBR_local.data
done
