PROBLEM=$1
ALGDIR=$2
cd $PROBLEM
for g in game*
#for g in game_phi3PGs
do
        cd $g
	echo -n "$g: "
	TIME=0
	for cc in sSVPG*
        do
                c=`echo $cc|sed 's/sSVPG//'`
                $ALGDIR/run_single.sh SVPG $c > ../logs/${g}_${c}.log
		ADD=`grep "Solving time" ../logs/${g}_${c}.log|awk -F':' '{ print $2 }'|awk '{ print $1 }'`
		TIME=`echo $ADD + $TIME | bc`
        done
        cd ..
	echo $TIME
done
cd ..
