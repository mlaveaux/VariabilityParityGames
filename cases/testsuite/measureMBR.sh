PROBLEM=$1
CMD=$2
PARAM=$3

cd $PROBLEM
for g in game*
do
        cd $g
        $CMD SVPG m F $PARAM> ../logs/${g}.log
	echo -n "$g: "
	S=$(echo `cat ../logs/${g}.log|grep Assisted|awk '{print $3}'|tr '\n' '+'`0|bc)
	n=$(cat ../logs/${g}.log|grep Assisted|wc -l)
	echo "$S,$n"
#	S=`cat ../logs/${g}.log|grep "Assisted leaf"|awk '{ print $6 }'|tr '\n' '+'`
#	S=`echo ${S}0|bc`
#	echo -n $S,
#	echo -n `cat ../logs/${g}.log|grep "Assisted leaf"|awk 'BEGIN {total=0}
#{total+=$3}
#END {printf("%.2f\n",total/NR)}'`","
#	echo -n `head -n2 SVPG |tail -n1|awk '{print $2}'|sed 's/;//'`,
#	cat ../logs/${g}.log|grep Solved|grep games|awk '{ print $2}'
        cd ..
done
cd ..
