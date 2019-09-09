PROBLEM=$1
CMD=$2
ARGS=$3

cd $PROBLEM
for g in gameFF*
do
        cd $g
        $CMD SVPG $ARGS > ../logs/${g}.log
	echo -n "$g: "
	grep "Solving time" ../logs/${g}.log|awk -F':' '{ print $2 }'|awk '{ print $1 }'
        cd ..
done
cd ..
