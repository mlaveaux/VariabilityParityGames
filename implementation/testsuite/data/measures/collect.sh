if [ "$1"  = "" ]
then
        probs="minepump elevator randomgames randomscalegames"
else
        probs=$1
fi
for p in $probs
do
	echo measureZlnk $p
	./measureZlnk.sh  $p VPGSolver_bdd > `pwd`/data/measures/${p}_zlnk.data
	echo measureMBR $p
	./measureMBR.sh $p VPGSolver_bdd l > `pwd`/data/measures/${p}_MBR_local.data
done
