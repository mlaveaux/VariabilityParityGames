PROBLEM=$1
ALGDIR=$2
cd $PROBLEM
for g in game*
do
        cd $g
        $ALGDIR/run.sh SVPG > ../logs/${g}.log
	echo -n "$g: "
	grep "Attracting time" ../logs/${g}.log|awk -F':' '{ print $2 }'|awk '{ print $1 }'
        cd ..
done
cd ..
