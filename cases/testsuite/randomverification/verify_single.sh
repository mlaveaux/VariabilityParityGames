CMD=$1
ARGS=$2
for g in game*
do
        cd $g
	echo "Start $g"
	for cc in sSVPG*
	do
		c=`echo $cc|sed 's/sSVPG//'`
		$CMD SVPG c$c f $ARGS > ../logs/${g}_${c}.log
		cat ../logs/${g}_${c}.log|tail -n3|head -n1|awk -F':' '{ print $2 }'|sed 's/ //'|sed 's/,$//'|tr ',' '\n'|sort -n > ../logs/${g}_${c}.even
		if ! diff ../logs/${g}_${c}.even sol_sSVPG${c}_even > /dev/null
		then
			echo "Inconsistency found in $g with configuration $c"
		fi
	done
	cd ..
done
