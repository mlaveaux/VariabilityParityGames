CMD=$1
ARGS=$2
for g in game*
do
        cd $g
	echo "Start $g"
	for cc in sSVPG*
	do
		c=`echo $cc|sed 's/sSVPG//'`
		$CMD SVPG c$c l $ARGS > ../logs/${g}_${c}.log
		cat ../logs/${g}_${c}.log|tail -n3|head -n1|awk -F':' '{ print $2 }'|sed 's/ //'|sed 's/,$//'|tr ',' '\n'|sort -n |tr -d '\n'> ../logs/${g}_${c}.zero
		if ! diff ../logs/${g}_${c}.zero sol_sSVPG${c}_zero > /dev/null
		then
			echo "Inconsistency found in $g with configuration $c"
		fi
	done
	cd ..
done
