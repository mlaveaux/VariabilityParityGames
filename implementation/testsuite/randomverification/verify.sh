ALGDIR=$1
for g in game*
do
	cd $g
        $ALGDIR/run_fulloutput.sh SVPG > ../logs/${g}.log
	echo "Start $g"
	for cc in sSVPG*
	do
		c=`echo $cc|sed 's/sSVPG//'`
		cat ../logs/${g}.log |grep "For product $c "|head -n1|awk -F':' '{ print $2 }'|sed 's/ //'|sed 's/,$//'|tr ',' '\n'|sort -n > ../logs/${g}_${c}.even
		if ! diff ../logs/${g}_${c}.even sol_sSVPG${c}_even > /dev/null
		then
			echo "Inconsistency found in $g with configuration $c"
		fi
	done
	cd ..
done
