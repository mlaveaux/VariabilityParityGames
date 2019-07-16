PROBLEM=$1
ALGDIR=$2
cd $PROBLEM
for g in game_phi1PGs
do
        cd $g
        $ALGDIR/run.sh SVPG > ../logs/${g}.log
	echo -n "$g: "
	grep "Solving time" ../logs/${g}.log|awk -F':' '{ print $2 }'|awk '{ print $1 }'
        cd ..
done
cd ..
