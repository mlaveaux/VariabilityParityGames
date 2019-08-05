PROBLEM=$1
CMD=$2

cd $PROBLEM
for g in game*
do
        cd $g
        $CMD SVPG m > ../logs/measurezlnk_${g}.log
	echo -n "$g: "
	N=`cat ../logs/measurezlnk_${g}.log|grep -i attracted|wc -l`
	sum=`cat ../logs/measurezlnk_${g}.log|grep -i attracted|awk '{ print $2 }'|tr '\n' '+'`
	S=`echo ${sum}0|bc`
	echo $N,$S
        cd ..
done
cd ..
