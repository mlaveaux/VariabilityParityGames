for p in minepump randomgames elevator
do
        echo Family zlnk $p
        ./time.sh $p VPGSolver_bdd > `pwd`/data/family/${p}_zlnk.data
	echo Family MBR $p
	./time.sh $p VPGSolver_bdd F > `pwd`/data/family/${p}_MBR.data
done
